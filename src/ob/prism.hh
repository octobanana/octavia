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

#ifndef OB_PRISM_HH
#define OB_PRISM_HH

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <memory>
#include <string>
#include <iostream>
#include <string_view>

namespace OB::Prism {

class Hex;
class RGBA;
class HSLA;

class Hex {
public:
  Hex(std::string&& hex);
  Hex(std::string const& hex);
  Hex(std::string_view const hex);
  Hex(char const* hex);
  Hex(RGBA&& rgba);
  Hex(RGBA const& rgba);
  Hex(HSLA&& hsla);
  Hex(HSLA const& hsla);
  Hex() = default;
  Hex(Hex&&) = default;
  Hex(Hex const&) = default;

  ~Hex() = default;

  operator std::string();
  Hex& operator=(std::string&& hex);
  Hex& operator=(std::string const& hex);
  Hex& operator=(std::string_view const hex);
  Hex& operator=(char const* hex);
  Hex& operator=(RGBA&& rgba);
  Hex& operator=(RGBA const& rgba);
  Hex& operator=(HSLA&& hsla);
  Hex& operator=(HSLA const& hsla);
  Hex& operator=(Hex&&) = default;
  Hex& operator=(Hex const&) = default;
  friend bool operator==(Hex const& lhs, Hex const& rhs);
  friend bool operator!=(Hex const& lhs, Hex const& rhs);
  friend std::ostream& operator<<(std::ostream& os, Hex const& obj);

  std::string const& str() const;
  void str(std::string_view const hex);
  Hex& from_rgba(RGBA const& rgba);
  Hex& from_hsla(HSLA const& hsla);

private:
  std::string _str;
};

class RGBA {
public:
  RGBA(std::uint8_t const r, std::uint8_t const g, std::uint8_t const b, std::uint8_t const a);
  RGBA(int const r, int const g, int const b, double const a);
  RGBA(Hex&& hex);
  RGBA(Hex const& hex);
  RGBA(HSLA&& hsla);
  RGBA(HSLA const& hsla);
  RGBA() = default;
  RGBA(RGBA&&) = default;
  RGBA(RGBA const&) = default;

  ~RGBA() = default;

  RGBA& operator=(Hex&& hex);
  RGBA& operator=(Hex const& hex);
  RGBA& operator=(HSLA&& hsla);
  RGBA& operator=(HSLA const& hsla);
  RGBA& operator=(RGBA&&) = default;
  RGBA& operator=(RGBA const&) = default;
  RGBA& operator+=(RGBA const& obj);
  friend RGBA operator+(RGBA lhs, RGBA const& rhs);
  friend bool operator<(RGBA const& lhs, RGBA const& rhs);
  friend bool operator==(RGBA const& lhs, RGBA const& rhs);
  friend bool operator!=(RGBA const& lhs, RGBA const& rhs);
  friend std::ostream& operator<<(std::ostream& os, RGBA const& obj);

  std::uint8_t r() const;
  RGBA& r(std::uint8_t const v);
  std::uint8_t g() const;
  RGBA& g(std::uint8_t const v);
  std::uint8_t b() const;
  RGBA& b(std::uint8_t const v);
  std::uint8_t a() const;
  RGBA& a(std::uint8_t const v);

  RGBA& from_hex(Hex const& hex);
  RGBA& from_hsla(HSLA const& hsla);

private:
  float from_hue(float j, float i, float h) const;

  std::uint8_t _r {0};
  std::uint8_t _g {0};
  std::uint8_t _b {0};
  std::uint8_t _a {0};
};

class HSLA {
public:
  HSLA(int const h, float const s, float const l, std::uint8_t const a);
  HSLA(int const h, float const s, float const l, double const a);
  HSLA(Hex&& hex);
  HSLA(Hex const& hex);
  HSLA(RGBA&& rgba);
  HSLA(RGBA const& rgba);
  HSLA() = default;
  HSLA(HSLA&&) = default;
  HSLA(HSLA const&) = default;

  ~HSLA() = default;

  HSLA& operator=(Hex&& hex);
  HSLA& operator=(Hex const& hex);
  HSLA& operator=(RGBA&& rbga);
  HSLA& operator=(RGBA const& rbga);
  HSLA& operator=(HSLA&&) = default;
  HSLA& operator=(HSLA const&) = default;
  HSLA& operator+=(HSLA const& obj);
  friend HSLA operator+(HSLA lhs, HSLA const& rhs);
  friend bool operator<(HSLA const& lhs, HSLA const& rhs);
  friend bool operator==(HSLA const& lhs, HSLA const& rhs);
  friend bool operator!=(HSLA const& lhs, HSLA const& rhs);
  friend std::ostream& operator<<(std::ostream& os, HSLA const& obj);

  int h() const;
  HSLA& h(int const v);
  float s() const;
  HSLA& s(float const v);
  float l() const;
  HSLA& l(float const v);
  std::uint8_t a() const;
  HSLA& a(std::uint8_t const v);

  HSLA& from_hex(Hex const& hex);
  HSLA& from_rgba(RGBA const& rgba);

private:
  int _h {0};
  float _s {0};
  float _l {0};
  std::uint8_t _a {0};
};

} // namespace OB::Prism

#endif // OB_PRISM_HH
