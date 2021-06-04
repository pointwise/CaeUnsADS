/****************************************************************************
 *
 * (C) 2021 Cadence Design Systems, Inc. All rights reserved worldwide.
 *
 * This sample source code is not supported by Cadence Design Systems, Inc.
 * It is provided freely for demonstration purposes only.
 * SEE THE WARRANTY DISCLAIMER AT THE BOTTOM OF THIS FILE.
 *
 ***************************************************************************/
/****************************************************************************
 *
 * CaeUnsADS implementation of runtimeWrite(), runtimeCreate(), and
 * runtimeDestroy()
 *
 ***************************************************************************/

#include "apiCAEP.h"
#include "apiCAEPUtils.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "runtimeWrite.h"
#include "pwpPlatform.h"
#include "string.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>


// Define array of valid BC names + one extra for error situations
const char * const BcNames[] = {
    "FLOWTAN",         //  1
    "INFLOW",          //  2
    "OUTFLOW",         //  3
    "FARFIELD",        //  4
    "NRINFLOW",        //  5
    "NROUTFLOW",       //  6
    "INVISCID",        //  7
    "WALLF",           //  8
    "WALLIQ",          //  9
    "WALLIA",          // 10
    "WALLIS",          // 11
    "NMLINFLOW",       // 12
    "PERIODIC",        // 13
    "PERIODICSHDW",    // 14
    "ISOTHERMAL",      // 15
    "ADIABATIC",       // 16
    "INTSCT",          // 17
    "INTSCTSHDW",      // 18
    "WALLFCHT",        // 19
    "FMVINFLOW",       // 20
    "BCERROR"
};
const PWP_UINT32 NumBcs = ARRAYSIZE(BcNames) - 1;

const char attrTitle[] = "Title";


static bool
GetBcData(PWGM_HDOMAIN dom, PWGM_CONDDATA &bc)
{
    bool ret = (0 != PwDomCondition(dom, &bc));
    if (!ret) {
        // For some reason we could not retrieve the BC data!!
        // Set bc to error values.
        bc.tid = NumBcs + 1;
        bc.name = BcNames[NumBcs];
    }
    else if (0 == bc.tid) {
        // Unspecified BC default to:
        //  "INVISCID" "7  No flow normal to the surface" 7
        bc.tid = 7;
        bc.id = 1;
    }
    return ret;
}


static bool
GetBcData(CAEP_RTITEM &rti, PWP_UINT32 ndx, PWGM_CONDDATA &bc)
{
    return GetBcData(PwModEnumDomains(rti.model, ndx), bc);
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

struct ADSData {
private:

    typedef std::vector<PWP_UINT32>     UINT32Vec;
    typedef std::set<PWP_UINT32>        UINT32Set;
    typedef std::vector<std::string>    StringVec;


public:

    ADSData(CAEP_RTITEM &rti) :
        rti_(rti),
        prefix_(0),
        adsBcNames_(),
        usedPrefixPairs_(),
        ndVar_(0)
    {
        rti_.adsData = this;
        memset(bcUsageCnt_, 0, sizeof(bcUsageCnt_));
    }

    ~ADSData()
    {
    }

    bool init()
    {
        bool ret = true;
        memset(bcUsageCnt_, 0, sizeof(bcUsageCnt_));
        PWP_UINT32 domainCount = PwModDomainCount(rti_.model);
        PWP_UINT32 warnId = 0;
        PWGM_CONDDATA bc;
        char adsBcName[80];
        prefix_.resize(domainCount, 0);
        adsBcNames_.resize(domainCount);
        for (PWP_UINT32 i = 0; i < domainCount; ++i) {
            if (!GetBcData(rti_, i, bc)) {
                // bc is set to error values that are safe to use below
                ret = false;
            }
            switch (bc.tid) {
            case 13: // fall through
            case 14: {
                // Build 13 to 14 shadow pair id
                PWP_UINT32 id = prefix_[i] = 100 * bc.id + bc.tid;
                if (!usedPrefixPairs_.insert(id).second) {
                    // Uh oh, id was already used!
                    std::ostringstream msg;
                    msg << "PERIODIC FACE '" << bc.name << "' (" << bc.tid
                        << ":" << bc.id << ") is a duplicate";
                    caeuSendWarningMsg(&rti_, msg.str().c_str(), ++warnId);
                }
                break; }
            case 19:
                // Special handle "WALLFCHT"
                prefix_[i] = 100 * bcUsageCnt_[bc.tid - 1] + 2008;
                break;
            case 20:
                // Special handle "FMVINFLOW"
                prefix_[i] =
                    PWP_UINT32(-(100 * PWP_INT32(bcUsageCnt_[bc.tid - 1]) + 2));
                break;
            case 17:
            case 18:
                // no pairing needed for 17/18 - fall through
            default:
                // Build BC usage id
                prefix_[i] = 100 * bcUsageCnt_[bc.tid - 1] + bc.tid;
                break;
            }
            // Build the BC name for use by ADS: NN_AdsBcType
            sprintf(adsBcName, "%02i_%s", bcUsageCnt_[bc.tid - 1],
                BcNames[bc.tid - 1]);
            adsBcNames_[i] = adsBcName;
            ++bcUsageCnt_[bc.tid - 1];
        }
        // usedPrefixPairs_ is loaded, make sure 13/14 periodic BCs have a mate.
        for (PWP_UINT32 i = 0; i < domainCount; ++i) {
            GetBcData(rti_, i, bc);
            PWP_UINT32 targetId = 0;
            switch (bc.tid) {
            case 13:
                targetId = 100 * bc.id + 14;
                break;
            case 14:
                targetId = 100 * bc.id + 13;
                break;
            default:
                continue;
            }
            if (usedPrefixPairs_.end() == usedPrefixPairs_.find(targetId)) {
                // Could not find matching BC in pair!
                std::ostringstream msg;
                msg << "PERIODIC FACE '" << bc.name << "' (" << bc.tid << ":"
                    << bc.id << ") does not have a match";
                caeuSendWarningMsg(&rti_, msg.str().c_str(), ++warnId);
            }
        }

        // Determine the number of dependent variables; ndVar_
        PWGM_CONDDATA condData;
        PWP_UINT32 ndx = 0;
        ndVar_ = 1;
        bool hasMixedVcTypes = false;
        while(!rti_.opAborted && PwBlkCondition(PwModEnumBlocks(rti_.model,
            ndx++), &condData)) {
            if (0 == condData.tid) {
                // ignore Unsepcified VCs
            }
            else if (1 == ndx) {
                // Always capture first VC tid value
                ndVar_ = condData.tid;
            }
            else if (ndVar_ != condData.tid) {
                hasMixedVcTypes = true;
                // Capture largest NDVAR value
                if (ndVar_ < condData.tid) {
                    ndVar_ = condData.tid;
                }
            }
        }
        if (hasMixedVcTypes) {
            caeuSendWarningMsg(&rti_, "This export contains mixed VC types!",
                ++warnId);
        }

        if (0 != warnId) {
            caeuSendWarningMsg(&rti_, "done!", 0);
        }
        return ret;
    }


    PWP_UINT32 getCDtid(PWGM_HDOMAIN dom) const
    {
        PWP_UINT32 ret = 0;
        if (PWGM_HDOMAIN_ISVALID(dom)) {
            ret = prefix_.at(PWGM_HDOMAIN_ID(dom));
        }
        return ret;
    }


    inline PWP_UINT32 getNDVAR() const
    {
        return ndVar_;
    }


    inline const char *getAdsBcName(PWP_UINT32 ndx) const
    {
        return adsBcNames_.at(ndx).c_str();
    }


private:

    // Runtime information
    CAEP_RTITEM &rti_;

    // Global sequence id assigned to each boundary domain. There is one item
    // for each boundary domain in the grid model.
    UINT32Vec   prefix_;

    // Number of times each BC is used in grid model. There is one item for
    // each BC type supported (plus one error item).
    PWP_UINT32  bcUsageCnt_[NumBcs + 1];

    // BC names formatted for ADS: "<NN>_<AdsBcTypeName>". Where, NN is the
    // usage count for <AdsBcTypeName>.
    StringVec   adsBcNames_;

    // Set of already used prefix_ values for paired BC types 13 and 14.
    // Duplicate id usage generates a warning.
    UINT32Set   usedPrefixPairs_;

    // Number of dependent variables
    PWP_UINT32  ndVar_;
};


static void
closeFile(CAEP_RTITEM &rti)
{
    if (0 != rti.fp) {
        pwpFileClose(rti.fp);
        rti.fp = 0;
    }
}


static bool
openFile(CAEP_RTITEM &rti, const char *ext, PWP_ENUM_ENCODING encoding)
{
    closeFile(rti);
    std::string fname(rti.pWriteInfo->fileDest);
    if (ext && ext[0]) {
        fname += ".";
        fname += ext;
    }
    int mode = pwpWrite;
    if (PWP_ENCODING_BINARY == encoding) {
        mode |= pwpBinary;
    }
    else {
        mode |= pwpAscii;
    }
    rti.fp = pwpFileOpen(fname.c_str(), mode);
    return 0 != rti.fp;
}


static bool
writeTitle(CAEP_RTITEM &rti)
{
    // get the title from set attribute -> title
    const char* title;
    PwModGetAttributeString(rti.model, attrTitle, &title);
    if (CAEPU_RT_ENC_BINARY(&rti)) {
        // Write left-justified, 80 character, space-padded string
        fprintf(rti.fp, "%-80.80s", title);
    }
    else {
        fprintf(rti.fp, "%s\n\n", title);
    }
    return true;
}


static inline void
writeArray(CAEP_RTITEM &rti, PWP_UINT32 *var, PWP_UINT32 count, int fldWd = 1)
{
    if (CAEPU_RT_ENC_BINARY(&rti)) {
        fwrite(var, sizeof(PWP_UINT32), count, rti.fp);
    }
    else {
        PWP_UINT32 i;
        for (i = 0; i < count - 1; ++i) {
            fprintf(rti.fp, "%*lu ", fldWd, (unsigned long)var[i]);
        }
        // write last value with newline
        fprintf(rti.fp, "%*lu\n", fldWd, (unsigned long)var[i]);
    }
}


static inline void
writeArray(CAEP_RTITEM &rti, float *var, PWP_UINT32 count)
{
    if (CAEPU_RT_ENC_BINARY(&rti)) {
        fwrite(var, sizeof(float), count, rti.fp);
    }
    else {
        PWP_UINT32 i;
        for (i = 0; i < count - 1; ++i) {
            fprintf(rti.fp, "%9f ", var[i]);
        }
        // write last value with newline
        fprintf(rti.fp, "%9f\n", var[i]);
    }
}


static bool
writeZeroLine(CAEP_RTITEM &rti)
{
    const PWP_UINT32 VARSZ = 15;
    // init var[] to all zeros
    PWP_UINT32 var[VARSZ] = { 0 };
    writeArray(rti, var, VARSZ);
    return true;
}


static PWP_UINT32
countElements(CAEP_RTITEM &rti) // NEL
{
    PWP_UINT32 eTot = 0; // total element count
    PWGM_ELEMCOUNTS eCounts; // element count bin
    PWP_UINT32 ndx = 0; // block index
    PWGM_HBLOCK hBlock = PwModEnumBlocks(rti.model, ndx); // block handle
    while (PWGM_HBLOCK_ISVALID(hBlock)) {
        eTot += PwBlkElementCount(hBlock, &eCounts);
        hBlock = PwModEnumBlocks(rti.model, ++ndx);
    }
    return eTot;
}


static PWP_UINT32
countBoundaryFaces(CAEP_RTITEM &rti) // NBCL
{
    PWP_UINT32 eTot = 0;
    PWGM_ELEMCOUNTS eCounts;
    PWP_UINT32 iDomain = 0;
    PWGM_HDOMAIN hDomain = PwModEnumDomains(rti.model, iDomain);
    while (PWGM_HDOMAIN_ISVALID(hDomain)) {
        eTot += PwDomElementCount(hDomain,&eCounts);
        hDomain = PwModEnumDomains(rti.model,++iDomain);
    }
    return eTot;
}


static bool
writeFirstLine(CAEP_RTITEM &rti)
{
    // nsector,nisbc,ndvar,imesh,niter,itime,inrit,units,nitsc,nbk,5xIDM
    const PWP_UINT32 VARSZ = 15;
    // init var[] to all zeros
    PWP_UINT32 var[VARSZ] = { 0 };
    // set values accordingly
    var[0] = 1; // NSECTIONS
    var[2] = rti.adsData->getNDVAR(); // NDVAR
    var[6] = PwModBlockCount(rti.model); // NBK
    writeArray(rti, var, VARSZ);
    return true;
}


static bool
writeSecondLine(CAEP_RTITEM &rti)
{
    // rinpax,uinpav,gc,gal,dtin,tmacc,9xIDM
    return writeZeroLine(rti);
}


static bool
writeThirdLine(CAEP_RTITEM &rti)
{
    // nnl,nel,nbcl,ngrid,ncdut,nsfl,bksl,8xIDM
    const PWP_UINT32 VARSZ = 15;
    // init var[] to all zeros
    PWP_UINT32 var[VARSZ] = { 0 };
    // set values accordingly
    var[0] = PwModVertexCount(rti.model); // NNL
    var[1] = countElements(rti); // NEL
    var[2] = countBoundaryFaces(rti); // NBCL
    var[4] = rti.adsData->getNDVAR(); // NCDUT
    writeArray(rti, var, VARSZ);
    return true;
}


static bool
writeFourthLine(CAEP_RTITEM &rti)
{
    // rpms,pgaug,egaug,fvs,sfrl,twal,pnll,pntl,tfree,lfree,5xIDM
    return writeZeroLine(rti);
}


static bool
writeVertices(CAEP_RTITEM &rti)
{
    const PWP_UINT32 VARSZ = 15;
    const float DVAR = 0.0;
    const float PSND = 0.0;

    bool ret = false;
    PWP_UINT32 NVAR = rti.adsData->getNDVAR();
    // Array bounds check. Use >= to allow for PSND value
    if (NVAR >= VARSZ) {
        CAEPU_RT_ABORT(&rti);
    }
    else {
        // Number of floats to write for each vertex == xyz + dep var count
        PWP_UINT32 count = 3 + NVAR;

        // init var[] to all zeros. +3 to allow for XYZ. The values after XYZ
        // don't change. Set them once up front.
        float var[3 + VARSZ] = { 0.0 };
        for (PWP_UINT32 i = 3; i < count; ++i) {
            var[i] = DVAR;
        }

        if (1 != NVAR) {
            // post-incr count so PSND gets written too
            var[count++] = PSND;
        }

        if (caeuProgressBeginStep(&rti, PwModVertexCount(rti.model))) {
            ret = true;
            PWGM_VERTDATA v;
            PWP_UINT32 vNdx = 0;
            while (PwVertDataMod(PwModEnumVertices(rti.model, vNdx++), &v)) {
                // update XYZ values
                var[0] = float(v.x);
                var[1] = float(v.y);
                var[2] = float(v.z);
                writeArray(rti, var, count);
                if (!caeuProgressIncr(&rti)) {
                    ret = false;
                    break;
                }
            }
        }
        caeuProgressEndStep(&rti);
    }
    return ret;
}


static bool
writeConnectivity(CAEP_RTITEM &rti)
{
    bool ret = false;
    PWP_UINT32 elemCnt = PwModEnumElementCount(rti.model, 0);
    if (caeuProgressBeginStep(&rti, elemCnt)) {
        ret = true;
        PWP_UINT32 j;
        PWP_UINT32 ndx[PWGM_ELEMDATA_VERT_SIZE];
        PWGM_ELEMDATA eData;
        PWP_UINT32 eNdx = 0;
        // iterate over all elements
        while (PwElemDataMod(PwModEnumElements(rti.model, eNdx++), &eData)) {
            for (j = 0; j < eData.vertCnt; ++j) {
                // ADS uses 1-based indices
                ndx[j] = eData.index[j] + 1;
            }
            // repeat last index to fill out all PWGM_ELEMDATA_VERT_SIZE values
            for (; j < PWGM_ELEMDATA_VERT_SIZE; ++j) {
                ndx[j] = ndx[eData.vertCnt - 1];
            }
            writeArray(rti, ndx, PWGM_ELEMDATA_VERT_SIZE, 5);
            if (!caeuProgressIncr(&rti)) {
                ret = false;
                break;
            }
        }
    }
    caeuProgressEndStep(&rti);
    return ret;
}


static PWP_UINT32
fixFace(PWGM_ENUM_ELEMTYPE eType, PWP_UINT32 pwFaceNdx)
{
    const PWP_UINT32 badIndex = 9999;
    PWP_UINT32 adsFaceNdx;
    switch (eType) {
    case PWGM_ELEMTYPE_TET: {
        static const PWP_UINT32 tetMap[] = { 4, 3, 1, 2, badIndex };
        adsFaceNdx = tetMap[pwFaceNdx < 4 ? pwFaceNdx : 4];
        break; }
    case PWGM_ELEMTYPE_HEX: {
        static const PWP_UINT32 hexMap[] = { 5, 6, 3, 2, 4, 1, badIndex };
        adsFaceNdx = hexMap[pwFaceNdx < 6 ? pwFaceNdx : 6];
        break; }
    case PWGM_ELEMTYPE_WEDGE: {
        static const PWP_UINT32 wedgeMap[] = { 5, 1, 3, 4, 2, badIndex };
        adsFaceNdx = wedgeMap[pwFaceNdx < 5 ? pwFaceNdx : 5];
        break; }
    case PWGM_ELEMTYPE_PYRAMID:
        adsFaceNdx = pwFaceNdx + 1;
        break;
    default:
        adsFaceNdx = badIndex;
        break;
    }
    return adsFaceNdx;
}


PWP_UINT32 beginCB(PWGM_BEGINSTREAM_DATA *data)
{
    // set starting progress step count
    CAEP_RTITEM *pRti = (CAEP_RTITEM*)data->userData;
    return caeuProgressBeginStep(pRti, data->totalNumFaces);
}


PWP_UINT32 faceCB(PWGM_FACESTREAM_DATA *data)
{
    PWP_UINT32 ret = 0;
    PWGM_ELEMDATA faceElemData;
    if (PwElemDataMod(data->owner.blockElem, &faceElemData)) {
        CAEP_RTITEM &rti = *((CAEP_RTITEM*)data->userData);
        PWP_UINT32 var[3];
        // The cell's index in the model's index space (1..totalNumCells)
        var[0] = data->owner.cellIndex + 1; // cellID
        // Convert from PW local face id to ADS local face id
        var[1] = fixFace(faceElemData.type, data->owner.cellFaceIndex);
        // Get the domains ADS type id
        var[2] = rti.adsData->getCDtid(data->owner.domain);
        writeArray(rti, var, 3);
        ret = caeuProgressIncr(&rti);
    }
    return ret;
}


PWP_UINT32 endCB(PWGM_ENDSTREAM_DATA *data)
{
    // end progress step
    return caeuProgressEndStep((CAEP_RTITEM*)data->userData);
}


static bool
writeBC(CAEP_RTITEM &rti)
{
    PWGM_ENUM_FACEORDER order = PWGM_FACEORDER_BCGROUPSONLY;
    return 0 != PwModStreamFaces(rti.model, order, beginCB, faceCB, endCB,
        &rti);
}


static bool
exportBCVAL(CAEP_RTITEM &rti)
{
    // Dump info comment header 
    fputs(
        "******************************************************************\n"
        "*BOUNDARY TYPE    DEFINITION                                     *\n"
        "*00                No value applied                              *\n"
        "*01                Reserved                                      *\n"
        "*02                Inflow                                        *\n"
        "*03                Outflow                                       *\n"
        "*04                Reserved                                      *\n"
        "*05                Reserved                                      *\n"
        "******************************************************************\n",
        rti.fp);

    PWP_UINT32 domainCount = PwModDomainCount(rti.model);
    fprintf(rti.fp, "*NUMBER OF BOUNDARY CONDITIONS\n"
                      "%12i\n", (int)domainCount);

    bool ret = true;
    PWGM_CONDDATA bc;
    for (PWP_UINT32 i = 0; i < domainCount; ++i) {
        if (!GetBcData(rti, i, bc)) {
            ret = false;
        }
        fputs("*BOUNDARY_TYPE BOUNDARY_NAME                        IFANG\n",
              rti.fp);
        fprintf(rti.fp, " %-14i%-37s%-12i\n", 0, bc.name, 0);
        fputs("*           MLO           PTLO           TTLO          ALPHA"
                    "           BETA\n"
              "      0.0000000      0.0000000      0.0000000      0.0000000"
                    "      0.0000000\n",
              rti.fp);
    }
    return ret;
}


static bool
exportBCTYPE(CAEP_RTITEM &rti)
{
    // Dump info comment header 
    fputs(
        "******************************************************************\n"
        "***ADS BC NAME***********DESCRIPTION******************************\n"
        "******************************************************************\n"
        "*** XX_INTERNAL     --> 'INTERIOR'                             ***\n"
        "*** XX_FLOWTAN      --> 'FLOW TANGENCY'                        ***\n"
        "*** XX_INFLOW       --> 'UP-STREAM'                            ***\n"
        "*** XX_OUTFLOW      --> 'DOWN-STREAM'                          ***\n"
        "*** XX_FARFIELD     --> 'FARFIELD IN/OUT'                      ***\n"
        "*** XX_NRINFLOW     --> 'CASCADE UP-STREAM'                    ***\n"
        "*** XX_NROUTFLOW    --> 'CASCADE DOWN-STREAM'                  ***\n"
        "*** XX_INVISCID     --> 'NO FLOW NORMAL TO THE SURFACE'        ***\n"
        "*** XX_WALLF        --> 'WALL FUNCTION'                        ***\n"
        "*** XX_WALLIQ       --> 'NON-SLIP WALL WITH Q INPUT'           ***\n"
        "*** XX_WALLIA       --> 'NON-SLIP WALL Q = 0.0'                ***\n"
        "*** XX_WALLIS       --> 'NON-SLIP WALL IN STATIONARY FRAME'    ***\n"
        "*** XX_WALLFCHT     --> 'WALL FUNCTION'                        ***\n"
        "*** XX_WALLIQCHT    --> 'NON-SLIP WALL WITH Q INPUT'           ***\n"
        "*** XX_WALLIACHT    --> 'NON-SLIP WALL Q = 0.0'                ***\n"
        "*** XX_WALLISCHT    --> 'NON-SLIP WALL IN STATIONARY FRAME'    ***\n"
        "*** XX_NMLINFLOW    --> 'INLET FLOW NORMAL TO MESH'            ***\n"
        "*** XX_PERIODIC     --> 'PERIODIC FACE'                        ***\n"
        "*** XX_PERIODICSHDW --> 'PERIODIC SHADOW FACE'                 ***\n"
        "*** XX_INTSCT       --> 'INTERSECTOR PERIODIC FACE'            ***\n"
        "*** XX_INTSCTSHDW   --> 'INTERSECTOR PERIODIC SHADOW FACE'     ***\n"
        "*** XX_ISOTHERMAL   --> 'CONSTANT T WALL FOR HEAT CONDUCTION'  ***\n"
        "*** XX_ADIABATIC    --> 'ADIABATIC WALL FOR HEAT CONDUCTION'   ***\n"
        "*** XX_FMVINFLOW    --> 'UPSTREAM WITH FLOATING MERIDIONAL V'  ***\n"
        "******************************************************************\n"
        "\n", rti.fp);

    bool ret = true; // assume all is okay
    PWP_UINT32 domainCount = PwModDomainCount(rti.model);
    fprintf(rti.fp, "*NUMBER OF BOUNDARY CONDITIONS\n"
                      "%-12i\n", (int)domainCount);
    PWGM_CONDDATA bc;
    for (PWP_UINT32 i = 0; i < domainCount; i++) {
        if (!GetBcData(rti, i, bc)) {
            ret = false;
        }
        fputs("*BC NUMBER,    CFX NAME,                           ADS NAME\n",
            rti.fp);
        fprintf(rti.fp, "%-15i%-36s%-12s\n", int(i + 1), bc.name,
            rti.adsData->getAdsBcName(i));
    }
    return ret;
}


PWP_BOOL
runtimeCreate(CAEP_RTITEM *)
{
    return caeuAssignInfoValue("AllowedFileByteOrders", "LittleEndian", true) &&
        caeuPublishValueDefinition(attrTitle, PWP_VALTYPE_STRING, "", "RW",
            "Case Name", "/^.+$/");
}


static bool
writeRestFile(CAEP_RTITEM &rti)
{
    bool ret = openFile(rti, "REST", rti.pWriteInfo->encoding);
    if (ret) {
        ret = writeTitle(rti) && writeFirstLine(rti) && writeSecondLine(rti) &&
            writeThirdLine(rti) && writeFourthLine(rti) &&
            writeVertices(rti) && writeConnectivity(rti) && writeBC(rti);
        closeFile(rti);
    }
    return ret && !CAEPU_RT_IS_ABORTED(&rti);
}


static bool
writeBcValFile(CAEP_RTITEM &rti)
{
    bool ret = openFile(rti, "BCVAL", PWP_ENCODING_ASCII);
    if (ret) {
        ret = exportBCVAL(rti);
        closeFile(rti);
    }
    return ret && !CAEPU_RT_IS_ABORTED(&rti);
}


static bool
writeBcTypeFile(CAEP_RTITEM &rti)
{
    bool ret = openFile(rti, "BCTYPE", PWP_ENCODING_ASCII);
    if (ret) {
        ret = exportBCTYPE(rti);
        closeFile(rti);
    }
    return ret && !CAEPU_RT_IS_ABORTED(&rti);
}


static bool
doStartup(CAEP_RTITEM &rti)
{
    bool ret = (rti.BCCnt == NumBcs);
    if (!ret) {
        caeuSendErrorMsg(&rti, "BC count mismatch!", 0);
    }
    return ret;
}


static bool
doCleanup(CAEP_RTITEM &rti)
{
    closeFile(rti);
    return true;
}


PWP_BOOL
runtimeWrite( CAEP_RTITEM *pRti, PWGM_HGRIDMODEL /*model*/,
    const CAEP_WRITEINFO * /*pWriteInfo*/)
{
    ADSData adsData(*pRti);
    return doStartup(*pRti) && adsData.init() && caeuProgressInit(pRti, 3) &&
        writeRestFile(*pRti) && writeBcValFile(*pRti) &&
        writeBcTypeFile(*pRti) && doCleanup(*pRti);
}


PWP_VOID
runtimeDestroy(CAEP_RTITEM * )
{
}

/****************************************************************************
 *
 * This file is licensed under the Cadence Public License Version 1.0 (the
 * "License"), a copy of which is found in the included file named "LICENSE",
 * and is distributed "AS IS." TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
 * LAW, CADENCE DISCLAIMS ALL WARRANTIES AND IN NO EVENT SHALL BE LIABLE TO
 * ANY PARTY FOR ANY DAMAGES ARISING OUT OF OR RELATING TO USE OF THIS FILE.
 * Please see the License for the full text of applicable terms.
 *
 ****************************************************************************/
