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
 * Pointwise Plugin utility functions
 *
 ***************************************************************************/

#ifndef _RTCAEPINITITEMS_H_
#define _RTCAEPINITITEMS_H_

#if defined(DEBUG)
    #define DRVAL(d,r)  d
#else
    #define DRVAL(d,r)  r
#endif

/*! \cond */
/*.................................................
    initialize caepRtItem[0]
*/
#define ID_CaeUnsADS  140 // id is ADS is owner
{
    /*== CAEP_FORMATINFO FormatInfo */
    {   PWP_SITE_GROUPNAME,      /* const char *group */
        "Leo (unstructured)",             /* const char *name */
        MAKEGUID(ID_CaeUnsADS),  /* PWP_UINT32 id */

        PWP_FILEDEST_BASENAME,   /* PWP_ENUM_FILEDEST fileDest */

        PWP_FALSE,               /* PWP_BOOL allowedExportConditionsOnly */
        PWP_TRUE,                /* PWP_BOOL allowedVolumeConditions */

        DRVAL(PWP_TRUE, PWP_FALSE),  /* PWP_BOOL allowedFileFormatASCII */
        PWP_TRUE,                /* PWP_BOOL allowedFileFormatBinary */
        PWP_FALSE,               /* PWP_BOOL allowedFileFormatUnformatted */

        PWP_TRUE,               /* PWP_BOOL allowedDataPrecisionSingle */
        PWP_FALSE,              /* PWP_BOOL allowedDataPrecisionDouble */

        PWP_FALSE,              /* PWP_BOOL allowedDimension2D */
        PWP_TRUE                /* PWP_BOOL allowedDimension3D */
    },

    &pwpRtItem[1],  /* PWU_RTITEM* */

    /*== CAEP_BCINFO*    pBCInfo;    -- array of format BCs or NULL */
    /*== PWP_UINT32      BCCnt;      -- # format BCs */
    CaeUnsADSBCInfo,            /* CAEP_BCINFO* */
    ARRAYSIZE(CaeUnsADSBCInfo), /* PWP_UINT32 BCCnt */

    /*== CAEP_VCINFO*    pVCInfo;    -- array of format VCs or NULL */
    /*== PWP_UINT32      VCCnt;      -- # format VCs */
    CaeUnsADSVCInfo,            /* CAEP_VCINFO* pVCInfo */
    ARRAYSIZE(CaeUnsADSVCInfo), /* PWP_UINT32 VCCnt */

    /*== const char**    pFileExt;   -- array of valid file extensions */
    /*== PWP_UINT32      ExtCnt;     -- # valid file extensions */
    CaeUnsADSFileExt,            /* const char **pFileExt */
    ARRAYSIZE(CaeUnsADSFileExt), /* PWP_UINT32 ExtCnt */

    /*== PWP_BOOL  elemType[PWGM_ELEMTYPE_SIZE]; -- un/supported elem */
    {   PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_BAR] */
        PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_HEX] */
        PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_QUAD] */
        PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_TRI] */
        PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_TET] */
        PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_WEDGE] */
        PWP_TRUE,              /* elemType[PWGM_ELEMTYPE_PYRAMID] */
        PWP_TRUE },            /* elemType[PWGM_ELEMTYPE_POINT] */

    0,  /* FILE *fp */

    /* PWU_UNFDATA UnfData */
    {   0,          /* PWP_UINT32 status */
        0,          /* FILE *fp */
        0,          /* sysFILEPOS fPos */
        PWP_FALSE,  /* PWP_BOOL hadError */
        PWP_FALSE,  /* PWP_BOOL inRec */
        0,          /* PWP_UINT32 recBytes */
        0,          /* PWP_UINT32 totRecBytes */
        0    },     /* PWP_UINT32 recCnt */

    0,  /* PWGM_HGRIDMODEL model */

    0,  /* const CAEP_WRITEINFO *pWriteInfo */

    0,  /* PWP_UINT32 progTotal */
    0,  /* PWP_UINT32 progComplete */
    {0},  /* clock_t clocks[CAEPU_CLKS_SIZE]; */
    0,  /* PWP_BOOL opAborted */

    /* if you added any custom data in rtCaepInstanceData.h,
       you need to initialize it here. The init below matches the 
       example MY_CAEP_DATA struct given in rtCaepInstanceData.h */
    /*
    {   0,
        0,
        0.0,
        "string" },
    */
},
/*! \endcond */

#endif /* _RTCAEPINITITEMS_H_ */

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
