/*
                                    88888888
                                  888888888888
                                 88888888888888
                                8888888888888888
                               888888888888888888
                              888888  8888  888888
                              88888    88    88888
                              888888  8888  888888
                              88888888888888888888
                              88888888888888888888
                             8888888888888888888888
                          8888888888888888888888888888
                        88888888888888888888888888888888
                              88888888888888888888
                            888888888888888888888888
                           888888  8888888888  888888
                           888     8888  8888     888
                                   888    888

                                   OCTOBANANA

Licensed under the MIT License

Copyright (c) 2019 Brett Robinson <https://octobanana.com/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ob/prism.hh"

#include <cmath>

#include <tuple>
#include <limits>
#include <iomanip>

namespace OB::Prism {

template<class T>
static typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type almost_equal(T x, T y, int ulp = 2) {
  return std::fabs(x-y) <= std::numeric_limits<T>::epsilon() * std::fabs(x+y) * ulp || std::fabs(x-y) < std::numeric_limits<T>::min();
}

static std::string hex_encode(std::uint8_t const ch) {
  char hex[3] {0};
  if (ch & 0x80) {
    std::snprintf(&hex[0], 3, "%02X", static_cast<unsigned int>(ch & 0xff));
  }
  else {
    std::snprintf(&hex[0], 3, "%02X", static_cast<unsigned int>(ch));
  }
  return std::string(hex);
}

static std::uint8_t hex_decode(std::string_view const str) {
  unsigned int hex {0};
  std::sscanf(str.data(), "%02X", &hex);
  return static_cast<std::uint8_t>(hex);
}

static void uppercase(std::string& str) {
  for (char& c : str) {
    if (c >= 'a' && c <= 'z') {
      c += 'A' - 'a';
    }
  }
}

static void lowercase(std::string& str) {
  for (char& c : str) {
    if (c >= 'A' && c <= 'Z') {
      c += 'a' - 'A';
    }
  }
}

Hex::Hex(std::string&& hex) {
  str(hex);
}

Hex::Hex(std::string const& hex) {
  str(hex);
}

Hex::Hex(std::string_view const hex) {
  str(hex);
}

Hex::Hex(char const* hex) {
  str(hex);
}

Hex::Hex(RGBA&& rgba) {
  from_rgba(rgba);
}

Hex::Hex(RGBA const& rgba) {
  from_rgba(rgba);
}

Hex::Hex(HSLA&& hsla) {
  from_hsla(hsla);
}

Hex::Hex(HSLA const& hsla) {
  from_hsla(hsla);
}

Hex::operator std::string() {
  return _str;
}

Hex& Hex::operator=(std::string&& hex) {
  *this = Hex(hex);
  return *this;
}

Hex& Hex::operator=(std::string const& hex) {
  *this = Hex(hex);
  return *this;
}

Hex& Hex::operator=(std::string_view const hex) {
  *this = Hex(hex);
  return *this;
}

Hex& Hex::operator=(char const* hex) {
  *this = Hex(hex);
  return *this;
}

Hex& Hex::operator=(RGBA&& rgba) {
  *this = Hex(rgba);
  return *this;
}

Hex& Hex::operator=(RGBA const& rgba) {
  *this = Hex(rgba);
  return *this;
}

Hex& Hex::operator=(HSLA&& hsla) {
  *this = Hex(hsla);
  return *this;
}

Hex& Hex::operator=(HSLA const& hsla) {
  *this = Hex(hsla);
  return *this;
}

bool operator==(Hex const& lhs, Hex const& rhs) {
  return lhs.str() == rhs.str();
}

bool operator!=(Hex const& lhs, Hex const& rhs) {
  return lhs.str() != rhs.str();
}

std::ostream& operator<<(std::ostream& os, Hex const& obj) {
  os << "#" << obj.str();
  return os;
}

std::string const& Hex::str() const {
  return _str;
}

void Hex::str(std::string_view const hex) {
  _str.clear();
  _str.reserve(8);
  switch (hex.size()) {
    case 3: {
      _str += hex.at(0);
      _str += hex.at(0);
      _str += hex.at(1);
      _str += hex.at(1);
      _str += hex.at(2);
      _str += hex.at(2);
      _str += "FF";
      break;
    }
    case 4: {
      _str += hex.at(0);
      _str += hex.at(0);
      _str += hex.at(1);
      _str += hex.at(1);
      _str += hex.at(2);
      _str += hex.at(2);
      _str += hex.at(3);
      _str += hex.at(3);
      break;
    }
    case 6: {
      _str = hex;
      _str += "FF";
      break;
    }
    case 8: {
      _str = hex;
      break;
    }
    default: {
      throw std::runtime_error("failed to convert string to Color::Hex");
    }
  }
  uppercase(_str);
}

Hex& Hex::from_rgba(RGBA const& rgba) {
  _str.reserve(8);
  _str += hex_encode(rgba.r());
  _str += hex_encode(rgba.g());
  _str += hex_encode(rgba.b());
  _str += hex_encode(rgba.a());
  return *this;
}

Hex& Hex::from_hsla(HSLA const& hsla) {
  return from_rgba(RGBA(hsla));
}

RGBA::RGBA(std::uint8_t const r, std::uint8_t const g, std::uint8_t const b, std::uint8_t const a) : _r {r}, _g {g}, _b {b}, _a {a} {}

RGBA::RGBA(int const r, int const g, int const b, double const a) : _r {static_cast<std::uint8_t>(r)}, _g {static_cast<std::uint8_t>(g)}, _b {static_cast<std::uint8_t>(b)}, _a {static_cast<std::uint8_t>(std::round(a * 255))} {}

RGBA::RGBA(Hex&& hex) {
  from_hex(hex);
}

RGBA::RGBA(Hex const& hex) {
  from_hex(hex);
}

RGBA::RGBA(HSLA&& hsla) {
  from_hsla(hsla);
}

RGBA::RGBA(HSLA const& hsla) {
  from_hsla(hsla);
}

RGBA& RGBA::operator=(Hex&& hex) {
  from_hex(hex);
  return *this;
}

RGBA& RGBA::operator=(Hex const& hex) {
  from_hex(hex);
  return *this;
}

RGBA& RGBA::operator=(HSLA&& hsla) {
  from_hsla(hsla);
  return *this;
}

RGBA& RGBA::operator=(HSLA const& hsla) {
  from_hsla(hsla);
  return *this;
}

RGBA& RGBA::operator+=(RGBA const& obj) {
  // r(((obj.r() * (obj.a() + 1)) + (r() * (256 - obj.a()))) >> 8);
  // g(((obj.g() * (obj.a() + 1)) + (g() * (256 - obj.a()))) >> 8);
  // b(((obj.b() * (obj.a() + 1)) + (b() * (256 - obj.a()))) >> 8);
  // r((obj.r() * obj.a() + r() * (255 - obj.a())) / 255);
  // g((obj.g() * obj.a() + g() * (255 - obj.a())) / 255);
  // b((obj.b() * obj.a() + b() * (255 - obj.a())) / 255);
  // r(((r() * a()) + (obj.r() * obj.a() * (1 - a()))) / (obj.a() + (a() * (1 - obj.a()))));

  if (obj.a() == 255) {
    *this = obj;
    return *this;
  }

  if (obj.a() == 0) {
    return *this;
  }

  r(static_cast<std::uint8_t>(((obj.r() * (obj.a() / 255.0)) + ((r() * (a() / 255.0)) * (1.0 - (obj.a() / 255.0)))) / ((obj.a() / 255.0) + ((a() / 255.0) * (1.0 - (obj.a() / 255.0))))));
  g(static_cast<std::uint8_t>(((obj.g() * (obj.a() / 255.0)) + ((g() * (a() / 255.0)) * (1.0 - (obj.a() / 255.0)))) / ((obj.a() / 255.0) + ((a() / 255.0) * (1.0 - (obj.a() / 255.0))))));
  b(static_cast<std::uint8_t>(((obj.b() * (obj.a() / 255.0)) + ((b() * (a() / 255.0)) * (1.0 - (obj.a() / 255.0)))) / ((obj.a() / 255.0) + ((a() / 255.0) * (1.0 - (obj.a() / 255.0))))));
  a(static_cast<std::uint8_t>(((obj.a() / 255.0) + ((a() / 255.0) * (1.0 - (obj.a() / 255.0)))) * 255));

  return *this;
}

RGBA operator+(RGBA lhs, RGBA const& rhs) {
  return lhs += rhs;
}

bool operator<(RGBA const& lhs, RGBA const& rhs) {
  auto lr = lhs.r();
  auto lg = lhs.g();
  auto lb = lhs.b();
  auto la = lhs.a();
  auto rr = rhs.r();
  auto rg = rhs.g();
  auto rb = rhs.b();
  auto ra = rhs.a();
  return std::tie(lr, lg, lb, la) < std::tie(rr, rg, rb, ra);
}

bool operator==(RGBA const& lhs, RGBA const& rhs) {
  auto lr = lhs.r();
  auto lg = lhs.g();
  auto lb = lhs.b();
  auto la = lhs.a();
  auto rr = rhs.r();
  auto rg = rhs.g();
  auto rb = rhs.b();
  auto ra = rhs.a();
  return std::tie(lr, lg, lb, la) == std::tie(rr, rg, rb, ra);
}

bool operator!=(RGBA const& lhs, RGBA const& rhs) {
  auto lr = lhs.r();
  auto lg = lhs.g();
  auto lb = lhs.b();
  auto la = lhs.a();
  auto rr = rhs.r();
  auto rg = rhs.g();
  auto rb = rhs.b();
  auto ra = rhs.a();
  return std::tie(lr, lg, lb, la) != std::tie(rr, rg, rb, ra);
}

std::ostream& operator<<(std::ostream& os, RGBA const& obj) {
  os << std::fixed << std::setprecision(2) << "rgba(" << static_cast<int>(obj.r()) << ", " << static_cast<int>(obj.g()) << ", " << static_cast<int>(obj.b()) << ", " << (std::round((static_cast<float>(obj.a()) / 255) * 100) * 0.01) << ")";
  return os;
}

std::uint8_t RGBA::r() const {
  return _r;
}

RGBA& RGBA::r(std::uint8_t const v) {
  _r = v;
  return *this;
}

std::uint8_t RGBA::g() const {
  return _g;
}

RGBA& RGBA::g(std::uint8_t const v) {
  _g = v;
  return *this;
}

std::uint8_t RGBA::b() const {
  return _b;
}

RGBA& RGBA::b(std::uint8_t const v) {
  _b = v;
  return *this;
}

std::uint8_t RGBA::a() const {
  return _a;
}

RGBA& RGBA::a(std::uint8_t const v) {
  _a = v;
  return *this;
}

RGBA& RGBA::from_hex(Hex const& hex) {
  switch (hex.str().size()) {
    case 3: {
      r(hex_decode(std::string(2, hex.str().at(0))));
      g(hex_decode(std::string(2, hex.str().at(1))));
      b(hex_decode(std::string(2, hex.str().at(2))));
      a(static_cast<std::uint8_t>(255));
      break;
    }
    case 4: {
      r(hex_decode(std::string(2, hex.str().at(0))));
      g(hex_decode(std::string(2, hex.str().at(1))));
      b(hex_decode(std::string(2, hex.str().at(2))));
      a(hex_decode(std::string(2, hex.str().at(3))));
      break;
    }
    case 6: {
      r(hex_decode(hex.str().substr(0, 2)));
      g(hex_decode(hex.str().substr(2, 2)));
      b(hex_decode(hex.str().substr(4, 2)));
      a(static_cast<std::uint8_t>(255));
      break;
    }
    case 8: {
      r(hex_decode(hex.str().substr(0, 2)));
      g(hex_decode(hex.str().substr(2, 2)));
      b(hex_decode(hex.str().substr(4, 2)));
      a(hex_decode(hex.str().substr(6, 2)));
      break;
    }
    default: {
      break;
    }
  }
  return *this;
}

float RGBA::from_hue(float j, float i, float h) const {
  float r;
  if (h < 0) {
    h += 1;
  }
  if (h > 1) {
    h -= 1;
  }
  if (h < 1 / 6.0f) {
    r = j + (i - j) * 6 * h;
  }
  else if (h < 1 / 2.0f) {
    r = i;
  }
  else if (h < 2 / 3.0f) {
    r = j + (i - j) * (2 / 3.0f - h) * 6;
  }
  else {
    r = j;
  }
  return r;
}

RGBA& RGBA::from_hsla(HSLA const& hsla) {
  float h {hsla.h() / 360.0f};
  float s {hsla.s() / 100.0f};
  float l {hsla.l() / 100.0f};

  float r_ {0};
  float g_ {0};
  float b_ {0};

  if (almost_equal(s, 0.0f)) {
    r_ = l;
    g_ = l;
    b_ = l;
  }
  else {
    float const i {(l < 0.5f) ? (l * (1 + s)) : ((l + s) - (l * s))};
    float const j {2 * l - i};

    r_ = from_hue(j, i, (h + 1 / 3.0f));
    g_ = from_hue(j, i, h);
    b_ = from_hue(j, i, (h - 1 / 3.0f));
  }

  r(static_cast<std::uint8_t>(std::round(r_ * 255)));
  g(static_cast<std::uint8_t>(std::round(g_ * 255)));
  b(static_cast<std::uint8_t>(std::round(b_ * 255)));
  a(hsla.a());
  return *this;
}

HSLA::HSLA(int const h, float const s, float const l, std::uint8_t const a) : _h {h}, _s {s}, _l {l}, _a {a} {}

HSLA::HSLA(int const h, float const s, float const l, double const a) : _h {h}, _s {s}, _l {l}, _a {static_cast<std::uint8_t>(std::round(a * 255))} {}

HSLA::HSLA(Hex&& hex) {
  from_hex(hex);
}

HSLA::HSLA(Hex const& hex) {
  from_hex(hex);
}

HSLA::HSLA(RGBA&& rgba) {
  from_rgba(rgba);
}

HSLA::HSLA(RGBA const& rgba) {
  from_rgba(rgba);
}

HSLA& HSLA::operator=(Hex&& hex) {
  return from_hex(hex);
}

HSLA& HSLA::operator=(Hex const& hex) {
  return from_hex(hex);
}

HSLA& HSLA::operator=(RGBA&& rgba) {
  return from_rgba(rgba);
}

HSLA& HSLA::operator=(RGBA const& rgba) {
  return from_rgba(rgba);
}

HSLA& HSLA::operator+=(HSLA const& obj) {
  *this = HSLA(RGBA(*this) + RGBA(obj));
  return *this;
}

HSLA operator+(HSLA lhs, HSLA const& rhs) {
  return lhs += rhs;
}

bool operator<(HSLA const& lhs, HSLA const& rhs) {
  auto lh = lhs.h();
  auto ls = lhs.s();
  auto ll = lhs.l();
  auto la = lhs.a();
  auto rh = rhs.h();
  auto rs = rhs.s();
  auto rl = rhs.l();
  auto ra = rhs.a();
  return std::tie(lh, ls, ll, la) < std::tie(rh, rs, rl, ra);
}

bool operator==(HSLA const& lhs, HSLA const& rhs) {
  auto lh = lhs.h();
  auto ls = lhs.s();
  auto ll = lhs.l();
  auto la = lhs.a();
  auto rh = rhs.h();
  auto rs = rhs.s();
  auto rl = rhs.l();
  auto ra = rhs.a();
  return std::tie(lh, ls, ll, la) == std::tie(rh, rs, rl, ra);
}

bool operator!=(HSLA const& lhs, HSLA const& rhs) {
  auto lh = lhs.h();
  auto ls = lhs.s();
  auto ll = lhs.l();
  auto la = lhs.a();
  auto rh = rhs.h();
  auto rs = rhs.s();
  auto rl = rhs.l();
  auto ra = rhs.a();
  return std::tie(lh, ls, ll, la) != std::tie(rh, rs, rl, ra);
}

std::ostream& operator<<(std::ostream& os, HSLA const& obj) {
  os << std::fixed << std::setprecision(2) << "hsla(" << obj.h() << ", " << obj.s() << "%, " << obj.l() << "%, " << (std::round((static_cast<float>(obj.a()) / 255) * 100) * 0.01) << ")";
  return os;
}

int HSLA::h() const {
  return _h;
}

HSLA& HSLA::h(int const v) {
  _h = v;
  return *this;
}

float HSLA::s() const {
  return _s;
}

HSLA& HSLA::s(float const v) {
  _s = v;
  return *this;
}

float HSLA::l() const {
  return _l;
}

HSLA& HSLA::l(float const v) {
  _l = v;
  return *this;
}

std::uint8_t HSLA::a() const {
  return _a;
}

HSLA& HSLA::a(std::uint8_t const v) {
  _a = v;
  return *this;
}

HSLA& HSLA::from_hex(Hex const& hex) {
  *this = HSLA(RGBA(hex));
  return *this;
}

HSLA& HSLA::from_rgba(RGBA const& rgba) {
  float r {rgba.r() / 255.0f};
  float g {rgba.g() / 255.0f};
  float b {rgba.b() / 255.0f};

  auto const min = std::min(r, std::min(g, b));
  auto const max = std::max(r, std::max(g, b));
  auto const init {(max + min) / 2.0f};

  float h_ {init};
  float s_ {init};
  float l_ {init};

  if (almost_equal(max, min)) {
    h_ = 0;
    s_ = 0;
  }
  else {
    float const v {max - min};
    s_ = v / (1.0f - std::abs((max + min) - 1.0f));

    if (almost_equal(max, r)) {
      h_ = (g - b) / v + (g < b ? 6 : 0);
    }
    else if (almost_equal(max, g)) {
      h_ = (b - r) / v + 2;
    }
    else {
      h_ = (r - g) / v + 4;
    }

    h_ /= 6.0f;
  }

  h(static_cast<int>(h_ * 360));
  s(std::round(s_ * 100));
  l(std::round(l_ * 100));
  a(rgba.a());
  return *this;
}

} // namespace OB::Prism
