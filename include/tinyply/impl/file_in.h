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
 */

#ifndef TINYPLY_IMPL_FILE_IN_H
#define TINYPLY_IMPL_FILE_IN_H

#include "impl/data_buffer.h"
#include "impl/header.h"

#include <algorithm>
#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <cstring>  // memcpy
#include <filesystem>
#include <fstream>
#include <functional>  // function
#include <iostream>
#include <memory>
#include <set>

namespace tinyply::impl {

struct FileIn {

    using PropertyLookup = Header::PropertyLookup;

    uint8_t scratch[64]; // large enough for max list size
    Header header;

    void read(std::istream& is);
    Element* request_element(const std::string_view& elementKey);

    std::shared_ptr<Data> request_properties_from_element(
        const std::string& elementKey,
        const std::vector<std::string> propertyKeys,
        const uint32_t list_size_hint=0
    );

    std::shared_ptr<Data> request_properties_from_element(
        const Element& element,
        const std::vector<std::string> propertyKeys,
        const uint32_t list_size_hint
    );

    size_t read_property_binary(
        const size_t stride,
        void* dest,
        size_t& destOffset,
        size_t destSize,
        std::istream& is
    );

    size_t read_property_ascii(
        Type t,
        const size_t stride,
        void* dest,
        size_t& destOffset,
        size_t destSize,
        std::istream& is
    );

    void parse_data(std::istream& is,
                    bool firstPass);
};

}  // namespace tinyply::impl


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_LINK_AS_LIBRARY

namespace tinyply::impl {

Element* FileIn::
request_element(const std::string_view& elementKey)
{
    if (header.elements.empty())
        throw std::runtime_error("header had no elements defined. malformed file?");
    if (elementKey.empty())
        throw std::invalid_argument("`elementKey` argument is empty");

    return header.find_element(elementKey);
}

std::shared_ptr<Data> FileIn::
request_properties_from_element(const std::string& elementKey,
                                const std::vector<std::string> propertyKeys,
                                const uint32_t list_size_hint)
{
    const auto element = request_element(elementKey);
    if (!element)
        throw std::invalid_argument(
            "requested element " + std::string(elementKey) + " not found"
        );

    return request_properties_from_element(*element,
                                           propertyKeys,
                                           list_size_hint);
}

std::shared_ptr<Data> FileIn::
request_properties_from_element(const Element& element,
                                std::vector<std::string> propertyKeys,
                                const uint32_t list_size_hint)
{
    if (propertyKeys.empty())
        throw std::invalid_argument("`propertyKeys` argument is empty");

    // Check that all requested properties are registered to the element.
    for (auto& key: propertyKeys)
        if (!element.contains(key)) {
            std::cerr << "requested property '" + key +
                "' not found in the element '" + element.name +
                "' ... removing it from the list" << std::endl;
            key.clear();
        }

    // Sanity check that all properties share the same type
    std::set<Type> scalarTypes;
    for (const auto& key: propertyKeys)
        if (key.length())
            scalarTypes.insert(element.get_property(key)->scalarType);
    if (scalarTypes.size() == 0 || *scalarTypes.begin() == Type::INVALID)
        throw std::invalid_argument(
            "requested properties contain no valid items"
        );
    else if (scalarTypes.size() > 1)
        throw std::invalid_argument(
            "all requested properties must share the scalar type"
        );

    // Each key in `propertyKeys` gets an entry into the userData map (keyed
    // by a hash of element name and property name), but groups of
    // properties (requested from the public api through this function) all
    // share the same `ParsingHelper`. When it comes time to .read(), we
    // check the number of unique Data shared pointers and allocate a single
    // buffer that will be used by each property key group.
    // That way, properties like, {"x", "y", "z"} will all be put into the
    // same buffer.

    auto data = std::make_shared<Data>(Type::INVALID,
                                       element.size,  // number of 'element.name' records
                                       false);        // not a list

    ParsingHelper helper {data,
                          std::make_shared<DataCursor>(),
                          list_size_hint};

    for (const auto& key: propertyKeys)
        if (key.length()) {
            const Property& property = *element.get_property(key);
            helper.data->t = property.scalarType;
            helper.data->isList = property.is_list();
            header.userData.insert(element, property, std::move(helper));
        }

    return data;
}


void FileIn::
parse_data(std::istream& is,
           const bool firstPass)
{
    std::function<void(PropertyLookup& f,
                       const Property& p,
                       uint8_t* dest,
                       size_t& destOffset,
                       size_t destSize,
                       std::istream& is)> read;

    std::function<size_t(PropertyLookup& f,
                         const Property& p,
                         std::istream& is)> skip;

    const auto start = is.tellg();

    uint32_t listSize {};
    size_t dummyCount {};
    std::string skip_ascii_buffer;

    // Special case mirroring read_property_binary but for list types; this
    // has an additional big endian check to flip the data in place immediately
    // after reading. We do this as a performance optimization; endian flipping
    // is done on regular properties as a post-process after reading (also for
    // optimization) but we need the correct little-endian list count as we
    // read the file.
    auto read_list_binary = [this](const Type& t,
                                   void* dst,
                                   size_t& destOffset,
                                   const size_t stride,
                                   std::istream& _is) noexcept
    {
        destOffset += stride;
        _is.read((char*)dst, stride);

        if (header.isBigEndian)
            swap_endanness(t, dst);

        return stride;
    };

    if (header.isBinary) {

        read = [this, &listSize, &dummyCount, &read_list_binary](PropertyLookup& f,
                                                                 const Property& p,
                                                                 uint8_t* dest,
                                                                 size_t& destOffset,
                                                                 const size_t destSize,
                                                                 std::istream& _is)
        {
            if (!p.is_list())

                return read_property_binary(f.prop_stride,
                                            dest + destOffset,
                                            destOffset,
                                            destSize,
                                            _is);

            read_list_binary(p.listType,
                             &listSize,
                             dummyCount,
                             f.list_stride,
                             _is); // the list size

            return read_property_binary(f.prop_stride * listSize,
                                        dest + destOffset,
                                        destOffset,
                                        destSize,
                                        _is); // properties in list
        };

        skip = [this, &listSize, &dummyCount, &read_list_binary](PropertyLookup& f,
                                                                 const Property& p,
                                                                 std::istream& _is) noexcept
        {
            if (!p.is_list()) {

                _is.read((char*)scratch, f.prop_stride);
                return f.prop_stride;
            }

            read_list_binary(p.listType,
                             &listSize,
                             dummyCount,
                             f.list_stride,
                             _is); // the list size (does not count for memory alloc)
            auto bytes_to_skip = f.prop_stride * listSize;
            _is.ignore(bytes_to_skip);

            return bytes_to_skip;
        };
    }
    else {  // ascii

        read = [this, &listSize, &dummyCount](PropertyLookup& f,
                                              const Property& p,
                                              uint8_t* dest,
                                              size_t& destOffset,
                                              const size_t destSize,
                                              std::istream& _is)
        {
            if (!p.is_list())

                read_property_ascii(p.scalarType,
                                    f.prop_stride,
                                    dest + destOffset,
                                    destOffset,
                                    destSize,
                                    _is);

            else {
                dummyCount = 0;
                read_property_ascii(p.listType,
                                    f.list_stride,
                                    &listSize,
                                    dummyCount,
                                    sizeof(listSize),
                                    _is); // the list size

                for (size_t i {}; i < listSize; ++i)

                    read_property_ascii(p.scalarType,
                                        f.prop_stride,
                                        dest + destOffset,
                                        destOffset,
                                        destSize,
                                        _is);
            }
        };

        skip = [this, &listSize, &dummyCount, &skip_ascii_buffer](PropertyLookup& f,
                                                                  const Property& p,
                                                                  std::istream& _is) noexcept
        {
            skip_ascii_buffer.clear();
            if (p.is_list()) {

                dummyCount = 0;
                read_property_ascii(p.listType,
                                    f.list_stride,
                                    &listSize,
                                    dummyCount,
                                    sizeof(listSize),
                                    _is);  // the list size (does not count for memory alloc)

                for (size_t i {}; i < listSize; ++i)
                    _is >> skip_ascii_buffer; // properties in list

                return listSize * f.prop_stride;
            }
            _is >> skip_ascii_buffer;
            return f.prop_stride;
        };
    }

    std::vector<std::vector<PropertyLookup>> element_property_lookup_table =
        header.make_property_lookup_table();

    // This is the inner import loop
    for (size_t element_idx {};
         auto& element: header.elements) {
        for (size_t count {}; count < element.size; ++count) {

            for (size_t property_idx {};
                 auto& property: element.properties) {

                PropertyLookup& lookup =
                    element_property_lookup_table[element_idx][property_idx];

                if (lookup.skip)
                    skip(lookup, property, is);

                else {

                    ParsingHelper const* helper = lookup.helper;
                    if (firstPass) {

                        helper->cursor->totalSizeBytes += skip(lookup, property, is);

                        // These lines will be changed when tinyply supports
                        // variable length lists. We add it here so our header data structure
                        // contains enough info to write it back out again (e.g. transcoding).
                        if (property.listCount == 0)
                            property.listCount = listSize;
                        if (property.listCount != listSize)
                            throw std::runtime_error("variable length lists are not supported yet.");
                    }
                    else
                        read(lookup,
                             property,
                             helper->data->buffer.get(),
                             helper->cursor->byteOffset,
                             helper->data->buffer.size_bytes(),  // destSize
                             is);
                }

                property_idx++;
            }
        }
        element_idx++;
    }

    // Reset istream position to the start of the data
    if (firstPass)
        is.seekg(start, is.beg);
}

size_t FileIn::
read_property_binary(const size_t stride,
                     void* dest,
                     size_t& destOffset,
                     const size_t destSize,
                     std::istream& is)
{
    if (destOffset + stride > destSize)
        throw std::runtime_error("unexpected EOF. malformed file?");

    destOffset += stride;
    is.read(reinterpret_cast<char*>(dest), stride);

    return stride;
}


size_t FileIn::
read_property_ascii(const Type t,
                    const size_t stride,
                    void* dest,
                    size_t& destOffset,
                    const size_t destSize,
                    std::istream& is)
{
    if (destOffset + stride > destSize)
        throw std::runtime_error("unexpected EOF. malformed file?");

    types.at(t).read_ascii(is, dest);
    destOffset += stride;

    return stride;
}

void FileIn::
read(std::istream& is)
{
    std::vector<std::shared_ptr<Data>> datas;
    for (auto& [_, helper]: header.userData.get())
        datas.push_back(helper.data);

    // Discover if we can allocate up front without parsing the file twice
    uint32_t list_hints {};
    for (auto& d: datas)
        for (const auto& [_, helper]: header.userData.get()) {
            list_hints += helper.list_size_hint;
            (void)d;
        }

    // No list hints? Then we need to calculate how much memory to allocate
    if (list_hints == 0)
        parse_data(is, true);

    // Count the number of properties (required for allocation)
    // e.g. if we have properties x y and z requested, we ensure
    // that their buffer points to the same Data
    std::unordered_map<Data*, int32_t> unique_data_count;
    for (auto& ptr: datas)
        unique_data_count[ptr.get()] += 1;

    // Since group-requested properties share the same cursor,
    // we need to find unique cursors so we only allocate once
    std::sort(datas.begin(), datas.end());
    datas.erase(std::unique(datas.begin(), datas.end()), datas.end());

    // We sorted by ptrs on Data, need to remap back onto its cursor in
    // the userData table
    for (auto& d: datas) {
        for (auto& [_, helper]: header.userData.get()) {
            if (helper.data == d && d->buffer.get() == nullptr) {

                // If we didn't receive any list hints, it means we did two
                // passes over the file to compute the total length of all
                // (potentially) variable-length lists
                if (list_hints == 0)
                    d->buffer = Buffer{helper.cursor->totalSizeBytes};
                else {
                    // otherwise, we can allocate up front, skipping the first pass.
                    const size_t list_size_multiplier =
                        helper.data->isList ? helper.list_size_hint : 1;

                    auto bytes_per_property = helper.data->count *
                                              types.at(helper.data->t).stride *
                                              list_size_multiplier;
                    bytes_per_property *= unique_data_count[d.get()];

                    d->buffer = Buffer{bytes_per_property};
                }

            }
        }
    }

    // Populate the data
    parse_data(is, false);

    // In-place big-endian to little-endian swapping if required
    if (header.isBigEndian)
        for (auto& d: datas)
            d->swap_endanness();
}


}  // namespace tinyply::impl

#endif  // TINYPLY_LINK_AS_LIBRARY
#endif  // TINYPLY_IMPL_FILE_IN_H
