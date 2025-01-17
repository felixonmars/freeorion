//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Clr.h
//!
//! Contains the utility class Clr, which represents colors in GG.

#ifndef _GG_Clr_h_
#define _GG_Clr_h_

#include <array>
#if __has_include(<charconv>)
  #include <charconv>
#endif
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <GG/Export.h>


namespace GG {

/** \brief A simple 32-bit structure that can act as a packed 32-bit unsigned
    integer representation of a RGBA color, a vector of the four unsigned
    bytes that compose an RGBA color, or the individual unsigned bytes "a",
    "r", "g", and "b".

    You should not use literals to initialize Color objects; depending on the
    endian-ness of the machine, 0x00FFFFFF would be transparent white
    (little-endian) or opaque yellow (big-endian).*/
struct Clr
{
    [[nodiscard]] constexpr Clr() = default;

    /** ctor that constructs a Clr from four ints that represent the color channels */
    [[nodiscard]] constexpr Clr(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept :
        r(r_), g(g_), b(b_), a(a_)
    {}

    /** ctor that constructs a Clr from std::array that represents the color channels */
    [[nodiscard]] constexpr Clr(std::array<uint8_t, 4> clr) noexcept :
        Clr(clr[0], clr[1], clr[2], clr[3])
    {}

    /** ctor that constructs a Clr from a string that represents the color
        channels in the format 'RRGGBB', 'RRGGBBAA' where each channel value
        ranges from 00 to FF. When the alpha component is left out, the alpha
        value FF is assumed. When characters out of the range 0-9 and A-F are
        passed, results are undefined.
    */
    [[nodiscard]] constexpr Clr(std::string_view hex_colour)
    {
        const auto sz = hex_colour.size();

        auto val_from_two_hex_chars = [](std::string_view chars) -> uint8_t {
            auto digit0 = chars[0];
            auto digit1 = chars[1];
            uint8_t val0 = 16 * (digit0 >= 'A' ? (digit0 - 'A' + 10) : (digit0 - '0'));
            uint8_t val1 = (digit1 >= 'A' ? (digit1 - 'A' + 10) : (digit1 - '0'));
            return val0 + val1;
        };
        static_assert(val_from_two_hex_chars("01") == 1);
        static_assert(val_from_two_hex_chars("FF") == 255);
        static_assert(val_from_two_hex_chars("A0") == 160);
        static_assert(val_from_two_hex_chars("!.") == 14u);

        r = (sz >= 2) ? val_from_two_hex_chars(hex_colour.substr(0, 2)) : 0;
        g = (sz >= 4) ? val_from_two_hex_chars(hex_colour.substr(2, 2)) : 0;
        b = (sz >= 6) ? val_from_two_hex_chars(hex_colour.substr(4, 2)) : 0;
        a = (sz >= 8) ? val_from_two_hex_chars(hex_colour.substr(6, 2)) : 255;
    }

    [[nodiscard]] explicit constexpr operator uint32_t() const noexcept
    { return (r << 24) + (g << 16) + (b << 8) + a; }

    [[nodiscard]] explicit operator std::string() const
    {
#if defined(__cpp_lib_to_chars)     //     '(' "255"  ' '  ')'  0
        std::array<std::string::value_type, 1 + 4*3 + 3*1 + 1 + 1> buf{"("};
        auto it = std::next(buf.data());
        const auto end_it = std::next(buf.data(), buf.size());
        auto result = std::to_chars(it, end_it, r);
        *result.ptr++ = ' ';
        result = std::to_chars(it, end_it, g);
        *result.ptr++ = ' ';
        result = std::to_chars(it, end_it, b);
        *result.ptr++ = ' ';
        result = std::to_chars(it, end_it, a);
        *result.ptr++ = ')';
        return std::string{buf.data()};
#else
        std::string buf;
        buf.reserve(1 + 4*3 + 3*2 + 1 + 1);
        buf.append("(").append(std::to_string(+r)).append(", ").append(std::to_string(+g))
           .append(", ").append(std::to_string(+b)).append(", ").append(std::to_string(+a))
           .append(")");
        return buf;
#endif
    }

    [[nodiscard]] constexpr std::array<uint8_t, 4> RGBA() const noexcept
    { return {r, g, b, a}; }

#if defined(__cpp_impl_three_way_comparison)
    [[nodiscard]] constexpr auto operator<=>(const Clr&) const noexcept = default;
#else
    [[nodiscard]] constexpr bool operator==(const Clr& rhs) const noexcept
    { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; };
    [[nodiscard]] constexpr bool operator!=(const Clr& rhs) const noexcept
    { return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a; };
#endif

    uint8_t r = 0;    ///< the red channel
    uint8_t g = 0;    ///< the green channel
    uint8_t b = 0;    ///< the blue channel
    uint8_t a = 0;    ///< the alpha channel
};

static_assert(uint32_t{Clr{0,0,0,1}} == 1u);
static_assert(uint32_t{Clr{0,0,2,3}} == 2*256u + 3u);
static_assert(uint32_t{Clr{255,1,0,0}} == 256*256*256*255u + 256*256*1u);

static_assert(Clr("A0FF01") == Clr{160, 255, 1, 255});
static_assert(Clr("12345678") == Clr{16*1+2, 16*3+4, 16*5+6, 16*7+8});

inline std::ostream& operator<<(std::ostream& os, Clr clr)
{
    os << "(" << +clr.r << ", " << +clr.g << ", " << +clr.b << ", " << +clr.a << ")";
    return os;
}

//! Returns the lightened version of color clr.  LightenClr leaves the alpha
//! channel unchanged, and multiplies the other channels by some factor.
constexpr Clr LightenClr(Clr clr, float factor = 2.0) noexcept
{
    return Clr(std::min<uint8_t>(clr.r * factor, 255), std::min<uint8_t>(clr.g * factor, 255),
               std::min<uint8_t>(clr.b * factor, 255), clr.a);
}

//! Returns the darkened version of color clr.  DarkenClr leaves the alpha
//! channel unchanged, and divides the other channels by some factor.
constexpr Clr DarkenClr(const Clr clr, float factor = 2.0) noexcept
{
    return Clr(
        static_cast<uint8_t>(clr.r / factor),
        static_cast<uint8_t>(clr.g / factor),
        static_cast<uint8_t>(clr.b / factor),
        clr.a);
}

constexpr Clr InvertClr(const Clr clr) noexcept {
    constexpr uint8_t MAX{std::numeric_limits<decltype(clr.a)>::max()};
    return Clr(MAX - clr.r, MAX - clr.g, MAX - clr.b, clr.a);
}

constexpr Clr BlendClr(Clr src, Clr dst, float factor) noexcept
{
    return Clr(static_cast<uint8_t>(src.r * factor + dst.r * (1 - factor)),
               static_cast<uint8_t>(src.g * factor + dst.g * (1 - factor)),
               static_cast<uint8_t>(src.b * factor + dst.b * (1 - factor)),
               static_cast<uint8_t>(src.a * factor + dst.a * (1 - factor)));
}

/** Named ctor that constructs a Clr from four floats that represent the color
    channels (each must be >= 0.0 and <= 1.0). */
constexpr Clr FloatClr(float r, float g, float b, float a) noexcept
{
    return Clr(static_cast<uint8_t>(r * 255),
               static_cast<uint8_t>(g * 255),
               static_cast<uint8_t>(b * 255),
               static_cast<uint8_t>(a * 255));
}

/** Returns the input Clr scaned by the input factor \a s. */
constexpr Clr operator*(Clr lhs, float s) noexcept
{
    return Clr(static_cast<uint8_t>(lhs.r * s),
               static_cast<uint8_t>(lhs.g * s),
               static_cast<uint8_t>(lhs.b * s),
               static_cast<uint8_t>(lhs.a * s));
}

/** Returns the component-wise sum of input Clrs. */
constexpr Clr operator+(Clr lhs, Clr rhs) noexcept
{ return Clr(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a); }

}


//! Calls the appropriate version of glColor*() with @a clr.
GG_API void glColor(GG::Clr clr);


#endif
