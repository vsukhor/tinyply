/*
 * Self-contained tests for tinyply: every PLY file is generated in memory or
 * in the system temp directory, so no external assets are required.
 *
 * This software is in the public domain. Where that dedication is not
 * recognized, you are granted a perpetual, irrevocable license to copy,
 * distribute, and modify this file as you see fit.
 *
 * Authored by Valerii Sukhorukov (vsukhorukov@yahoo.com, https://github.com/vsukhor)
 */

// The implementation is compiled in tinyply_impl.cpp. Including the headers
// here without TINYPLY_AS_LIBRARY checks that they link from several
// translation units.
#include "tinyply.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <vector>

namespace tinyply::tests {

namespace fs = std::filesystem;
using Data = Reader::Data;

// Checks ======================================================================

// Not `assert`: the checks must also run in Release builds.

int failures {};

void check(const bool ok,
           const char* expr,
           const char* file,
           const int line)
{
    if (ok)
        return;

    ++failures;
    std::cerr << file << ":" << line << ": check failed: " << expr << std::endl;
}

#define CHECK(cond) check((cond), #cond, __FILE__, __LINE__)

#define CHECK_THROWS(expr, E)                                       \
    do {                                                            \
        bool thrown {};                                             \
        try { expr; }                                               \
        catch (const E&) { thrown = true; }                         \
        check(thrown, #expr " throws " #E, __FILE__, __LINE__);     \
    } while (false)

// Helpers =====================================================================

// Serializes values into a binary PLY payload in the requested byte order.
// Like tinyply itself, assumes a little-endian host.
struct Payload {

    bool bigEndian {};
    std::string bytes;

    explicit Payload(const bool bigEndian = false)
        : bigEndian {bigEndian}
    {}

    template<typename T>
    Payload& put(const T v)
    {
        char b[sizeof(T)];
        std::memcpy(b, &v, sizeof(T));
        if (bigEndian)
            std::reverse(b, b + sizeof(T));
        bytes.append(b, sizeof(T));

        return *this;
    }
};

// A stream buffer without seek support, like a pipe.
struct NoSeekBuf : std::streambuf {

    std::string s;

    explicit NoSeekBuf(std::string str)
        : s {std::move(str)}
    {
        setg(s.data(), s.data(), s.data() + s.size());
    }
};

// A file in the temp directory, removed on destruction.
struct TempFile {

    fs::path path;

    explicit TempFile(const std::string& name)
        : path {fs::temp_directory_path() / ("tinyply-test-" + name)}
    {}

    ~TempFile()
    {
        std::error_code ec;
        fs::remove(path, ec);
    }
};

template<typename T>
const uint8_t* bytes_of(const std::vector<T>& v)
{
    return reinterpret_cast<const uint8_t*>(v.data());
}

template<typename T>
std::vector<T> values(const Data& d)
{
    std::vector<T> v(d.buffer.size_bytes() / sizeof(T));
    std::memcpy(v.data(), d.buffer.get(), v.size() * sizeof(T));

    return v;
}

// Two vertices (x, y) and two triangles, in ASCII.
const std::string trianglesAscii =
    "ply\nformat ascii 1.0\n"
    "element vertex 3\nproperty float x\nproperty float y\n"
    "element face 2\nproperty list uchar int vertex_indices\n"
    "end_header\n"
    "0 0\n1 0\n0 1\n"
    "3 0 1 2\n3 2 1 0\n";

// Round trips =================================================================

void round_trip(const bool asBinary)
{
    const std::vector<float> xyz {0, 0, 0,  1, 0, 0,  0, 1, 0,  1.5f, -2.25f, 1e6f};
    const std::vector<uint8_t> rgb {255, 0, 0,  0, 255, 0,  0, 0, 255,  9, 9, 9};
    const std::vector<uint32_t> tris {0, 1, 2,  1, 3, 2};

    TempFile f {asBinary ? "round-trip-binary.ply" : "round-trip-ascii.ply"};
    {
        Writer w;
        w.add_properties_to_element("vertex", {"x", "y", "z"}, Type::FLOAT32,
                                    4, bytes_of(xyz), Type::INVALID, 0);
        w.add_properties_to_element("vertex", {"red", "green", "blue"}, Type::UINT8,
                                    4, bytes_of(rgb), Type::INVALID, 0);
        w.add_properties_to_element("face", {"vertex_indices"}, Type::UINT32,
                                    2, bytes_of(tris), Type::UINT8, 3);
        w.add_comment("round trip");
        w.write(f.path, asBinary);
    }

    std::ifstream is(f.path, std::ios::binary);
    Reader r;
    CHECK(r.parse_header(is));
    CHECK(r.is_binary() == asBinary);
    CHECK(r.comments() == std::vector<std::string> {"round trip"});

    auto v = r.request_properties_from_element("vertex", {"x", "y", "z"});
    auto c = r.request_properties_from_element("vertex", {"red", "green", "blue"});
    auto t = r.request_properties_from_element("face", {"vertex_indices"});
    r.read(is);

    CHECK(v->t == Type::FLOAT32);
    CHECK(v->count == 4);
    CHECK(values<float>(*v) == xyz);
    CHECK(values<uint8_t>(*c) == rgb);
    CHECK(t->isList);
    CHECK(t->count == 2);
    CHECK(values<uint32_t>(*t) == tris);
}

void ascii_round_trip()  { round_trip(false); }
void binary_round_trip() { round_trip(true); }

// Writer ======================================================================

void writer_add_element_first()
{
    const std::vector<float> xyz {1, 2, 3,  4, 5, 6};

    TempFile f {"add-element-first.ply"};
    {
        Writer w;
        w.add_element("vertex");
        w.add_properties_to_element("vertex", {"x", "y", "z"}, Type::FLOAT32,
                                    2, bytes_of(xyz), Type::INVALID, 0);
        w.write(f.path, false);
    }

    std::ifstream is(f.path, std::ios::binary);
    Reader r;
    r.parse_header(is);
    auto v = r.request_properties_from_element("vertex", {"x", "y", "z"});
    r.read(is);

    CHECK(v->count == 2);
    CHECK(values<float>(*v) == xyz);
}

void writer_rejects_bad_properties()
{
    const std::vector<float> xyz {1, 2, 3,  4, 5, 6};

    TempFile f {"rejects-bad-properties.ply"};
    {
        Writer w;
        w.add_properties_to_element("vertex", {"x", "y", "z"}, Type::FLOAT32,
                                    2, bytes_of(xyz), Type::INVALID, 0);

        CHECK_THROWS(w.add_properties_to_element("vertex", {"w"}, Type::FLOAT32,
                                                 3, bytes_of(xyz), Type::INVALID, 0),
                     std::invalid_argument);
        CHECK_THROWS(w.add_properties_to_element("vertex", {"nx", "x"}, Type::FLOAT32,
                                                 2, bytes_of(xyz), Type::INVALID, 0),
                     std::invalid_argument);
        CHECK_THROWS(w.add_properties_to_element("vertex", {"u", "u"}, Type::FLOAT32,
                                                 2, bytes_of(xyz), Type::INVALID, 0),
                     std::invalid_argument);
        w.write(f.path, true);
    }

    // The rejected calls must not leave partially added properties behind.
    std::ifstream is(f.path, std::ios::binary);
    Reader r;
    r.parse_header(is);
    const auto elements = r.get_elements();
    CHECK(elements.size() == 1);
    CHECK(elements[0].properties.size() == 3);
    CHECK(!elements[0].contains("nx"));
}

// Header ======================================================================

void header_structure()
{
    std::istringstream is(trianglesAscii);
    Reader r;
    CHECK(r.parse_header(is));
    CHECK(!r.is_binary());

    const auto elements = r.get_elements();
    CHECK(elements.size() == 2);
    CHECK(elements[0].name == "vertex");
    CHECK(elements[0].size == 3);
    CHECK(elements[0].contains("y"));
    CHECK(elements[0].find_property("y") == 1);
    CHECK(!elements[0].contains("z"));
    CHECK(elements[1].properties[0].is_list());
    CHECK(elements[1].properties[0].listType == Type::UINT8);
    CHECK(elements[1].properties[0].scalarType == Type::INT32);
}

void header_unknown_field()
{
    std::istringstream is("ply\nformat ascii 1.0\nbogus line\n"
                          "element vertex 0\nproperty float x\nend_header\n");
    Reader r;
    CHECK(!r.parse_header(is));
}

void header_invalid_list_count_type()
{
    for (const std::string format: {"ascii", "binary_little_endian"})
        for (const std::string countType: {"float", "double", "int64"}) {

            std::istringstream is("ply\nformat " + format + " 1.0\n"
                                  "element face 1\n"
                                  "property list " + countType + " int idx\n"
                                  "end_header\n");
            Reader r;
            CHECK_THROWS(r.parse_header(is), std::runtime_error);
        }
}

// Reading =====================================================================

void element_property_name_collision()
{
    // "ab" + "c" and "a" + "bc" must remain distinct requests.
    std::istringstream is("ply\nformat ascii 1.0\n"
                          "element ab 1\nproperty int c\n"
                          "element a 1\nproperty float bc\n"
                          "end_header\n5\n2.5\n");
    Reader r;
    r.parse_header(is);
    auto abc = r.request_properties_from_element("ab", {"c"});
    auto abc2 = r.request_properties_from_element("a", {"bc"});
    r.read(is);

    CHECK(values<int32_t>(*abc) == std::vector<int32_t> {5});
    CHECK(values<float>(*abc2) == std::vector<float> {2.5f});
}

void list_size_hint_single_pass()
{
    // With a hint, the file is read in one pass, so no seeking is needed.
    NoSeekBuf buf {trianglesAscii};
    std::istream is(&buf);
    Reader r;
    r.parse_header(is);
    auto t = r.request_properties_from_element("face", {"vertex_indices"}, 3);
    r.read(is);

    CHECK(values<int32_t>(*t) == (std::vector<int32_t> {0, 1, 2, 2, 1, 0}));
}

void two_pass_needs_seekable_stream()
{
    NoSeekBuf buf {trianglesAscii};
    std::istream is(&buf);
    Reader r;
    r.parse_header(is);
    r.request_properties_from_element("face", {"vertex_indices"});

    CHECK_THROWS(r.read(is), std::runtime_error);
}

void mixed_list_count_widths(const bool bigEndian)
{
    // A uint32 count of 258 is followed by uchar counts: no stale upper bytes
    // may leak into the narrower counts. Also checks big-endian scalars.
    std::string header = std::string("ply\nformat binary_") +
                         (bigEndian ? "big" : "little") + "_endian 1.0\n"
                         "element a 1\nproperty list uint int p\n"
                         "element f 2\nproperty list uchar ushort q\n"
                         "property float w\n"
                         "end_header\n";

    Payload p {bigEndian};
    p.put<uint32_t>(258);
    for (int32_t i {}; i < 258; ++i)
        p.put(i);
    for (uint16_t k {}; k < 2; ++k)
        p.put<uint8_t>(2).put<uint16_t>(10 + k).put<uint16_t>(20 + k).put(0.5f + k);

    std::istringstream is(header + p.bytes);
    Reader r;
    r.parse_header(is);
    auto q = r.request_properties_from_element("f", {"q"});
    auto w = r.request_properties_from_element("f", {"w"});
    r.read(is);

    CHECK(values<uint16_t>(*q) == (std::vector<uint16_t> {10, 20, 11, 21}));
    CHECK(values<float>(*w) == (std::vector<float> {0.5f, 1.5f}));
}

void mixed_list_count_widths_le() { mixed_list_count_widths(false); }
void mixed_list_count_widths_be() { mixed_list_count_widths(true); }

void mixed_list_count_widths_ascii()
{
    std::string text = "ply\nformat ascii 1.0\n"
                       "element a 1\nproperty list uint int p\n"
                       "element f 1\nproperty list uchar int q\n"
                       "end_header\n258";
    for (int i {}; i < 258; ++i)
        text += " 1";
    text += "\n3 7 8 9\n";

    std::istringstream is(text);
    Reader r;
    r.parse_header(is);
    auto q = r.request_properties_from_element("f", {"q"});
    r.read(is);

    CHECK(values<int32_t>(*q) == (std::vector<int32_t> {7, 8, 9}));
}

void skipped_variable_length_list()
{
    // Mixed triangles and quads, but only the scalar after them is requested.
    std::istringstream is("ply\nformat ascii 1.0\n"
                          "element face 2\n"
                          "property list uchar int vertex_indices\n"
                          "property uchar red\n"
                          "end_header\n"
                          "3 0 1 2 7\n4 0 1 2 3 8\n");
    Reader r;
    r.parse_header(is);
    auto red = r.request_properties_from_element("face", {"red"});
    r.read(is);

    CHECK(values<uint8_t>(*red) == (std::vector<uint8_t> {7, 8}));
}

void requested_variable_length_list()
{
    std::istringstream is("ply\nformat ascii 1.0\n"
                          "element face 2\n"
                          "property list uchar int vertex_indices\n"
                          "end_header\n"
                          "3 0 1 2\n4 0 1 2 3\n");
    Reader r;
    r.parse_header(is);
    r.request_properties_from_element("face", {"vertex_indices"});

    CHECK_THROWS(r.read(is), std::runtime_error);
}

// Malformed input =============================================================

void truncated_binary()
{
    Payload p;
    p.put(1.f).put(2.f);

    std::istringstream is("ply\nformat binary_little_endian 1.0\n"
                          "element vertex 1000\nproperty float x\n"
                          "end_header\n" + p.bytes);
    Reader r;
    r.parse_header(is);
    r.request_properties_from_element("vertex", {"x"});

    CHECK_THROWS(r.read(is), std::runtime_error);
}

void truncated_binary_list()
{
    Payload p;
    p.put<uint8_t>(3).put<int32_t>(0);

    std::istringstream is("ply\nformat binary_little_endian 1.0\n"
                          "element face 1\n"
                          "property list uchar int vertex_indices\n"
                          "end_header\n" + p.bytes);
    Reader r;
    r.parse_header(is);
    r.request_properties_from_element("face", {"vertex_indices"}, 3);

    CHECK_THROWS(r.read(is), std::runtime_error);
}

void truncated_ascii()
{
    std::istringstream is("ply\nformat ascii 1.0\n"
                          "element vertex 3\nproperty float x\n"
                          "end_header\n1 2\n");
    Reader r;
    r.parse_header(is);
    r.request_properties_from_element("vertex", {"x"});

    CHECK_THROWS(r.read(is), std::runtime_error);
}

void negative_list_count()
{
    {
        std::istringstream is("ply\nformat ascii 1.0\n"
                              "element face 1\nproperty list char int q\n"
                              "end_header\n-1 3\n");
        Reader r;
        r.parse_header(is);
        r.request_properties_from_element("face", {"q"});

        CHECK_THROWS(r.read(is), std::runtime_error);
    }
    {
        Payload p;
        p.put<int8_t>(-1).put<int32_t>(3);

        std::istringstream is("ply\nformat binary_little_endian 1.0\n"
                              "element face 1\nproperty list char int q\n"
                              "end_header\n" + p.bytes);
        Reader r;
        r.parse_header(is);
        r.request_properties_from_element("face", {"q"});

        CHECK_THROWS(r.read(is), std::runtime_error);
    }
}

// Runner ======================================================================

struct Test {

    const char* name;
    void (*run)();
};

const Test all[] {
    {"ascii_round_trip",                ascii_round_trip},
    {"binary_round_trip",               binary_round_trip},
    {"writer_add_element_first",        writer_add_element_first},
    {"writer_rejects_bad_properties",   writer_rejects_bad_properties},
    {"header_structure",                header_structure},
    {"header_unknown_field",            header_unknown_field},
    {"header_invalid_list_count_type",  header_invalid_list_count_type},
    {"element_property_name_collision", element_property_name_collision},
    {"list_size_hint_single_pass",      list_size_hint_single_pass},
    {"two_pass_needs_seekable_stream",  two_pass_needs_seekable_stream},
    {"mixed_list_count_widths_le",      mixed_list_count_widths_le},
    {"mixed_list_count_widths_be",      mixed_list_count_widths_be},
    {"mixed_list_count_widths_ascii",   mixed_list_count_widths_ascii},
    {"skipped_variable_length_list",    skipped_variable_length_list},
    {"requested_variable_length_list",  requested_variable_length_list},
    {"truncated_binary",                truncated_binary},
    {"truncated_binary_list",           truncated_binary_list},
    {"truncated_ascii",                 truncated_ascii},
    {"negative_list_count",             negative_list_count},
};

}  // namespace tinyply::tests

int main()
{
    using namespace tinyply::tests;

    int failed {};
    for (const auto& [name, run]: all) {

        const int before = failures;
        try {
            run();
        }
        catch (const std::exception& e) {
            ++failures;
            std::cerr << name << ": unexpected exception: " << e.what() << std::endl;
        }

        const bool ok = failures == before;
        failed += !ok;
        std::cout << (ok ? "[ OK ] " : "[FAIL] ") << name << std::endl;
    }

    std::cout << std::size(all) - failed << "/" << std::size(all)
              << " tests passed" << std::endl;

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
