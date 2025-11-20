/*
 * tinyply 2.3.4 (https://github.com/ddiakopoulos/tinyply)
 *
 * A single-header, zero-dependency (except the C++ STL) public domain implementation
 * of the PLY mesh file format. Requires C++11; errors are handled through exceptions.
 *
 * This software is in the public domain. Where that dedication is not
 * recognized, you are granted a perpetual, irrevocable license to copy,
 * distribute, and modify this file as you see fit.
 *
 * Authored by Dimitri Diakopoulos (http://www.dimitridiakopoulos.com)
 *
 */

#ifndef TINYPLY_IMPL_PROPERTY_H
#define TINYPLY_IMPL_PROPERTY_H

#include "types.h"

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

namespace tinyply::impl {

    struct Property {


        std::string name;

        Type scalarType {Type::INVALID};
        Type listType {Type::INVALID};
        size_t listCount {};

        explicit Property(std::istream& is);

        explicit constexpr Property(
            const Type type,
            const std::string& name
        )
            : name {name}
            , scalarType {type}
        {}

        explicit constexpr Property(
            const Type listType,
            const Type scalarType,
            const std::string& name,
            const size_t listCount
        )
            : name {name}
            , scalarType {scalarType}
            , listType {listType}
            , listCount {listCount}
        {}

        constexpr Type type_from_string(
            const std::string_view t
        ) const noexcept;

        constexpr bool is_list() const noexcept
        {
            return listType != Type::INVALID;
        }

        void report(std::string_view pref) const noexcept;
    };

}  // namespace tinyply::impl


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_LINK_AS_LIBRARY

namespace tinyply::impl {

Property::
Property(std::istream& is)
{
    std::string type;
    is >> type;
    if (type == "list") {

        std::string countType;
        is >> countType >> type;
        listType = type_from_string(countType);
        assert(is_list());
    }
    scalarType = type_from_string(type);
    is >> name;
}

constexpr Type Property::
type_from_string(const std::string_view t) const noexcept
{
    if      (t == "int8" || t == "char")      return Type::INT8;
    else if (t == "uint8" || t == "uchar")    return Type::UINT8;
    else if (t == "int16" || t == "short")    return Type::INT16;
    else if (t == "uint16" || t == "ushort")  return Type::UINT16;
    else if (t == "int32" || t == "int")      return Type::INT32;
    else if (t == "uint32" || t == "uint")    return Type::UINT32;
    else if (t == "float32" || t == "float")  return Type::FLOAT32;
    else if (t == "float64" || t == "double") return Type::FLOAT64;

    return Type::INVALID;
}

void Property::
report(std::string_view pref) const noexcept
{
    std::cout << pref << "\tproperty: " << name
              << " (type=" << types.at(scalarType).str << ")";
    if (is_list())
        std::cout << " (list_type=" << types.at(listType).str << ")";
    std::cout << std::endl;
}


}  // namespace tinyply::impl

#endif  // TINYPLY_LINK_AS_LIBRARY
#endif  // TINYPLY_IMPL_PROPERTY_H
