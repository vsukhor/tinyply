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

#ifndef TINYPLY_IMPL_TYPES_H
#define TINYPLY_IMPL_TYPES_H

#include "misc.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <istream>
#include <map>
#include <ostream>
#include <string_view>


namespace tinyply {
namespace impl {

    enum class Type: uint8_t {

        INVALID,
        INT8,
        UINT8,
        INT16,
        UINT16,
        INT32,
        UINT32,
        FLOAT32,
        FLOAT64
    };


    struct Info {

        const Type t;

        const int stride;
        std::string_view str;

        int write_ascii(std::ostream& os,
                        const uint8_t* src) const
        {
            switch (t) {

                case Type::INT8:    os << static_cast<int32_t>(*reinterpret_cast<const int8_t*>(src));   break;
                case Type::UINT8:   os << static_cast<uint32_t>(*reinterpret_cast<const uint8_t*>(src)); break;
                case Type::INT16:   os << *reinterpret_cast<const int16_t*>(src);  break;
                case Type::UINT16:  os << *reinterpret_cast<const uint16_t*>(src); break;
                case Type::INT32:   os << *reinterpret_cast<const int32_t*>(src);  break;
                case Type::UINT32:  os << *reinterpret_cast<const uint32_t*>(src); break;
                case Type::FLOAT32: os << *reinterpret_cast<const float*>(src);    break;
                case Type::FLOAT64: os << *reinterpret_cast<const double*>(src);   break;
                case Type::INVALID: throw std::invalid_argument("invalid ply type");
            }
            return stride;
        }

        int read_ascii(std::istream& is,
                       void* dest) const
        {
            switch (t) {

                case Type::INT8:    *((int8_t*)dest)  = static_cast<int8_t>(impl::read_ascii<int32_t>(is));   break;
                case Type::UINT8:   *((uint8_t*)dest) = static_cast<uint8_t>(impl::read_ascii<uint32_t>(is)); break;
                case Type::INT16:   cast_ascii<int16_t>(dest, is);   break;
                case Type::UINT16:  cast_ascii<uint16_t>(dest, is);  break;
                case Type::INT32:   cast_ascii<int32_t>(dest, is);   break;
                case Type::UINT32:  cast_ascii<uint32_t>(dest, is);  break;
                case Type::FLOAT32: cast_ascii<float>(dest, is);     break;
                case Type::FLOAT64: cast_ascii<double>(dest, is);    break;
                case Type::INVALID: throw std::invalid_argument("invalid ply type");
            }

            return stride;
        }
    };

    static inline const std::map<const Type, const Info> types
    {
        { Type::INT8,    Info(Type::INT8, 1, "char") },
        { Type::UINT8,   Info(Type::UINT8, 1, "uchar") },
        { Type::INT16,   Info(Type::INT16, 2, "short") },
        { Type::UINT16,  Info(Type::UINT16, 2, "ushort") },
        { Type::INT32,   Info(Type::INT32, 4, "int") },
        { Type::UINT32,  Info(Type::UINT32, 4, "uint") },
        { Type::FLOAT32, Info(Type::FLOAT32, 4, "float") },
        { Type::FLOAT64, Info(Type::FLOAT64, 8, "double") },
        { Type::INVALID, Info(Type::INVALID, 0, "INVALID") }
    };

    void swap_endanness(Type t, void* dst)
    {
        switch (t) {

            case Type::INT16:  *(int16_t*)dst  = endian_swap<int16_t, int16_t>(*(int16_t*)dst);    break;
            case Type::UINT16: *(uint16_t*)dst = endian_swap<uint16_t, uint16_t>(*(uint16_t*)dst); break;
            case Type::INT32:  *(int32_t*)dst  = endian_swap<int32_t, int32_t>(*(int32_t*)dst);    break;
            case Type::UINT32: *(uint32_t*)dst = endian_swap<uint32_t, uint32_t>(*(uint32_t*)dst); break;
            default: break;
        }

    }

}  // namespace impl

using Type = impl::Type;

}  // namespace tinyply

#ifdef TINYPLY_LINK_AS_LIBRARY

namespace tinyply::impl {

}  // namespace tinyply::impl

#endif  // TINYPLY_LINK_AS_LIBRARY
#endif  // TINYPLY_IMPL_TYPES_H
