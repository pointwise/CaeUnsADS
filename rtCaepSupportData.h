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

#ifndef _RTCAEPSUPPORTDATA_H_
#define _RTCAEPSUPPORTDATA_H_

/*! \cond */

/*                   Pointwise
    ADS BC Type      BC Type                                        tid
    --------------   --------------------------------------------   ---
    "FLOWTAN"        "1  Flow tangency"                               1
    "INFLOW"         "2  Up-stream"                                   2
    "OUTFLOW"        "3  Down-stream"                                 3
    "FARFIELD"       "4  Farfield in/out"                             4
    "NRINFLOW"       "5  Cascade up-stream"                           5
    "NROUTFLOW"      "6  Cascade down-stream"                         6
    "INVISCID"       "7  No flow normal to the surface"               7
    "WALLF"          "8  Wall function"                               8
    "WALLIQ"         "9  Non-slip wall with Q input"                  9
    "WALLIA"         "10 Non-slip wall Q = 0.0 "                     10
    "WALLIS"         "11 Non-slip wall in stationary frame"          11
    "NMLINFLOW"      "12 Inlet flow normal to mesh"                  12
    "PERIODIC"       "13 Periodic face"                              13
    "PERIODICSHDW"   "14 Periodic with face 13"                      14
    "ISOTHERMAL"     "15 Constant T wall for heat conduction"        15
    "ADIABATIC"      "16 Adiabatic wall for heat conduction"         16
    "INTSCT"         "17 Intersector periodic with face"             17
    "INTSCTSHDW"     "18 Intersector periodic with face 17"          18
    "WALLFCHT"       "2008 Wall function - CHT"                      19
    "FMVINFLOW"      "-2 Upstream with floating meridional V"        20
*/

/*------------------------------------*/
/* CaeUnsADS format item setup data */
/*------------------------------------*/
CAEP_BCINFO CaeUnsADSBCInfo[] = {
    { "1  Flow tangency", 1 },
    { "2  Up-stream", 2 },
    { "-2 Upstream with floating meridional V", 20 },
    { "3  Down-stream", 3 },
    { "4  Farfield in/out", 4 },
    { "5  Cascade up-stream", 5 },
    { "6  Cascade down-stream", 6 },
    { "7  No flow normal to the surface", 7 },
    { "8  Wall function", 8 },
    { "9  Non-slip wall with Q input", 9 },
    { "10 Non-slip wall Q = 0.0 ", 10 },
    { "11 Non-slip wall in stationary frame", 11 },
    { "12 Inlet flow normal to mesh", 12 },
    { "13 Periodic face", 13 },
    { "14 Periodic with face 13", 14 },
    { "15 Constant T wall for heat conduction", 15 },
    { "16 Adiabatic wall for heat conduction", 16 },
    { "17 Intersector periodic with face", 17 },
    { "18 Intersector periodic with face 17", 18 },
    { "2008 Wall function - CHT", 19}
};
/*------------------------------------*/
CAEP_VCINFO CaeUnsADSVCInfo[] = {
    { "Fluid", 5 },
    { "Solid", 1 },
};
/*------------------------------------*/
const char *CaeUnsADSFileExt[] = {
     "REST", "BCTYPE", "BCVAL"
};
/*! \endcond */

#endif /* _RTCAEPSUPPORTDATA_H_ */

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
