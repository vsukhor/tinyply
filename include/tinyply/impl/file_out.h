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

#ifndef TINYPLY_IMPL_FILE_OUT_H
#define TINYPLY_IMPL_FILE_OUT_H

#include "impl/data_buffer.h"
#include "impl/header.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <cstring>  // memcpy
#include <filesystem>
#include <fstream>
#include <functional>  // function
#include <iostream>
#include <memory>
#include <set>

namespace tinyply::impl {

struct FileOut {

    using PropertyLookup = Header::PropertyLookup;

    Header header;

    void write(std::ostream& os,
               bool asBinary);
    void write(const std::filesystem::path& p,
               bool asBinary);

    constexpr Element& add_element(const std::string& elementName) noexcept;

    void add_properties_to_element(
        Element& element,
        const std::vector<std::string>& propertyKeys,
        const Type type,
        const size_t count,
        uint8_t const* data,
        const Type listType,
        const size_t listCount
    );

    void add_properties_to_element(
        const std::string& elementKey,
        const std::vector<std::string>& propertyKeys,
        const Type type,
        const size_t count,
        uint8_t const* data,
        const Type listType,
        const size_t listCount
    );

    void write_ascii(std::ostream& os) noexcept;
    void write_binary(std::ostream& os) noexcept;

    void write_scalar_ascii(Type t,
                            std::ostream& os,
                            uint8_t const* src,
                            size_t& srcOffset) const;
    void write_scalar_binary(std::ostream& os,
                             uint8_t const* src,
                             size_t& srcOffset,
                             const size_t stride) const noexcept;
};

}  // namespace tinyply::impl


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TINYPLY_LINK_AS_LIBRARY

namespace tinyply::impl {

constexpr Element& FileOut::
add_element(const std::string& elName) noexcept
{
    return header.elements.emplace_back(elName);
}

void FileOut::
add_properties_to_element(Element& element,
                          const std::vector<std::string>& propertyKeys,
                          const Type type,
                          const size_t count,
                          uint8_t const* data,
                          const Type listType,
                          const size_t listCount)
{
    ParsingHelper helper;
    helper.data = std::make_shared<Data>(type, Buffer(data), count, false);
    helper.cursor = std::make_shared<DataCursor>();
    for (auto& key: propertyKeys)
        header.userData.insert(element.name, key, std::move(helper));

    element.create_properties(propertyKeys, type, listType, listCount);
}

void FileOut::
add_properties_to_element(const std::string& elementKey,
                          const std::vector<std::string>& propertyKeys,
                          const Type type,
                          const size_t count,
                          uint8_t const* data,
                          const Type listType,
                          const size_t listCount)
{
    ParsingHelper helper;
    helper.data = std::make_shared<Data>(type, Buffer(data), count, false);
    helper.cursor = std::make_shared<DataCursor>();
    for (auto& key: propertyKeys)
        header.userData.insert(elementKey, key, std::move(helper));

    auto e = header.find_element(elementKey);
    if (!e)
        e = &header.elements.emplace_back(elementKey, count);
    e->create_properties(propertyKeys, type, listType, listCount);
}


void FileOut::
write_scalar_ascii(const Type t,
                   std::ostream& os,
                   const uint8_t* src,
                   size_t& srcOffset) const
{
    srcOffset += types.at(t).write_ascii(os, src);
    os << " ";
}

void FileOut::
write_scalar_binary(std::ostream& os,
                    uint8_t const* src,
                    size_t& srcOffset,
                    const size_t stride) const noexcept
{
    os.write(reinterpret_cast<const char*>(src), stride);
    srcOffset += stride;
}

void FileOut::
write(std::ostream& os,
      const bool asBinary)
{
    for (auto& d: header.userData.get())
        d.second.cursor->byteOffset = 0;

    header.isBinary = asBinary;
    header.isBigEndian = false;
    asBinary
        ? write_binary(os)
        : write_ascii(os);
}

void FileOut::
write(const std::filesystem::path& p,
      const bool asBinary)
{
    const auto a = asBinary ? std::ios::out
                            : std::ios::out | std::ios::binary;
    std::ofstream ost(p, a);
    if (ost.fail())
        throw std::runtime_error("failed to open " + p.string());

        std::cout << "writing to " << p.string() << std::endl;

    write(ost, asBinary);
}

void FileOut::
write_binary(std::ostream& os) noexcept
{
    header.isBinary = true;
    header.write(os);

    uint8_t listSize[4] = {}; //{ 0, 0, 0, 0 };
    size_t dummyCount {};

    auto element_property_lookup = header.make_property_lookup_table();

    for (size_t element_idx {};
         auto& e: header.elements) {
        for (size_t i {}; i < e.size; ++i) {
            for (size_t property_index {};
                 auto& p : e.properties) {

                auto& f = element_property_lookup[element_idx][property_index];
                auto* helper = f.helper;

                if (f.skip || helper == nullptr)
                    continue;

                if (p.is_list()) {

                    std::memcpy(listSize, &p.listCount, sizeof(uint32_t));
                    write_scalar_binary(os,
                                        listSize,
                                        dummyCount,
                                        f.list_stride);
                    write_scalar_binary(
                        os,
                        helper->data->buffer.get() + helper->cursor->byteOffset,
                        helper->cursor->byteOffset,
                        f.prop_stride * p.listCount
                    );
                }
                else
                    write_scalar_binary(
                        os,
                        helper->data->buffer.get() + helper->cursor->byteOffset,
                        helper->cursor->byteOffset,
                        f.prop_stride
                    );
                property_index++;
            }
        }
        element_idx++;
    }
}

void FileOut::
write_ascii(std::ostream& os) noexcept
{
    header.write(os);

    auto element_property_lookup = header.make_property_lookup_table();

    for (size_t element_idx = 0;
         auto& e: header.elements) {
        for (size_t i {}; i < e.size; ++i) {
            for (size_t property_index = 0;
                 auto& p: e.properties) {

                auto& f = element_property_lookup[element_idx][property_index];
                auto* helper = f.helper;

                if (f.skip || helper == nullptr)
                    continue;

                if (p.is_list()) {

                    os << p.listCount << " ";
                    for (size_t j {}; j < p.listCount; ++j)

                        write_scalar_ascii(
                            p.scalarType,
                            os,
                            helper->data->buffer.get() + helper->cursor->byteOffset,
                            helper->cursor->byteOffset
                        );
                }
                else
                    write_scalar_ascii(
                        p.scalarType,
                        os,
                        helper->data->buffer.get() + helper->cursor->byteOffset,
                        helper->cursor->byteOffset
                    );

                property_index++;
            }
            os << "\n";
        }
        element_idx++;
    }
}

}  // namespace tinyply::impl

#endif  // TINYPLY_LINK_AS_LIBRARY
#endif  // TINYPLY_IMPL_FILE_OUT_H
