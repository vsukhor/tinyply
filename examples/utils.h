// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.
// https://github.com/ddiakopoulos/tinyply

// This file is only required for the example and test programs.

#pragma once

#ifndef TINYPLY_EXAMPLES_UTILS_H
#define TINYPLY_EXAMPLES_UTILS_H

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>

inline
std::vector<uint8_t> read_file_binary(const std::filesystem::path& file)
{
    std::ifstream ifs(file, std::ios::binary);
    std::vector<uint8_t> fileBufferBytes;

    if (ifs.is_open()) {

        ifs.seekg(0, std::ios::end);
        size_t sizeBytes = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        fileBufferBytes.resize(sizeBytes);
        if (ifs.read((char*)fileBufferBytes.data(), sizeBytes))
            return fileBufferBytes;
    }
    else
        throw std::runtime_error("could not open binary ifstream to path " + file.string());

    return fileBufferBytes;
}

struct memory_buffer
    : public std::streambuf {

    char* p_start {};
    char* p_end {};
    size_t size;

    memory_buffer(char const* first_elem,
                  const size_t size)
        : p_start {const_cast<char*>(first_elem)}
        , p_end {p_start + size}
        , size {size}
    {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(const off_type off,
                     std::ios_base::seekdir dir,
                     std::ios_base::openmode which) override
    {
        if (dir == std::ios_base::cur)
            gbump(static_cast<int>(off));
        else
            setg(p_start,
                (dir == std::ios_base::beg ? p_start : p_end) + off,
                p_end);

        return gptr() - p_start;
    }

    pos_type seekpos(const pos_type pos,
                     std::ios_base::openmode which) override
    {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct memory_stream
    : virtual memory_buffer
    , public std::istream
{
    memory_stream(char const * first_elem,
                  size_t size)
        : memory_buffer(first_elem, size)
        , std::istream(static_cast<std::streambuf*>(this))
    {}
};

class manual_timer {

    using Clock = std::chrono::high_resolution_clock;

    Clock::time_point t0;
    double timestamp{};

public:

    void start()
    {
        t0 = Clock::now();
    }

    void stop()
    {
        timestamp = std::chrono::duration<double>(Clock::now() - t0).count() * 1000.0;
    }

    const double& get() const
    {
        return timestamp;
    }
};

struct float2 { float x, y; };
struct float3 { float x, y, z; };
struct double3 { double x, y, z; };
struct uint3 { uint32_t x, y, z; };
struct uint4 { uint32_t x, y, z, w; };

struct geometry {

    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<float2> texcoords;
    std::vector<uint3> triangles;
};

struct Cube
    : public geometry {

    static constexpr size_t numVertices {8};

    struct Vertex {
        float3 position;
        float3 normal;
        float2 texCoord;
    };

    static constexpr std::array<uint4, 6> quads {{
        { 0, 1, 2, 3 },
        { 4, 5, 6, 7 },
        { 8, 9, 10, 11 },
        { 12, 13, 14, 15 },
        { 16, 17, 18, 19 },
        { 20, 21, 22, 23 }
    }};

    static constexpr Cube default_configuration() noexcept
    {
        constexpr std::array<Vertex, 3*numVertices> verts = {{
            { { -1, -1, -1 },{ -1, 0, 0 },{ 0, 0 } },
            { { -1, -1, +1 },{ -1, 0, 0 },{ 1, 0 } },
            { { -1, +1, +1 },{ -1, 0, 0 },{ 1, 1 } },
            { { -1, +1, -1 },{ -1, 0, 0 },{ 0, 1 } },

            { { +1, -1, +1 },{ +1, 0, 0 },{ 0, 0 } },
            { { +1, -1, -1 },{ +1, 0, 0 },{ 1, 0 } },
            { { +1, +1, -1 },{ +1, 0, 0 },{ 1, 1 } },
            { { +1, +1, +1 },{ +1, 0, 0 },{ 0, 1 } },

            { { -1, -1, -1 },{ 0, -1, 0 },{ 0, 0 } },
            { { +1, -1, -1 },{ 0, -1, 0 },{ 1, 0 } },
            { { +1, -1, +1 },{ 0, -1, 0 },{ 1, 1 } },
            { { -1, -1, +1 },{ 0, -1, 0 },{ 0, 1 } },

            { { +1, +1, -1 },{ 0, +1, 0 },{ 0, 0 } },
            { { -1, +1, -1 },{ 0, +1, 0 },{ 1, 0 } },
            { { -1, +1, +1 },{ 0, +1, 0 },{ 1, 1 } },
            { { +1, +1, +1 },{ 0, +1, 0 },{ 0, 1 } },

            { { -1, -1, -1 },{ 0, 0, -1 },{ 0, 0 } },
            { { -1, +1, -1 },{ 0, 0, -1 },{ 1, 0 } },
            { { +1, +1, -1 },{ 0, 0, -1 },{ 1, 1 } },
            { { +1, -1, -1 },{ 0, 0, -1 },{ 0, 1 } },

            { { -1, +1, +1 },{ 0, 0, +1 },{ 0, 0 } },
            { { -1, -1, +1 },{ 0, 0, +1 },{ 1, 0 } },
            { { +1, -1, +1 },{ 0, 0, +1 },{ 1, 1 } },
            { { +1, +1, +1 },{ 0, 0, +1 },{ 0, 1 } }
        }};


        Cube res;
        res.set_triangles(quads);
        res.set_vertices(verts);

        return res;
    }

    constexpr
    void set_triangles(const std::array<uint4, 6>& quads)
    {
        for (const auto& q: quads) {

            triangles.push_back({ q.x, q.y, q.z });
            triangles.push_back({ q.x, q.z, q.w });
        }
    }

    constexpr
    void set_vertices(const std::array<Vertex, 3*numVertices>& vv) noexcept
    {
        for (const auto& v: vv) {

            vertices.push_back(v.position);
            normals.push_back(v.normal);
            texcoords.push_back(v.texCoord);
        }
    }

    void print(const std::string& str) const
    {
        std::cout << str << "\n";
        std::cout << "vertices: \n";
        for (int i {}; const auto& v: vertices)
            std::cout << i++ << ": " << v.x << " " << v.y << " " << v.z
                      << std::endl;

        std::cout << "normals: \n";
        for (int i {}; const auto& n: normals)
            std::cout << i++ << ": " << n.x << " " << n.y << " " << n.z
                      << std::endl;
    }
};

#endif  // TINYPLY_EXAMPLES_UTILS_H
