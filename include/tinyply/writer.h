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

#ifndef TINYPLY_WRITER_H
#define TINYPLY_WRITER_H

#include "impl/data_buffer.h"
#include "impl/element.h"
#include "impl/file_out.h"
#include "impl/types.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tinyply {

    struct Writer {

        template<Type T> using type = impl::type<T>;

        std::unique_ptr<impl::FileOut> file;

        Writer();

        /**
         * Performs no validation and assumes that the data passed into
         * `add_properties_to_element` is well-formed.
         */
        void write(std::ofstream& os,
                   bool asBinary);
        void write(const std::filesystem::path& p,
                   bool asBinary);

        bool is_binary() const noexcept;

        void add_comment(const std::string& str) noexcept;

        impl::Element& add_element(const std::string& elementKey) noexcept;
        /*
         * In the general case where |list_size_hint| is zero, `read` performs
         * a two-passparse to support variable length lists.
         * The most general use of the ply format is storing triangle meshes.
         * When this fact is known a-priori, we can pass an expected list length
         * that will apply to this element. Doing so results in an up-front
         * memory allocation and a single-pass import, a 2x performance optimization.
         */
        void add_properties_to_element(
            const std::string& elementKey,
            const std::vector<std::string>& propertyKeys,
            impl::Type type,
            size_t count,
            uint8_t const* data,
            impl::Type listType,
            size_t listCount
        );
    };

    using FileOut = Writer;

}  // namespace tinyply


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_AS_LIBRARY

namespace tinyply {

Writer::
Writer()
    : file {std::make_unique<impl::FileOut>()}
{}

void Writer::
write(std::ofstream& os,
      const bool asBinary)
{
    return file->write(os, asBinary);
}

void Writer::
write(const std::filesystem::path& p,
      const bool asBinary)
{
    return file->write(p, asBinary);
}

void Writer::
add_comment(const std::string& str) noexcept
{
    file->header.comments.push_back(str);
}

bool Writer::
is_binary() const noexcept
{
    return file->header.isBinary;
}

impl::Element& Writer::
add_element(const std::string& elementKey) noexcept
{
    return file->add_element(elementKey);
}

void Writer::
add_properties_to_element(const std::string& elementKey,
                          const std::vector<std::string>& propertyKeys,
                          const impl::Type type,
                          const size_t count,
                          uint8_t const* data,
                          const impl::Type listType,
                          const size_t listCount)
{
    return file->add_properties_to_element(elementKey,
                                           propertyKeys,
                                           type,
                                           count,
                                           data,
                                           listType,
                                           listCount);
}

}  // namespace tinyply

#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_WRITER_H
