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

#ifndef TINYPLY_IMPL_DATA_BUFFER_H
#define TINYPLY_IMPL_DATA_BUFFER_H

#include "misc.h"
#include "types.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <memory>

namespace tinyply::impl {


    class Buffer {

        struct delete_array {
            void operator()(uint8_t* p) { delete[] p; }
        };

        std::unique_ptr<uint8_t, decltype(Buffer::delete_array())> data;

        size_t size {};
        uint8_t* alias {};

    public:

        Buffer() {};
        explicit Buffer(const size_t size)
            : data (new uint8_t[size], delete_array())
            , size {size}
            , alias {data.get()}  // allocating
        {}

        explicit constexpr Buffer(const uint8_t* ptr)
            : alias {const_cast<uint8_t*>(ptr)}  // non-allocating, todo: set size?
        {}

        constexpr uint8_t* get() noexcept
        {
            return alias;
        }

        constexpr const uint8_t* get() const noexcept
        {
            return alias;
        }

        constexpr size_t size_bytes() const noexcept
        {
            return size;
        }
    };


    struct Data {

        Type t;
        Buffer buffer;
        size_t count {};  // how many items are in the element?
        bool isList {};

        explicit Data(
            const Type t,
            const size_t count,
            const bool isList
        )
            : t {t}
            , count {count}
            , isList {isList}
        {}

        explicit Data(
            const Type t,
            Buffer&& buffer,
            const size_t count,
            const bool isList
        )
            : t {t}
            , buffer {std::move(buffer)}
            , count {count}
            , isList {isList}
        {}

        size_t num_items() const noexcept;

        void endian_reverse() noexcept;
    };


    struct DataCursor {

        size_t byteOffset {};
        size_t totalSizeBytes {};
    };

}  // namespace tinyply::impl

#ifdef TINYPLY_AS_LIBRARY

namespace tinyply::impl {

    size_t Data::
    num_items() const noexcept
    {
        return (buffer.size_bytes() / types.at(t).stride);
    }

    void Data::
    endian_reverse() noexcept
    {
        uint8_t* b = buffer.get();
        const size_t stride = types.at(t).stride;
        const size_t size = buffer.size_bytes();

        switch (t) {

            case Type::INT16:   endian_swap_buffer<int16_t, int16_t>(b, size, stride);   break;
            case Type::UINT16:  endian_swap_buffer<uint16_t, uint16_t>(b, size, stride); break;
            case Type::INT32:   endian_swap_buffer<int32_t, int32_t>(b, size, stride);   break;
            case Type::UINT32:  endian_swap_buffer<uint32_t, uint32_t>(b, size, stride); break;
            case Type::FLOAT32: endian_swap_buffer<uint32_t, float>(b, size, stride);    break;
            case Type::FLOAT64: endian_swap_buffer<uint64_t, double>(b, size, stride);   break;
            default: break;
        }
    }

}  // namespace tinyply::impl

#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_IMPL_DATA_BUFFER_H
