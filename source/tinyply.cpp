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
// This file is only needed if you prefer the latter - to create a nice library.
// Regularly, this switch is done via cmake, as also the choice between the
// static and shared versions. The definitions here are just to provide a more
// direct control.
// If on the ohter hand you prefer to use the headers alone, you may undef this
// and just include either 'tinyply.h' or each of 'reader.h' or 'writer.h'
// directly into your project.

#ifndef TINYPLY_LINK_AS_LIBRARY
#define TINYPLY_LINK_AS_LIBRARY
#endif

#include "reader.h"
#include "writer.h"
