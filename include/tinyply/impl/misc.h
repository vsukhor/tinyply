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

#ifndef TINYPLY_IMPL_MISC_H
#define TINYPLY_IMPL_MISC_H

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <istream>
#include <string>

namespace tinyply::impl {

// Endian swaps ================================================================

template<typename T,
         typename T2> constexpr
T2 endian_swap(const T& /*v*/) noexcept
{
    static_assert(false, "endian_swap is not implemented this type");

    return T2 {};
}

template<> constexpr
uint16_t endian_swap<uint16_t, uint16_t>(const uint16_t& v) noexcept
{
    return (v << 8) | (v >> 8);
}

template<> constexpr
uint32_t endian_swap<uint32_t, uint32_t>(const uint32_t& v) noexcept
{
    return (v << 24) |
           ((v << 8) & 0x00ff0000) |
           ((v >> 8) & 0x0000ff00) |
           (v >> 24);
}

template<> constexpr
uint64_t endian_swap<uint64_t, uint64_t>(const uint64_t& v) noexcept
{
    return (((v & 0x00000000000000ffLL) << 56) |
            ((v & 0x000000000000ff00LL) << 40) |
            ((v & 0x0000000000ff0000LL) << 24) |
            ((v & 0x00000000ff000000LL) << 8)  |
            ((v & 0x000000ff00000000LL) >> 8)  |
            ((v & 0x0000ff0000000000LL) >> 24) |
            ((v & 0x00ff000000000000LL) >> 40) |
            ((v & 0xff00000000000000LL) >> 56));
}

template<> inline
int16_t endian_swap<int16_t, int16_t>(const int16_t& v) noexcept
{
    uint16_t r = endian_swap<uint16_t, uint16_t>(*(uint16_t*)&v);
    return *(int16_t*)&r;
}

template<> inline
int32_t endian_swap<int32_t, int32_t>(const int32_t& v) noexcept
{
    uint32_t r = endian_swap<uint32_t, uint32_t>(*(uint32_t*)&v);
    return *(int32_t*)&r;
}

template<> inline
int64_t endian_swap<int64_t, int64_t>(const int64_t& v) noexcept
{
    uint64_t r = endian_swap<uint64_t, uint64_t>(*(uint64_t*)&v);
    return *(int64_t*)&r;
}

template<> constexpr
float endian_swap<uint32_t, float>(const uint32_t& v) noexcept
{
    union { float f; uint32_t i; };
    i = endian_swap<uint32_t, uint32_t>(v);
    return f;
}

template<> constexpr
double endian_swap<uint64_t, double>(const uint64_t& v) noexcept
{
    union { double d;
            uint64_t i; };
    i = endian_swap<uint64_t, uint64_t>(v);
    return d;
}

template<typename T,
         typename T2>
constexpr
void endian_swap_buffer(
    uint8_t* data_ptr,
    const size_t num_bytes,
    const size_t stride
) noexcept
{
    for (size_t count {}; count < num_bytes; count += stride) {

        *(reinterpret_cast<T2*>(data_ptr)) =
            endian_swap<T, T2>(*(reinterpret_cast<const T*>(data_ptr)));

        data_ptr += stride;
    }
}

// Hash ========================================================================

uint32_t hash_fnv1a(const std::string& str) noexcept
{
    static constexpr uint32_t fnv1aBase32 {0x811C9DC5u};
    static constexpr uint32_t fnv1aPrime32 {0x01000193u};

    auto result = fnv1aBase32;
    for (const auto& c: str) {
        result ^= static_cast<uint32_t>(c);
        result *= fnv1aPrime32;
    }
    return result;
}

// ascii =======================================================================

template<typename T>
T read_ascii(std::istream& is)
{
    T data;
    is >> data;
    return data;
}

template<typename T>
void cast_ascii(void* dest,
                std::istream& is)
{
    *(static_cast<T*>(dest)) = read_ascii<T>(is);
}


}  // namespace tinyply::impl

# endif  // TINYPLY_IMPL_MISC_H
