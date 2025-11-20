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

#ifndef TINYPLY_IMPL_USER_DATA_H
#define TINYPLY_IMPL_USER_DATA_H

#include "impl/element.h"
#include "impl/misc.h"
#include "impl/property.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace tinyply::impl {

    struct ParsingHelper {

        std::shared_ptr<Data> data;
        std::shared_ptr<DataCursor> cursor;
        uint32_t list_size_hint;
    };


    struct UserData {

        using Map = std::unordered_map<uint32_t, const ParsingHelper>;

        Map dataMap;

        void  insert(const Element& element,
                     const Property& property,
                     const ParsingHelper&& helper)
        {
            insert(element.name, property.name, std::move(helper));
        }

        void  insert(const std::string& elementName,
                     const std::string& propertyName,
                     const ParsingHelper&& helper)
        {
            auto hash = hash_fnv1a(elementName + propertyName);
            auto result = dataMap.insert(Map::value_type{hash, std::move(helper)});

            if (result.second == false)
                throw std::invalid_argument(
                    "element-property key has already been requested: " +
                    elementName + " " + propertyName
                );
        }

        ParsingHelper const* find(const Element& element,
                                  const Property& property) const noexcept
        {
            auto hash = hash_fnv1a(element.name + property.name);
            const auto it = dataMap.find(hash);
            return it == dataMap.end() ? nullptr
                                       : &(it->second);
        }

        Map& get() noexcept
        {
            return dataMap;
        }

        void print() const
        {

        }
    };

}  // namespace tinyply::impl

#ifdef TINYPLY_AS_LIBRARY

namespace tinyply::impl {

}  // namespace tinyply::impl


#endif  // TINYPLY_AS_LIBRARY
#endif  // TINYPLY_IMPL_USER_DATA_H
