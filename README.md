# tinyply
![build](https://github.com/vsukhor/tinyply/actions/workflows/cmake.yml/badge.svg)
[![License is Unlicense](http://img.shields.io/badge/license-Unlicense-blue.svg?style=flat)](http://unlicense.org/)

A zero-dependency (except the C++ STL) __public domain__ implementation of the PLY file format. It is convenient for import/export of geometry or topology data like graphs, meshes, polymer structures, etc. This repo is a refactores version derived from C++ [implementation](https://github.com/ddiakopoulos/tinyply) by [Dimitri Diakopoulos](http://www.dimitridiakopoulos.com). An excellent overview and the definition of the PLY format is available [here](http://paulbourke.net/dataformats/ply/) along with its reference implementation in C. PLY is often used in the computer vision and graphics communities for its relative simplicity, ability to support arbitrary mesh attributes, and binary modes. Famously, PLY is used to distribute 3D models in the [Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/), including the bunny.

This version of the library uses elements of C++20 and so requires a relatively recent compiler (e.g. GCC 14 is sufficient). _tinyply_ supports exporting and importing PLY files in both binary and ascii formats. It may be used for filesizes >= 4gb and can read big-endian binary files (but not write them).

## Usage

_tinyply_ can be used both as a header-only and as a precompiled library.
Most conveniently, the switch between the two is done by defining TINYPLY_AS_LIBRARY.

If on the ohter hand you prefer to use the headers alone, you may just include either 'tinyply.h' or each of 'reader.h' or 'writer.h'
directly into your project.

## Getting Started

The project comes with a simple example program demonstrating a circular write / read and all of the major API functionality.

## License

This software is in the public domain. Where that dedication is not recognized, you are granted a perpetual, irrevocable license to copy, distribute, and modify this file as you see fit. If these terms are not suitable to your organization, you may choose to license it under the terms of the 2-clause simplified BSD.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
