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

#include "app/window.hh"

#include "ob/text.hh"
#include "ob/term.hh"
#include "ob/prism.hh"
#include "ob/timer.hh"

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

Buffer::Buffer(Size const size, Cell const& cell) {
  this->size(size, cell);
}

void Buffer::operator()(Pos const pos, Cell const& cell) {
  // return on out of bounds
  if (pos.x > _size.x - 1 || pos.y > _size.y - 1) {return;}
  cursor(std::move(pos));
  operator()(cell);
}

void Buffer::operator()(Cell const& cell) {
  auto& val = _value.at(_size.y - _pos.y - 1).at(_pos.x);
  if (cell.style.type != Style::Type::Clear && cell.zidx >= val.zidx) {
    if (cell.style.type == Style::Type::Default) {
      val = cell;
    }
    else {
      val = Cell{cell.zidx, Style{cell.style.type, cell.style.attr, val.style.fg + cell.style.fg, val.style.bg + cell.style.bg}, cell.text != " " ? cell.text : (val.text.size() && cell.style.bg.a() != 255 ? val.text : " ")};
    }
  }
}

void Buffer::put(Pos const pos, Cell const& cell) {
  // return on out of bounds
  if (pos.x > _size.x - 1 || pos.y > _size.y - 1) {return;}
  cursor(std::move(pos));
  put(cell);
}

void Buffer::put(Cell const& cell) {
  OB::Text::View view {cell.text};
  for (auto const& e : view) {
    if (e.cols == 2) {
      if (_pos.x + 1 == _size.x - 1) {
        _pos.x = 0;
        if (++_pos.y >= _size.y) {
          _pos.y = 0;
        }
      }
      auto& val = _value.at(_size.y - _pos.y - 1).at(_pos.x);
      if (cell.style.type != Style::Type::Clear && cell.zidx >= val.zidx) {
        if (val.zidx == 0 || val.style.type == Style::Type::Clear) {
          val = Cell{cell.zidx, cell.style, std::string{e.str}};
        }
        else {
          val = Cell{cell.zidx, Style{cell.style.type, cell.style.attr, val.style.fg + cell.style.fg, val.style.bg + cell.style.bg}, cell.text != " " ? std::string{e.str} : (val.text.size() && cell.style.bg.a() != 255 ? val.text : " ")};
        }
      }
      if (_pos.x += 2 >= _size.x) {
        _pos.x = 0;
        if (++_pos.y >= _size.y) {
          _pos.y = 0;
        }
      }
    }
    else {
      auto& val = _value.at(_size.y - _pos.y - 1).at(_pos.x);
      if (cell.style.type != Style::Type::Clear && cell.zidx >= val.zidx) {
        if (cell.style.type == Style::Type::Default) {
          val = Cell{cell.zidx, cell.style, std::string{e.str}};
        }
        else {
          val = Cell{cell.zidx, Style{cell.style.type, cell.style.attr, val.style.fg + cell.style.fg, val.style.bg + cell.style.bg}, cell.text != " " ? std::string{e.str} : (val.text.size() && cell.style.bg.a() != 255 ? val.text : " ")};
        }
      }
      if (++_pos.x >= _size.x) {
        _pos.x = 0;
        if (++_pos.y >= _size.y) {
          _pos.y = 0;
        }
      }
    }
  }
}

Cell& Buffer::at(Pos const pos) {
  return _value.at(pos.y).at(pos.x);
}

Cell const& Buffer::at(Pos const pos) const {
  return _value.at(pos.y).at(pos.x);
}

std::vector<Cell>& Buffer::row(std::size_t const y) {
  return _value.at(_size.y - y - 1);
}

std::vector<Cell> const& Buffer::row(std::size_t const y) const {
  return _value.at(_size.y - y - 1);
}

Cell& Buffer::col(Pos const pos) {
  return _value.at(_size.y - pos.y - 1).at(pos.x);
}

Cell const& Buffer::col(Pos const pos) const {
  return _value.at(_size.y - pos.y - 1).at(pos.x);
}

Pos Buffer::cursor() const {
  return _pos;
}

void Buffer::cursor(Pos const pos) {
  _pos = pos;
}

Size Buffer::size() const {
  return _size;
}

void Buffer::size(Size const size, Cell const& cell) {
  _size = size;
  _pos = Pos();
  _value.clear();
  for (std::size_t h = 0; h < _size.y; ++h) {
    _value.emplace_back();
    for (std::size_t w = 0; w < _size.x; ++w) {
      _value.back().emplace_back(cell);
    }
  }
}

bool Buffer::empty() const {
  return _value.empty();
}

void Buffer::clear() {
  _pos = Pos();
  _size = Size();
  _value.clear();
}

void Window::winch() {
  clear = true;
  style = style_base;
  buf.size(size, Cell{0, style, " "});
  buf_prev = buf;
}

void Window::refresh() {
  clear = true;
  style = style_base;
  buf_prev = buf;
}

void Window::render() {
  if (clear) {
    clear = false;
    line += aec::cursor_set(1, buf.size().y);
    line += aec::clear;
    if (style.type != Style::Type::Clear) {
      if (style.attr != Style::Null) {
        if (style.attr & Style::Bold) {line += aec::bold;}
        if (style.attr & Style::Reverse) {line += aec::reverse;}
        if (style.attr & Style::Underline) {line += aec::underline;}
      }
      if (style.type == Style::Bit_24) {
        term_fg(line, style.fg);
        term_bg(line, style.bg);
      }
    }
    line += aec::screen_clear;
    write(line);
  }

  for (std::size_t y = 0; y < buf.size().y; ++y) {
    for (std::size_t x = 0; x < buf.size().x; ++x) {
      auto const& cell = buf.at(Pos(x, y));
      auto const& prev = buf_prev.at(Pos(x, y));

      if ((cell.text != prev.text) ||
          (cell.style.attr != prev.style.attr) ||
          (cell.style.type != prev.style.type) ||
          (cell.style.fg != prev.style.fg) ||
          (cell.style.bg != prev.style.bg)) {

        bool diff_attr {style.attr != cell.style.attr};
        bool diff_type {style.type != cell.style.type};
        bool diff_fg {style.fg != cell.style.fg};
        bool diff_bg {style.bg != cell.style.bg};

        line += aec::cursor_set(x + 1, y + 1);

        if (diff_type) {
          style.type = cell.style.type;
          if (style.type == Style::Type::Default) {
            line += aec::clear;
          }
        }

        if (diff_attr) {
          diff_fg = true;
          diff_bg = true;
          style.attr = cell.style.attr;
          line += aec::clear;
          if (style.attr != Style::Null) {
            if (style.attr & Style::Bold) {line += aec::bold;}
            if (style.attr & Style::Reverse) {line += aec::reverse;}
            if (style.attr & Style::Underline) {line += aec::underline;}
          }
        }

        if (cell.style.type == Style::Type::Clear) {
          style = Style();
          line += aec::clear;
        }
        else if (cell.style.type == Style::Bit_24) {
          if (diff_fg) {
            style.fg = cell.style.fg;
            term_fg(line, style.fg);
          }

          if (diff_bg) {
            style.bg = cell.style.bg;
            term_bg(line, style.bg);
          }
        }

        line += cell.text;
      }
    }
    bsize += line.size();
    write(line);
  }

  bsize = 0;
  buf_prev = buf;
  buf.size(buf.size(), Cell{0, style_base, " "});
  ++frames;
}

void Window::write(std::string& str) {
  if (str.empty()) {return;}
  int num {0};
  char const* ptr {str.data()};
  std::size_t size {str.size()};
  while (size > 0 && static_cast<std::size_t>(num = ::write(STDOUT_FILENO, ptr, size)) != size) {
    if (num < 0) {
      if (errno == EINTR || errno == EAGAIN) {continue;}
      throw std::runtime_error("write failed");
    }
    size -= static_cast<std::size_t>(num);
    ptr += static_cast<std::size_t>(num);
  }
  str.clear();
}

void Window::render_file(std::ofstream& file) {
  for (std::size_t y = 0; y < buf.size().y; ++y) {
    for (std::size_t x = 0; x < buf.size().x; ++x) {
      auto const& cell = buf.at(Pos(x, y));
      line += cell.text;
    }
    bsize += line.size();
    line += "\n";
    write(line);
    // write_file(file, line);
  }

  bsize = 0;
  buf_prev = buf;
  buf.size(buf.size(), Cell{0, style_base, " "});
  ++frames;
}

void Window::write_file(std::ofstream& file, std::string& str) {
  if (str.empty()) {return;}
  file.seekp(0);
  file << str << std::flush;
  if (!file) {throw std::runtime_error("write failed");}
  str.clear();
}

void Window::term_fg(std::string& str, OB::Prism::RGBA rgba) {
  str += "\x1b[38;2;";
  str += std::to_string(rgba.r());
  str += ";";
  str += std::to_string(rgba.g());
  str += ";";
  str += std::to_string(rgba.b());
  str += "m";
}

void Window::term_bg(std::string& str, OB::Prism::RGBA rgba) {
  str += "\x1b[48;2;";
  str += std::to_string(rgba.r());
  str += ";";
  str += std::to_string(rgba.g());
  str += ";";
  str += std::to_string(rgba.b());
  str += "m";
}
