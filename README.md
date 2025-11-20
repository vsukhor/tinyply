# tinyply
![build](https://github.com/vsukhor/tinyply/actions/workflows/cmake.yml/badge.svg)
[![License is Unlicense](http://img.shields.io/badge/license-Unlicense-blue.svg?style=flat)](http://unlicense.org/)

tinyply

A zero-dependency (except the C++ STL) __public domain__ implementation of the PLY file format convenient for import/export of geometry or topology data like graphs, meshes, polymer structures, etc. This is a restructured version based on C++ [implementation](http://paulbourke.net/dataformats/ply/) by [Dimitri Diakopoulos](http://www.dimitridiakopoulos.com). An overview and definition of the file format is available [here](http://paulbourke.net/dataformats/ply/) along with the reference implementation in C. PLY format is often used in the computer vision and graphics communities for its relative simplicity, ability to support arbitrary mesh attributes, and binary modes. Famously, PLY is used to distribute 3D models in the [Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/), including the bunny.

This version of the library uses elements of C++20 and so requires a relatively recent compiler (e.g. GCC 14 is sufficient). _tinyply_ supports exporting and importing PLY files in both binary and ascii formats. It may be used for filesizes >= 4gb and can read big-endian binary files (but not write them).

## Usage

_tinyply_ can be used both as a header-only and as a precompiled library.
Most conveniently, the swithc between the two is done via cmake by defining TINYPLY_LINK_AS_LIBRARY.

If on the ohter hand you prefer to use the headers alone, you may just include either 'tinyply.h' or each of 'reader.h' or 'writer.h'
directly into your project.

## Getting Started

The project comes with a simple example program demonstrating a circular write / read and all of the major API functionality.

## In The Wild

Since 2015, original version of tinyply has been used in hundreds of open-source projects including pointcloud tools, raytracers, synthetic data renderers, computational geometry libraries, and more. A few notable projects are highlighted below:

* [libigl](https://libigl.github.io/), a robust computational geometry library from UoT professors Alec Jacobson and Daniele Panozzo.
* [Maplab](https://github.com/ethz-asl/maplab) from ETH Zürich, a research-oriented visual-inertial mapping framework.
* [glChAoS.P](https://github.com/BrutPitt/glChAoS.P) from Michele Morrone, a rendering sandbox for 3D strange attractors.
* [Cilantro](https://github.com/kzampog/cilantro), a robust and featureful C++ library for working with pointcloud data.
* [HabitatAI](https://aihabitat.org/), Facebook's 3D simulator for training AI agents in photorealistic environments.
* [ScanNet](http://www.scan-net.org/), an RGB+D dataset of 2.5 million views across 1500 scans.
* [PlaneRCNN](https://github.com/NVlabs/planercnn), 3D plane detection via single-shot images from NVIDIA Research
* [KNOSSOS](https://knossos.app/), a framework to visualize and annotate 3D image data (neural morphology and connectivity).

tinyply not what you're looking for? tinyply trades some performance for simplicity and flexibility. For domain-specific uses (e.g. where your application does not need to handle arbitrary user-fed PLY files), there are other speedier alternatives. For more, please check out the following benchmarks:

* Vilya Harvey's [ply-parsing-perf](https://github.com/vilya/ply-parsing-perf)
* Maciej Halber's [ply_io_benchmark](https://github.com/mhalber/ply_io_benchmark)

## License

This software is in the public domain. Where that dedication is not recognized, you are granted a perpetual, irrevocable license to copy, distribute, and modify this file as you see fit. If these terms are not suitable to your organization, you may choose to license it under the terms of the 2-clause simplified BSD.
