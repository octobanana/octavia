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

#include "app/filter.hh"

#include "ob/ring.hh"

#include <cassert>

#include <string>
#include <algorithm>
#include <stdexcept>


namespace Filter
{

using namespace std::string_literals;

// TODO pass part instead of width
void savitzky_golay(std::vector<double>& bars, std::size_t size, std::size_t width, double const threshold) {
  if (width < 3 || width % 2 != 1) {
    throw std::logic_error("savitzky_golay: invalid width '"s + std::to_string(width) + "', width must be odd and >= 3"s);
  }
  if (size <= 2) {return;}
  auto part = (width - 1) / 2;
  if (size < part + 1) {
    part = size - 1;
    width = part * 2 + 1;
  }
  auto const c = 1.0 / width;

  OB::ring_vector<double> win (width);
  for (std::size_t i = part; i > 0; --i) {
    win.push(bars[i]);
  }
  for (std::size_t i = 0; i < part; ++i) {
    win.push(bars[i]);
  }

  for (std::size_t i = 0; i < size; ++i) {
    double res {0};

    if (i > size - 1 - part) {
      win.push(bars[size - 1 - (i - (size - 1 - part))]);
    }
    else {
      win.push(bars[i + part]);
    }

    for (std::size_t j = 0; j < width; ++j) {
      res += (win[j] * c) + j - part;
    }

    if (threshold == 0.0 || std::abs(bars[i] - res) < threshold) {
      bars[i] = res;
    }
  }
}

// TODO optimize more, try to reduce vector size and copying
// maybe try combining interpolate and decimate steps
// in a loop, interpolate until buffer has enough values to decimate, max buffer size will then be `std::max(interpolate, decimate)` instead of `out.size() * std::max(interpolate, decimate)`
std::vector<std::pair<double, double>> resample(std::vector<std::pair<double, double>> out, std::size_t interpolate, std::size_t decimate) {
  if (interpolate == decimate || (interpolate <= 1 && decimate <= 1)) {return out;}

  auto const d = std::gcd(interpolate, decimate);
  interpolate /= d;
  decimate /= d;

  std::vector<std::pair<double, double>> vec;
  vec.reserve(out.size() * std::max(interpolate, decimate));

  if (interpolate > 1) {
    auto const n = interpolate;

    // for (std::size_t i = 0; i < out.size(); ++i) {
    //   auto const val = out[i];
    //   for (std::size_t j = 0; j < n; ++j) {
    //     vec.emplace_back(val);
    //   }
    // }

    for (std::size_t i = 0; i + 1 < out.size(); ++i) {
      auto const val = out[i];
      auto const next = out[i + 1];
      auto const f = (val.first - next.first) / static_cast<double>(n);
      auto const g = (val.second - next.second) / static_cast<double>(n);
      for (std::size_t j = 0; j < n; ++j) {
        vec.emplace_back(val.first - (f * static_cast<double>(j)), val.second - (g * static_cast<double>(j)));
      }
    }
    auto const last = vec.back();
    for (std::size_t j = 0; j < n; ++j) {
      vec.emplace_back(last);
    }

    out = vec;
    vec.clear();
  }

  if (decimate > 1) {
    auto const n = decimate;
    std::vector<std::pair<double, double>> tmp;
    tmp.reserve(n);
    double prev = 0;
    for (std::size_t i = 0; i + n - 1 < out.size(); i += n) {
      tmp.assign(out.begin() + i, out.begin() + i + n);
      std::sort(tmp.begin(), tmp.end(), [](auto const& lhs, auto const& rhs) {return lhs.first > rhs.first;});
      tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());
      std::size_t k = 0;
      for (; k < tmp.size(); ++k) {
        if (tmp[k].second > prev) {
          break;
        }
      }
      if (k == tmp.size()) {--k;}
      vec.emplace_back(tmp[k]);
      prev = tmp[k].second;
    }
    out = vec;
  }

  return out;
}

double center_frequency(double const low, double const high) {
  if (high / low < 1.1) {
    return (low + high) / 2.0;
  }
  return std::sqrt(low * high);
}

double q_factor_band_pass(double const low, double const high) {
  return center_frequency(low, high) / (high - low);
}

double q_factor_notch(double const low, double const high) {
  return (high - low) / center_frequency(low, high);
}

} // namespace Filter
