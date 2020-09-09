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

#ifndef APP_RECORD_HH
#define APP_RECORD_HH

#include "ob/fft.hh"

#include "app/filter.hh"

#include <SFML/Audio.hpp>
#include <SFML/System.hpp>

#include <cmath>
#include <cstddef>

#include <mutex>
#include <atomic>
#include <vector>
#include <complex>

class Record : public sf::SoundRecorder {
public:
  using value_type = double;
  using FFT = OB::FFT<value_type>;
  using complex_type = std::complex<value_type>;

  struct Band {
    double min {0};
    double max {0};
    double val {0};
  };

  struct Bands {
    Band sub_bass {20, 60, 0};
    Band bass {60, 250, 0};
    Band low_midrange {250, 500, 0};
    Band midrange {500, 2000, 0};
    Band upper_midrange {2000, 4000, 0};
    Band presence {4000, 6000, 0};
    Band brilliance {6000, 20000, 0};
  };

  struct Channel {
    Bands bands;
    std::vector<value_type> samples;
    std::vector<value_type> fmtbuf;
    Filter::Low_Pass low_pass_filter;
    Filter::High_Pass high_pass_filter;
    Filter::High_Shelf high_shelf_filter;
  };

  Record(std::size_t size = 1024);
  ~Record();

  void process_interval(sf::Time interval);
  unsigned int sample_rate() const;
  void sample_rate(unsigned int rate);
  std::string const& device() const;
  bool device(std::string const& name);
  unsigned int channels() const;
  void channels(unsigned int count);
  static std::vector<std::string> devices();
  static std::string device_default();
  static bool available();
  bool start();
  void stop();

  bool recording() const;
  Bands const& bands_left() const;
  Bands const& bands_right() const;
  std::vector<double> const& buffer_left() const;
  std::vector<double> const& buffer_right() const;
  std::vector<double> samples_left();
  std::vector<double> samples_right();
  std::size_t size() const;
  std::size_t low_pass() const;
  void low_pass(std::size_t const hz);
  std::size_t high_pass() const;
  void high_pass(std::size_t const hz);
  void process();
  void process_left();
  void process_right();

private:
  void process_impl(Channel& channel);
  bool onStart() override;
  void onStop() override;
  bool onProcessSamples(sf::Int16 const* samples, std::size_t size) override;

  void filter_init();
  void filter(Channel& channel, std::size_t const pos);

  std::mutex _mutex;
  std::atomic<bool> _silence {true};
  std::atomic<bool> _update {false};
  std::atomic<bool> _recording {false};
  double const _hann_constant {0.5};
  std::size_t const _size {2048};
  std::size_t _sample_rate {48000};
  std::size_t _low_pass {20000};
  std::size_t _high_pass {20};
  bool _trim_bins {true};
  FFT _fft;
  Channel _left;
  Channel _right;
  std::vector<complex_type> _inbuf;
  std::vector<complex_type> _outbuf;
  std::vector<value_type> _hann;
};

#endif // APP_RECORD_HH
