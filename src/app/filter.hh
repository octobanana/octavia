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

#ifndef APP_FILTER_HH
#define APP_FILTER_HH

#include <cmath>
#include <cassert>
#include <cstddef>

#include <vector>
#include <numeric>

namespace Filter
{

double constexpr Pi {3.1415926535897932384626433832795028841971};

void savitzky_golay(std::vector<double>& bars, std::size_t size, std::size_t width = 3, double const threshold = 0.0);

std::vector<double> resample(std::vector<double> out, std::size_t interpolate, std::size_t decimate);

double center_frequency(double const low, double const high);
double q_factor_band_pass(double const low, double const high);
double q_factor_notch(double const low, double const high);

class Biquad {
public:
  virtual ~Biquad() = default;

  virtual void init(double const sample_rate, double const freq, double const q, double const db_gain) = 0;

  double process(double const x) {
    double const y = _b0 * x + _b1 * _x1 - _a1 * _y1 + _b2 * _x2 - _a2 * _y2;
    _x2 = _x1;
    _x1 = x;
    _y2 = _y1;
    _y1 = y;
    return y;
  }

  void clear() {
    _x1 = 0;
    _y1 = 0;
    _x2 = 0;
    _y2 = 0;
  }

protected:
  void normalize() {
    _a1 /= _a0;
    _a2 /= _a0;
    _b0 /= _a0;
    _b1 /= _a0;
    _b2 /= _a0;
  }

  double _a0 {0};
  double _a1 {0};
  double _a2 {0};
  double _b0 {0};
  double _b1 {0};
  double _b2 {0};

private:
  double _x1 {0};
  double _y1 {0};
  double _x2 {0};
  double _y2 {0};
};

class All_Pass : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const q, double const db_gain = 0) {
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin / (2.0 * q);
    double const wcos1 = 1.0 - wcos;

    _a0 = 1.0 + alpha;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha;
    _b0 = 1.0 - alpha;
    _b1 = -2.0 * wcos;
    _b2 = 1.0 + alpha;

    clear();
    normalize();
  }
};

class Low_Pass : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const q, double const db_gain = 0) {
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin / (2.0 * q);
    double const wcos1 = 1.0 - wcos;

    _a0 = 1.0 + alpha;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha;
    _b0 = wcos1 / 2.0;
    _b1 = wcos1;
    _b2 = wcos1 / 2.0;

    clear();
    normalize();
  }
};

class High_Pass : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const q, double const db_gain = 0) {
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin / (2.0 * q);
    double const wcos1 = 1.0 + wcos;

    _a0 = 1.0 + alpha;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha;
    _b0 = wcos1 / 2.0;
    _b1 = -wcos1;
    _b2 = wcos1 / 2.0;

    clear();
    normalize();
  }
};

class Band_Pass_1 : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const bw, double const db_gain = 0) {
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin * std::sinh(std::log(2.0) / 2.0 * bw * w / wsin);

    _a0 = 1.0 + alpha;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha;
    _b0 = bw * alpha;
    _b1 = 0;
    _b2 = -bw * alpha;

    clear();
    normalize();
  }
};

class Band_Pass_2 : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const bw, double const db_gain = 0) {
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin * std::sinh(std::log(2.0) / 2.0 * bw * w / wsin);

    _a0 = 1.0 + alpha;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha;
    _b0 = alpha;
    _b1 = 0;
    _b2 = -alpha;

    clear();
    normalize();
  }
};

class notch : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const bw, double const db_gain = 0) {
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin * std::sinh(std::log(2.0) / 2.0 * bw * w / wsin);

    _a0 = 1.0 + alpha;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha;
    _b0 = 1;
    _b1 = -2.0 * wcos;
    _b2 = 1;

    clear();
    normalize();
  }
};

class peak : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const bw, double const db_gain = 0) {
    double const a = std::pow(10.0, db_gain / 40.0);
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin * std::sinh(std::log(2.0) / 2.0 * bw * w / wsin);

    _a0 = 1.0 + alpha / a;
    _a1 = -2.0 * wcos;
    _a2 = 1.0 - alpha / a;
    _b0 = 1.0 + alpha * a;
    _b1 = -2.0 * wcos;
    _b2 = 1.0 - alpha * a;

    clear();
    normalize();
  }
};

class Low_Shelf : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const s, double const db_gain) {
    double const a = std::pow(10.0, db_gain / 40.0);
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin / 2.0 * std::sqrt((a + 1.0 / a) * (1.0 / s - 1.0) + 2.0);
    double const alpha2 = 2.0 * std::sqrt(a) * alpha;

    _a0 = (a + 1.0) + (a - 1.0) * wcos + alpha2;
    _a1 = -2.0 * ((a - 1.0) + (a + 1.0) * wcos);
    _a2 = (a + 1.0) + (a - 1.0) * wcos - alpha2;
    _b0 = a * ((a + 1.0) - (a - 1.0) * wcos + alpha2);
    _b1 = 2.0 * a * ((a - 1.0) - (a + 1.0) * wcos);
    _b2 = a * ((a + 1.0) - (a - 1.0) * wcos - alpha2);

    clear();
    normalize();
  }
};

class High_Shelf : public Biquad {
public:
  void init(double const sample_rate, double const freq, double const s, double const db_gain) {
    double const a = std::pow(10.0, db_gain / 40.0);
    double const w = 2.0 * Pi * (freq / sample_rate);
    double const wcos = std::cos(w);
    double const wsin = std::sin(w);
    double const alpha = wsin / 2.0 * std::sqrt((a + 1.0 / a) * (1.0 / s - 1.0) + 2.0);
    double const alpha2 = 2.0 * std::sqrt(a) * alpha;

    _a0 = (a + 1.0) - (a - 1.0) * wcos + alpha2;
    _a1 = 2.0 * ((a - 1.0) - (a + 1.0) * wcos);
    _a2 = (a + 1.0) - (a - 1.0) * wcos - alpha2;
    _b0 = a * ((a + 1.0) + (a - 1.0) * wcos + alpha2);
    _b1 = -2.0 * a * ((a - 1.0) + (a + 1.0) * wcos);
    _b2 = a * ((a + 1.0) + (a - 1.0) * wcos - alpha2);

    clear();
    normalize();
  }
};

} // namespace Filter

#endif // APP_FILTER_HH
