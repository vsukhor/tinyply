/*
 * This file is derived from
 * tinyply 2.3.4 (https://github.com/ddiakopoulos/tinyply)
 *
 * A zero-dependency (except the C++ STL) public domain implementation
 * of the PLY file format. Requires C++20; errors are handled through exceptions.
 *
 * This software is in the public domain. Where that dedication is not
 * recognized, you are granted a perpetual, irrevocable license to copy,
 * distribute, and modify this file as you see fit.
 *
 * Authored by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
 * Updated by Valerii Sukhorukov (vsukhorukov@yahoo.com, https://github.com/vsukhor)
 */

// Tinyply can be used both as a header-only and as a precompiled library.
// This file is only needed if you prefer the latter.
// If on the ohter hand you prefer to use the headers alone, you may undef this
// and just include either 'tinyply.h' or each of 'reader.h' or 'writer.h'
// directly into your project and define this:
#define TINYPLY_AS_LIBRARY

#include "reader.h"
#include "writer.h"
