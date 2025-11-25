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

#ifndef TINYPLY_READER_H
#define TINYPLY_READER_H

#include "impl/data_buffer.h"
#include "impl/element.h"
#include "impl/file_in.h"
#include "impl/types.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <istream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tinyply {

    class Reader {

        std::unique_ptr<impl::FileIn> file;

    public:

        using Data = impl::Data;

        Reader();

        /**
         * The ply format requires an ascii header. This can be used to
         * determine at runtime which properties or elements exist in the file.
         * Limited validation of the header is performed;
         * it is assumed the header correctly reflects the contents of the
         * payload. This function may throw.
         * \returns true on success, false on failure.
         */
        bool parse_header(std::istream& is);

        /**
         * Execute a read operation.
         * Data must be requested via `request_properties_from_element(...)`
         * prior to calling this function.
         */
        void read(std::istream& is);

        /*
         * These functions are valid after a call to `parse_header(...)`.
         * Reader the case of writing, comments() reference may also be used to
         * add new comments to the ply header.
         */
        std::vector<impl::Element> get_elements() const;
        std::vector<std::string> get_info() const;
        std::vector<std::string>& comments();
        bool is_binary() const;

        /*
         * Reader the general case where |list_size_hint| is zero, `read` performs
         * a two-passparse to support variable length lists.
         * The most general use of the ply format is storing triangle meshes.
         * When this fact is known a-priori, we can pass an expected list length
         * that will apply to this element. Doing so results in an up-front
         * memory allocation and a single-pass import, a 2x performance optimization.
         */
        std::shared_ptr<impl::Data> request_properties_from_element(
            const std::string& elementKey,
            const std::vector<std::string> propertyKeys,
            uint32_t list_size_hint = 0
        );

        void report_structure() const noexcept;
    };

    using FileIn = Reader;

}  // namespace tinyply


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_AS_LIBRARY

namespace tinyply {

Reader::
Reader()
    : file {std::make_unique<impl::FileIn>()}
{}

bool Reader::
parse_header(std::istream& is)
{
    return file->header.parse(is);
}

void Reader::
read(std::istream& is)
{
    return file->read(is);
}

std::vector<impl::Element> Reader::
get_elements() const
{
    return file->header.elements;
}

std::vector<std::string>& Reader::
comments()
{
    return file->header.comments;
}

std::vector<std::string> Reader::
get_info() const
{
    return file->header.objInfo;
}

bool Reader::
is_binary() const
{
    return file->header.isBinary;
}

std::shared_ptr<impl::Data> Reader::
request_properties_from_element(const std::string& elementKey,
                                const std::vector<std::string> propertyKeys,
                                const uint32_t list_size_hint)
{
    return file->request_properties_from_element(elementKey,
                                                 propertyKeys,
                                                 list_size_hint);
}

void Reader::
report_structure() const noexcept
{
    file->header.report();
}

}  // namespace tinyply

#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_READER_H
