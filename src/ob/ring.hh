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

#ifndef OB_RING_HH
#define OB_RING_HH

#include <vector>
#include <initializer_list>

namespace OB {

template<typename T>
class basic_ring {
  using value_type = typename T::value_type;
  using reference = typename T::reference;
  using const_reference = typename T::const_reference;
  using size_type = typename T::size_type;

public:

  basic_ring(basic_ring&&) = default;

  basic_ring(basic_ring const&) = default;

  explicit basic_ring(size_type const size, value_type const& value = value_type()) : _buffer(size, value) {
  }

  template<typename It>
  basic_ring(It begin, It end) : _buffer(begin, end) {
  }

  basic_ring(T const& buf) : _buffer(buf) {
  }

  basic_ring(T&& buf) : _buffer(buf) {
  }

  basic_ring(std::initializer_list<value_type> init) : _buffer(init) {
  }

  virtual ~basic_ring() = default;

  basic_ring& operator=(basic_ring&&) = default;

  basic_ring& operator=(basic_ring const&) = default;

  reference operator[](size_type const pos) {
    return _buffer[get_pos(pos)];
  }

  const_reference operator[](size_type const pos) const {
    return _buffer[get_pos(pos)];
  }

  size_type size() const noexcept {
    return _buffer.size();
  }

  reference push(value_type const& arg) {
    reference ref = _buffer[_index];
    ref = arg;
    increase_index();
    return ref;
  }

private:
  size_type get_pos(size_type const pos) const noexcept {
    return (_index + pos) % _buffer.size();
  }

  void increase_index() noexcept {
    if (++_index >= _buffer.size()) {_index = 0;}
  }

  size_type _index {0};
  T _buffer;
}; // class basic_ring

template<typename T>
using ring_vector = basic_ring<std::vector<T>>;

} // namespace OB

#endif // OB_RING_HH
