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

namespace Filter
{

void savitzky_golay(std::vector<double>& bars, std::size_t size, std::size_t width, double const threshold) {
  if (size == 0 || width == 0) {return;}
  if (width >= size) {width = size - 1;}
  auto const part = static_cast<std::size_t>(std::trunc(width / 2.0));
  if (part == 0) {return;}

  OB::ring_vector<double> win {bars.begin(), bars.begin() + static_cast<long int>(width)};
  auto const c = 1.0 / (part * 2 + 1);
  auto const end = size - part;

  for (std::size_t i = part; i < end; ++i) {
    double res {0};

    for (std::size_t j = 0; j < width; ++j) {
      res += (win[j] * c) + j - part;
    }

    win.push(bars[i + part + 1]);

    if (threshold == 0.0 || std::abs(bars[i] - res) < threshold) {
      bars[i] = res;
    }
  }
}

template<typename T>
static constexpr T lerp(T const a, T const b, T const t) {
  return a + (t * (b - a));
}

std::vector<double> resample(std::vector<double> out, std::size_t interpolate, std::size_t decimate) {
  if (interpolate == decimate || (interpolate <= 1 && decimate <= 1)) {return out;}

  // Filter::Low_Pass filter_interpolate;
  // Filter::Low_Pass filter_decimate;
  // filter_interpolate.init(interpolate, interpolate / 2, 1.0);
  // filter_decimate.init(decimate, decimate / 2 , 1.0);

  auto const d = std::gcd(interpolate, decimate);
  interpolate /= d;
  decimate /= d;

  if (interpolate > 1) {
    std::vector<double> vec;
    auto const n = interpolate;
    for (std::size_t i = 0; i + 1 < out.size(); ++i) {
      auto const val = out[i];
      auto const next = out[i + 1];
      auto const f = (val - next) / n;
      for (std::size_t j = 0; j < n; ++j) {
        vec.emplace_back(val + (f * j));
      }
    }
    auto const last = vec.back();
    for (std::size_t j = 0; j < n; ++j) {
      vec.emplace_back(last);
    }
    out = vec;
  }

  // for (auto& e : out) {
  //   e = filter_decimate.process(filter_interpolate.process(e));
  // }

  if (decimate > 1) {
    std::vector<double> vec;
    auto const n = decimate;
    for (std::size_t i = 0; i + n - 1 < out.size(); i += n) {
      // double val = 0;
      // for (std::size_t j = 0; j < n; ++j) {
      //   val += out[i + j];
      // }
      // vec.emplace_back(val / n);

      // auto const val = out[i];
      // vec.emplace_back(val);

      // auto const val = out[i];
      // auto min = val;
      // auto max = val;
      // for (std::size_t j = 1; j < n; ++j) {
      //   min = std::min(min, out[i + j]);
      //   max = std::max(max, out[i + j]);
      // }
      // vec.emplace_back(lerp(min, max, 0.5));

      auto const val = out[i];
      auto max = val;
      for (std::size_t j = 1; j < n; ++j) {
        max = std::max(max, out[i + j]);
      }
      vec.emplace_back(max);
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
