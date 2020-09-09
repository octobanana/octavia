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

#ifndef APP_UTIL_HH
#define APP_UTIL_HH

#include "ob/term.hh"
#include "ob/timer.hh"

#include <cstddef>

#include <chrono>
#include <random>
#include <string>
#include <thread>
#include <iostream>
#include <functional>

template<typename T = std::size_t>
T random_range(T l, T u, unsigned int seed = std::random_device{}()) {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<T> distr(l, u);
  return distr(gen);
}

template<typename T = std::chrono::milliseconds>
void sleep(T const& duration) {
  std::this_thread::sleep_for(duration);
}

template<typename T = std::chrono::milliseconds>
std::size_t fn_timer(std::function<void()> const& fn) {
  OB::Timer<std::chrono::steady_clock> timer;
  timer.start();
  fn();
  timer.stop();
  return static_cast<std::size_t>(timer.time<T>().count());
}

void dbg_log(std::string const& head, std::string const& body);

#endif // APP_UTIL_HH
