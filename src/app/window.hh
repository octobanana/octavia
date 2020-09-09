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

Copyright (c) 2020 Brett Robinson <https://octobanana.com/>

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

#ifndef WINDOW_HH
#define WINDOW_HH

#include "ob/text.hh"
#include "ob/term.hh"
#include "ob/prism.hh"

#include <cmath>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <tuple>
#include <deque>
#include <regex>
#include <chrono>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <complex>
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace aec = OB::Term::ANSI_Escape_Codes;

template<typename T>
struct Vec2n;

template<typename T>
Vec2n<T> operator*(Vec2n<T> lhs, Vec2n<T> const& rhs);

template<typename T>
Vec2n<T> operator/(Vec2n<T> lhs, Vec2n<T> const& rhs);

template<typename T>
Vec2n<T> operator+(Vec2n<T> lhs, Vec2n<T> const& rhs);

template<typename T>
Vec2n<T> operator-(Vec2n<T> lhs, Vec2n<T> const& rhs);

template<typename T>
std::ostream& operator<<(std::ostream& os, Vec2n<T> const& obj);

template<typename T>
bool operator<(Vec2n<T> const& rhs, Vec2n<T> const& lhs);

template<typename T>
bool operator<=(Vec2n<T> const& rhs, Vec2n<T> const& lhs);

template<typename T>
bool operator>(Vec2n<T> const& rhs, Vec2n<T> const& lhs);

template<typename T>
bool operator>=(Vec2n<T> const& rhs, Vec2n<T> const& lhs);

template<typename T>
bool operator==(Vec2n<T> const& rhs, Vec2n<T> const& lhs);

template<typename T>
bool operator!=(Vec2n<T> const& rhs, Vec2n<T> const& lhs);

template<typename T>
struct Vec2n {
  T x {0};
  T y {0};
  Vec2n(T const x, T const y) : x {x}, y {y} {}
  Vec2n() = default;
  Vec2n(Vec2n<T>&&) = default;
  Vec2n(Vec2n<T> const&) = default;
  template<typename N>
  Vec2n(Vec2n<N>&& obj) {
    x = static_cast<T>(obj.x);
    y = static_cast<T>(obj.y);
  }
  template<typename N>
  Vec2n(Vec2n<N> const& obj) {
    x = static_cast<T>(obj.x);
    y = static_cast<T>(obj.y);
  }

  ~Vec2n() = default;

  Vec2n<T>& operator=(Vec2n<T>&&) = default;
  Vec2n<T>& operator=(Vec2n<T> const&) = default;
  Vec2n<T>& operator*=(Vec2n<T> const& obj);
  Vec2n<T>& operator/=(Vec2n<T> const& obj);
  Vec2n<T>& operator+=(Vec2n<T> const& obj);
  Vec2n<T>& operator-=(Vec2n<T> const& obj);

  friend Vec2n<T> operator* <>(Vec2n<T> lhs, Vec2n<T> const& rhs);
  friend Vec2n<T> operator/ <>(Vec2n<T> lhs, Vec2n<T> const& rhs);
  friend Vec2n<T> operator+ <>(Vec2n<T> lhs, Vec2n<T> const& rhs);
  friend Vec2n<T> operator- <>(Vec2n<T> lhs, Vec2n<T> const& rhs);
  friend std::ostream& operator<< <>(std::ostream& os, Vec2n<T> const& obj);
  friend bool operator< <>(Vec2n<T> const& lhs, Vec2n<T> const& rhs);
  friend bool operator<= <>(Vec2n<T> const& lhs, Vec2n<T> const& rhs);
  friend bool operator> <>(Vec2n<T> const& lhs, Vec2n<T> const& rhs);
  friend bool operator>= <>(Vec2n<T> const& lhs, Vec2n<T> const& rhs);
  friend bool operator== <>(Vec2n<T> const& lhs, Vec2n<T> const& rhs);
  friend bool operator!= <>(Vec2n<T> const& lhs, Vec2n<T> const& rhs);
};

template<typename T>
std::ostream& operator<<(std::ostream& os, Vec2n<T> const& obj) {
  os << obj.x << ":" << obj.y;
  return os;
}

template<typename T>
Vec2n<T>& Vec2n<T>::operator*=(Vec2n<T> const& obj) {
  x *= obj.x;
  y *= obj.y;
  return *this;
}

template<typename T>
Vec2n<T> operator*(Vec2n<T> lhs, Vec2n<T> const& rhs) {
  return lhs *= rhs;
}

template<typename T>
Vec2n<T>& Vec2n<T>::operator/=(Vec2n<T> const& obj) {
  x /= obj.x;
  y /= obj.y;
  return *this;
}

template<typename T>
Vec2n<T> operator/(Vec2n<T> lhs, Vec2n<T> const& rhs) {
  return lhs /= rhs;
}

template<typename T>
Vec2n<T>& Vec2n<T>::operator+=(Vec2n<T> const& obj) {
  x += obj.x;
  y += obj.y;
  return *this;
}

template<typename T>
Vec2n<T> operator+(Vec2n<T> lhs, Vec2n<T> const& rhs) {
  return lhs += rhs;
}

template<typename T>
Vec2n<T>& Vec2n<T>::operator-=(Vec2n<T> const& obj) {
  x -= obj.x;
  y -= obj.y;
  return *this;
}

template<typename T>
Vec2n<T> operator-(Vec2n<T> lhs, Vec2n<T> const& rhs) {
  return lhs -= rhs;
}

template<typename T>
bool operator<(Vec2n<T> const& lhs, Vec2n<T> const& rhs) {
  return (lhs.x < rhs.x) && (lhs.y < rhs.y);
}

template<typename T>
bool operator<=(Vec2n<T> const& lhs, Vec2n<T> const& rhs) {
  return (lhs.x <= rhs.x) && (lhs.y <= rhs.y);
}

template<typename T>
bool operator>=(Vec2n<T> const& lhs, Vec2n<T> const& rhs) {
  return (lhs.x >= rhs.x) && (lhs.y >= rhs.y);
}

template<typename T>
bool operator>(Vec2n<T> const& lhs, Vec2n<T> const& rhs) {
  return (lhs.x > rhs.x) && (lhs.y > rhs.y);
}

template<typename T>
bool operator==(Vec2n<T> const& lhs, Vec2n<T> const& rhs) {
  return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

template<typename T>
bool operator!=(Vec2n<T> const& lhs, Vec2n<T> const& rhs) {
  return !(lhs == rhs);
}

template<typename T>
std::optional<T> slope(Vec2n<T> const& p1, Vec2n<T> const& p2) {
  return p1.x == p2.x ? std::optional<T>() : (p2.y - p1.y) / (p2.x - p1.x);
}

template<typename T>
T y_intercept(Vec2n<T> const& p, T const& m) {
  return p.y - p.x * m;
}

template<typename T>
std::function<T(T const&)> line(Vec2n<T> const& p1, Vec2n<T> const& p2) {
  if (auto m = slope(p1, p2); m) {
    auto b = y_intercept(p1, m.value());
    return [m = m.value(), b](T const& x) -> T {
      return m * x + b;
    };
  }
  return [](T const& x) -> T {
    return x;
  };
}

template<typename T>
bool contains(Vec2n<T> const& point, Vec2n<T> const& min, Vec2n<T> const& max) {
  return
    (point.x >= min.x && point.x <= max.x) &&
    (point.y >= min.y && point.y <= max.y);
}

template<typename T>
bool intersect(Vec2n<T> const& amin, Vec2n<T> const& amax, Vec2n<T> const& bmin, Vec2n<T> const& bmax) {
  return
    (amin.x <= bmax.x && amax.x >= bmin.x) &&
    (amin.y <= bmax.y && amax.y >= bmin.y);
}

template<typename T>
Vec2n<T> trunc(Vec2n<T> obj) {
  obj.x = std::trunc(obj.x);
  obj.y = std::trunc(obj.y);
  return obj;
}

template<typename T>
Vec2n<T> floor(Vec2n<T> obj) {
  obj.x = std::floor(obj.x);
  obj.y = std::floor(obj.y);
  return obj;
}

template<typename T>
Vec2n<T> ceil(Vec2n<T> obj) {
  obj.x = std::ceil(obj.x);
  obj.y = std::ceil(obj.y);
  return obj;
}

template<typename T>
Vec2n<T> abs(Vec2n<T> obj) {
  obj.x = std::abs(obj.x);
  obj.y = std::abs(obj.y);
  return obj;
}

template<typename T>
Vec2n<T> round(Vec2n<T> obj) {
  obj.x = std::round(obj.x);
  obj.y = std::round(obj.y);
  return obj;
}

using Pos = Vec2n<std::size_t>;
using Size = Vec2n<std::size_t>;
using Vec2f = Vec2n<float>;

struct Style {
  friend std::ostream& operator<<(std::ostream& os, Style const& obj);
  enum Type : std::uint8_t {
    Clear = 0,
    Default,
    Bit_2,
    Bit_4,
    Bit_8,
    Bit_24,
    Bit_32,
  };
  enum Attr : std::uint8_t {
    Null = 0,
    Bold = 1 << 0,
    Reverse = 1 << 1,
    Underline = 1 << 2,
  };
  std::uint8_t type {Clear};
  std::uint8_t attr {Null};
  OB::Prism::RGBA fg;
  OB::Prism::RGBA bg;
};

inline std::ostream& operator<<(std::ostream& os, Style const& obj) {
  os
  << static_cast<int>(obj.attr) << " "
  << static_cast<int>(obj.type) << " "
  << obj.fg << " "
  << obj.bg;
  return os;
}

struct Cell {
  int zidx;
  Style style;
  std::string text {" "};
};

class Buffer {
public:
  Buffer(Size const size, Cell const& cell = {});
  Buffer() = default;
  Buffer(Buffer&&) = default;
  Buffer(Buffer const&) = default;
  ~Buffer() = default;
  Buffer& operator=(Buffer&&) = default;
  Buffer& operator=(Buffer const&) = default;
  void operator()(Pos const pos, Cell const& cell);
  void operator()(Cell const& cell);
  void put(Pos const pos, Cell const& cell);
  void put(Cell const& cell);
  Cell& at(Pos const pos);
  Cell const& at(Pos const pos) const;
  std::vector<Cell>& row(std::size_t const y);
  std::vector<Cell> const& row(std::size_t const y) const;
  Cell& col(Pos const pos);
  Cell const& col(Pos const pos) const;
  Pos cursor() const;
  void cursor(Pos const pos);
  Size size() const;
  void size(Size const size, Cell const& cell = {});
  bool empty() const;
  void clear();

private:
  Pos _pos;
  Size _size;
  std::vector<std::vector<Cell>> _value;
}; // class Buffer

class Window {
public:

// private:
  void winch();
  void refresh();
  void render();
  void write(std::string& str);
  void render_file(std::ofstream& file);
  void write_file(std::ofstream& file, std::string& str);
  void term_fg(std::string& str, OB::Prism::RGBA rgba);
  void term_bg(std::string& str, OB::Prism::RGBA rgba);

  Size size;
  std::size_t frames {0};
  std::size_t bsize {0};
  Style style_base;
  Style style;
  Buffer buf;
  Buffer buf_prev;
  Buffer buf_1;
  Buffer buf_2;
  std::string line;
  bool clear {true};
};

#endif // WINDOW_HH
