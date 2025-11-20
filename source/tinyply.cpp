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
