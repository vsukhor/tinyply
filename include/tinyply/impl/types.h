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
 * Modified by Valerii Sukhorukov (vsukhorukov@yahoo.com, https://github.com/vsukhor)
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef TINYPLY_IMPL_TYPES_H
#define TINYPLY_IMPL_TYPES_H

#include "misc.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <istream>
#include <map>
#include <ostream>
#include <string_view>
#include <tuple>


namespace tinyply {
namespace impl {


    using typetup = std::tuple<void,
                               int8_t,
                               uint8_t,
                               int16_t,
                               uint16_t,
                               int32_t,
                               uint32_t,
                               float,
                               double>;

    // Gest index of tuple's element type.
    // See answer by Casey (https://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type?rq=3)
    template <class T, class Tuple>
    struct Index;
    template <class T, class... Types>
    struct Index<T, std::tuple<T, Types...>> {
        static const std::size_t value = 0;
    };
    template <class T, class U, class... Types>
    struct Index<T, std::tuple<U, Types...>> {
        static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
    };

    enum class Type: uint8_t {

        INVALID = Index<void, typetup>::value,
        INT8    = Index<int8_t, typetup>::value,
        UINT8   = Index<uint8_t, typetup>::value,
        INT16   = Index<int16_t, typetup>::value,
        UINT16  = Index<uint16_t, typetup>::value,
        INT32   = Index<int32_t, typetup>::value,
        UINT32  = Index<uint32_t, typetup>::value,
        FLOAT32 = Index<float, typetup>::value,
        FLOAT64 = Index<double, typetup>::value,
    };

    template<Type T>
    using type = std::tuple_element_t<static_cast<size_t>(T), typetup>;

    struct Info {

        const Type t;
        const int stride;
        const std::string_view str;

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

    void endian_reverse(Type t, void* dst)
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
template<Type T> using type = impl::type<T>;

}  // namespace tinyply

#ifdef TINYPLY_AS_LIBRARY

namespace tinyply::impl {

}  // namespace tinyply::impl

#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_IMPL_TYPES_H
