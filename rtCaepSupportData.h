/****************************************************************************
 *
 * Pointwise Plugin utility functions
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2014 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _RTCAEPSUPPORTDATA_H_
#define _RTCAEPSUPPORTDATA_H_

/*! \cond */

/*------------------------------------*/
/* CaeUnsADS format item setup data */
/*------------------------------------*/
CAEP_BCINFO CaeUnsADSBCInfo[] = {
    { "1  Flow tangency", 1 },
    { "2  Up-stream", 2 },
    { "-2 Up-stream, constant tangential velocity", 3 },
    { "3  Down-stream", 4 },
    { "4  Farfield in/out", 5 },
    { "5  Cascade up-stream", 6 },
    { "6  Cascade down-stream", 7 },
    { "7  No flow normal to the surface", 8 },
    { "8  Wall function", 9 },
    { "9  Non-slip wall with Q input", 10 },
    { "10 Non-slip wall Q = 0.0 ", 11 },
    { "11 Non-slip wall in stationary frame", 12 },
    { "12 Inlet flow normal to mesh", 13 },
    { "13 Periodic face", 14 },
    { "14 Periodic with face 13", 15 },
    { "15 Constant T wall for heat conduction", 16 },
    { "16 Adiabatic wall for heat conduction", 17 },
    { "17 Intersector periodic with face", 18 },
    { "18 Intersector periodic with face 17", 19 },
    { "2008 Wall function - CHT", 20 },
	{ "32 Mass Flow Inlet", 21},
	{ "21 Non-Matching Periodic Face", 22 },
	{ "22 Non-Matching Periodic Shadow Face", 23 },
	{ "25 Structured Inlet Face on Unstructured Mesh", 24 },
	{ "26 Structured Outlet Face on Unstructured Mesh", 25 },
	{ "35 Turbomachinery Inlet for Unstructured Mesh", 26 },
	{ "36Turbomachinery Outlet for Unstructured Mesh", 27 },
	{ "47 Internal Interpolation Face", 28 },
	{ "48 Internal Interpolation Shadow Face", 29 },
	{ "19 Intersector Face", 30 },
	{ "20 Intersector Shadow Face", 31 },
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
