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

#ifndef TINYPLY_IMPL_ELEMENT_H
#define TINYPLY_IMPL_ELEMENT_H

#include "impl/property.h"

#include <cassert>
#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <string>
#include <string_view>
#include <vector>

namespace tinyply::impl {

    struct Element {

        std::string name;

        size_t size {};  ///< number of records for this element

        std::vector<Property> properties;

        explicit Element(std::istream& istream);
        explicit constexpr Element(const std::string& name);
        explicit constexpr Element(
            const std::string& name,
            const size_t count
        );

        Property const* const get_property(std::string_view propName) const;
        constexpr int64_t find_property(std::string_view propName) const noexcept;
        constexpr bool contains(std::string_view propName) const noexcept;

        void create_properties(
            const std::vector<std::string>& propNames,
            const Type type,
            const Type listType,
            const size_t listCount
        );

        void report(std::string_view pref) const noexcept;
    };

}  // namespace tinyply::impl


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_AS_LIBRARY

namespace tinyply::impl {

Element::
Element(std::istream& is)
{
    is >> name >> size;
}

constexpr
Element::
Element(const std::string& name)
    : name {name}
    , size {}
{}

constexpr
Element::
Element(
    const std::string& name,
    const size_t count
)
    : name {name}
    , size {count}
{}

constexpr
int64_t Element::
find_property(std::string_view propName) const noexcept
{
    for (size_t i {}; i < properties.size(); ++i)
        if (properties[i].name == propName)
            return i;

    return -1;
}

constexpr
bool Element::
contains(std::string_view propName) const noexcept
{
    return find_property(propName) >= 0;
}

Property const* const Element::
get_property(std::string_view propName) const
{
    for (const auto& p: properties)
        if (p.name == propName)
            return &p;

    throw std::invalid_argument("property '" + std::string(propName) +
                                "' was not found in element '" + name +"'");
//    assert(false);
    return nullptr;
}

void Element::
create_properties(const std::vector<std::string>& propNames,
                  const Type type,
                  const Type listType,
                  const size_t listCount)
{
    for (const auto& key: propNames) {

        if (contains(key)) {
            throw std::invalid_argument(
                "all requested properties must share the same type."
            );
            assert(false);
        }

        auto newProp = listType == Type::INVALID
            ? Property{type, key}
            : Property{listType, type, key, listCount};

        properties.push_back(newProp);
    }
}

void Element::
report(std::string_view pref) const noexcept
{
    std::cout << pref << "element: "
              << name << " (" << size << ")"
              << std::endl;

    for (const auto& p: properties)
        p.report(pref);
}


}  // namespace tinyply::impl

#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_IMPL_ELEMENT_H
