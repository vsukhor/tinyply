# tinyply

![build](https://github.com/vsukhor/tinyply/actions/workflows/cmake.yml/badge.svg)
[![License: public domain](https://img.shields.io/badge/license-public%20domain-blue.svg?style=flat)](LICENSE)

A zero-dependency (except the C++ standard library), **public domain** C++20 implementation of the PLY file format. It is convenient for importing and exporting geometry or topology data such as meshes, graphs and polymer structures.

This repository is a refactored version of Dimitri Diakopoulos's [tinyply](https://github.com/ddiakopoulos/tinyply) 2.3.4.

PLY is widely used in computer vision and graphics for its simplicity, its support for arbitrary per-element attributes, and its binary modes. It is, for example, the format of the [Stanford 3D Scanning Repository](https://graphics.stanford.edu/data/3Dscanrep/), home of the Stanford bunny. The current version is made for my personal use, as e.g. it is the default io lib of [graffine](https://github.com/vsukhor/graffine). Paul Bourke's [PLY format description](https://paulbourke.net/dataformats/ply/) covers the format in detail and provides its reference implementation in C.

## Features

- Reads and writes both ASCII and binary PLY files.
- Reads big-endian binary files; writes little-endian only.
- Handles files larger than 4 GB.
- Reports errors, including malformed or truncated input, through exceptions.

## Requirements

- A C++20 compiler (tested with GCC-14, GCC-16).
- CMake 3.28 or later, to build the library and the example.

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

This builds the static library and the example program. Pass `-DSHARED_LIB=ON` for a shared library, or `-DLIB=OFF` to skip the library. After building, the library is copied to `lib/` and the example to `examples/bin/` in the source tree.

To build and run the tests, which need no external files:

```sh
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage

The headers include each other as `"impl/..."`, so add `include/tinyply` (not `include/`) to your include path.

The implementation is compiled only where `TINYPLY_AS_LIBRARY` is defined. There are two ways to use it:

- **Header-only:** in exactly one `.cpp` file, `#define TINYPLY_AS_LIBRARY` before including `tinyply.h`. Include it without the macro everywhere else.
- **Precompiled library:** link against `libtinyply` (built from `source/tinyply.cpp`) and include the headers without defining the macro.

Include `tinyply.h` for both reading and writing, or only `reader.h` or `writer.h`.

```cpp
#define TINYPLY_AS_LIBRARY  // header-only use: in exactly one .cpp file
#include "tinyply.h"

#include <array>
#include <cstring>
#include <fstream>
#include <vector>

// Write
std::vector<std::array<float, 3>> vertices {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
std::vector<std::array<uint32_t, 3>> triangles {{0, 1, 2}};

tinyply::Writer out;
out.add_properties_to_element("vertex", {"x", "y", "z"},
                              tinyply::Type::FLOAT32, vertices.size(),
                              reinterpret_cast<const uint8_t*>(vertices.data()),
                              tinyply::Type::INVALID, 0);    // not a list
out.add_properties_to_element("face", {"vertex_indices"},
                              tinyply::Type::UINT32, triangles.size(),
                              reinterpret_cast<const uint8_t*>(triangles.data()),
                              tinyply::Type::UINT8, 3);      // list: uchar count, 3 items
out.write("triangle.ply", true);                             // true: binary, false: ASCII

// Read
std::ifstream is("triangle.ply", std::ios::binary);
tinyply::Reader in;
in.parse_header(is);
auto xyz   = in.request_properties_from_element("vertex", {"x", "y", "z"});
auto faces = in.request_properties_from_element("face", {"vertex_indices"});
in.read(is);

// Properties requested together share one interleaved buffer: x0 y0 z0 x1 ...
std::vector<std::array<float, 3>> loaded(xyz->count);
std::memcpy(loaded.data(), xyz->buffer.get(), xyz->buffer.size_bytes());
```

Notes:

- The writer stores pointers to your data without copying it, so the data must stay alive until `write()` is called.
- Properties requested together must share the same scalar type. Their values are interleaved in the order they appear in the file, not the order requested.
- Without a list size hint (the optional last argument of `request_properties_from_element`), `read()` parses the file twice and needs a seekable stream. Passing the known list length, e.g. `3` for triangle meshes, makes it a single pass.
- `Reader::get_elements()` lists the elements and properties in the header, so you can choose what to request at run time.

## Example

[examples/cube.cpp](examples/cube.cpp) writes a cube mesh to ASCII, reads it back, writes it as binary and reads it again. It demonstrates all of the major API functionality, including converting the read buffers into your own types. Run it from the directory where the `example_cube*.ply` output files should go:

```sh
examples/bin/cube
```

## Limitations

- Lists must have the same length in every record of an element, so meshes that mix triangles and quads cannot be read. Such lists can still be skipped by not requesting them.
- The writer outputs fixed-length lists only.
- 64-bit integer property types are not supported.
- Little-endian hosts only.

## License

This software is in the public domain, with the option of the 2-clause BSD license where public domain is not recognized or not suitable. See [LICENSE](LICENSE) for the full terms.
