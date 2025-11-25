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

#ifndef TINYPLY_IMPL_HEADER_H
#define TINYPLY_IMPL_HEADER_H

#include "element.h"
#include "user_data.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace tinyply::impl {

    struct Header {

        struct PropertyLookup {

            ParsingHelper const* helper {};
            bool skip {};
            size_t prop_stride {};  // precomputed
            size_t list_stride {};  // precomputed
        };

        UserData userData;

        bool isBinary {};
        bool isBigEndian {};

        std::vector<Element> elements;
        std::vector<std::string> comments;
        std::vector<std::string> objInfo;

        std::vector<std::vector<PropertyLookup>> make_property_lookup_table();
        constexpr Element* find_element(std::string_view key) noexcept;

        bool parse(std::istream& is);
        void read_format(std::istream& is);
        void read_element(std::istream& is);
        void read_property(std::istream& is);
        void read_text(std::string line,
                       std::vector<std::string>& place,
                       int erase = 0);

        void write(std::ostream& os) noexcept;

        void report() const noexcept;
    };

}  // namespace tinyply::impl


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_AS_LIBRARY

namespace tinyply::impl {

// The `userData` table is an easy data structure for capturing what data the
// user would like out of the ply file, but an inner-loop hash lookup is not ideal.
// The property lookup table flattens the table down into a 2D array optimized
// for parsing.
// The first index is the element, and the second index is the property.
std::vector<std::vector<Header::PropertyLookup>> Header::
make_property_lookup_table()
{
    std::vector<std::vector<PropertyLookup>> element_property_lookup;

    for (auto& element: elements) {
        std::vector<PropertyLookup> lookups;

        for (auto& property: element.properties) {
            PropertyLookup f;

            const auto parsingHelper = userData.find(element, property);
            if (parsingHelper)
                f.helper = parsingHelper;
            else
                f.skip = true;

            f.prop_stride = types.at(property.scalarType).stride;
            if (property.is_list())
                f.list_stride = types.at(property.listType).stride;

            lookups.push_back(f);
        }

        element_property_lookup.push_back(lookups);
    }

    return element_property_lookup;
}

constexpr
Element* Header::
find_element(std::string_view key) noexcept
{
    for (auto& e: elements)
        if (e.name == key)

            return &e;

    return nullptr;
}


bool Header::
parse(std::istream& is)
{
    std::string line;
    bool success = true;
    while (std::getline(is, line)) {

        std::istringstream ls(line);
        std::string token;
        ls >> token;

        if (token == "ply" ||
            token == "PLY" ||
            token == "") continue;

        else if (token == "comment")    read_text(line, comments, 8);
        else if (token == "format")     read_format(ls);
        else if (token == "element")    read_element(ls);
        else if (token == "property")   read_property(ls);
        else if (token == "obj_info")   read_text(line, objInfo, 9);
        else if (token == "end_header") break;
        else { // unexpected header field
            std::cerr << "Unexpected header field encountered: " << token
                      << std::endl;
            success = false;
        }
    }
    return success;
}

void Header::
read_text(std::string line,
          std::vector<std::string>& place,
          const int erase)
{
    place.push_back(erase > 0 ? line.erase(0, erase)
                              : line);
}

void Header::
read_format(std::istream& is)
{
    std::string s;
    is >> s;

    if (s == "binary_little_endian")
        isBinary = true;
    else if (s == "binary_big_endian")
        isBinary = isBigEndian = true;
}

void Header::
read_element(std::istream& is)
{
    elements.emplace_back(is);
}

void Header::
read_property(std::istream& is)
{
    if (!elements.size())
        throw std::runtime_error("no elements defined; file is malformed");

    elements.back().properties.emplace_back(is);
}

void Header::
write(std::ostream& os) noexcept
{
    const std::locale& fixLoc = std::locale("C");
    os.imbue(fixLoc);

    os << "ply\n";
    if (isBinary)
        os << (isBigEndian ? "format binary_big_endian 1.0"
                           : "format binary_little_endian 1.0") << "\n";
    else
        os << "format ascii 1.0\n";

    for (const auto& comment: comments)
        os << "comment " << comment << "\n";

    auto property_lookup = make_property_lookup_table();

    size_t element_idx {};
    for (auto& e: elements) {

        os << "element " << e.name << " " << e.size << "\n";
        size_t property_idx = 0;
        for (const auto& p: e.properties) {

            PropertyLookup& lookup = property_lookup[element_idx][property_idx];

            if (!lookup.skip) {
                if (p.is_list()) {

                    os << "property list "
                       << types.at(p.listType).str << " "
                       << types.at(p.scalarType).str << " "
                       << p.name << "\n";
                }
                else
                    os << "property "
                       << types.at(p.scalarType).str << " "
                       << p.name << "\n";
            }
            property_idx++;
        }
        element_idx++;
    }
    os << "end_header\n";
}

void Header::
report() const noexcept
{
    constexpr std::string_view pref {"\t[ply_header] "};

    std::cout << pref << "Type: "
              << (isBinary ? "binary" : "ascii") << std::endl;

    for (const auto& c: comments)
        std::cout << pref << "Comment: " << c << std::endl;

    for (const auto& c: objInfo)
        std::cout << pref << "Info: " << c << std::endl;

    for (const auto& e: elements)
        e.report(pref);
}

}  // namespace tinyply::impl

#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_IMPL_HEADER_H
