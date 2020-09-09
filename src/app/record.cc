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

#include "app/record.hh"
#include "app/util.hh"

#include <cassert>

Record::Record(std::size_t size) : _size {size} {
  _left.samples.resize(_size, 0);
  _left.fmtbuf.resize(_size, -120);

  _right.samples.resize(_size, 0);
  _right.fmtbuf.resize(_size, -120);

  _inbuf.resize(_size);
  _outbuf.resize(_size);

  _hann.reserve(_size);
  for (std::size_t i = 0; i < _size; ++i) {
    _hann.emplace_back(_hann_constant * (1.0 - std::cos(2.0 * M_PI * i / static_cast<value_type>(_size))));
  }

  filter_init();
}

Record::~Record() {
  stop();
}

void Record::process_interval(sf::Time interval) {
  sf::SoundRecorder::setProcessingInterval(interval);
}

unsigned int Record::sample_rate() const {
  return sf::SoundRecorder::getSampleRate();
}

void Record::sample_rate(unsigned int rate) {
  _sample_rate = rate;
  assert(_sample_rate != 0);
  assert(_sample_rate >= _low_pass * 2);
  filter_init();
}

std::string const& Record::device() const {
  return sf::SoundRecorder::getDevice();
}

bool Record::device(std::string const& name) {
  return sf::SoundRecorder::setDevice(name);
}

unsigned int Record::channels() const {
  return sf::SoundRecorder::getChannelCount();
}

void Record::channels(unsigned int count) {
  sf::SoundRecorder::setChannelCount(count);
}

std::vector<std::string> Record::devices() {
  return sf::SoundRecorder::getAvailableDevices();
}

std::string Record::device_default() {
  return sf::SoundRecorder::getDefaultDevice();
}

bool Record::available() {
  return sf::SoundRecorder::isAvailable();
}

bool Record::start() {
  return sf::SoundRecorder::start(_sample_rate);
}

void Record::stop() {
  sf::SoundRecorder::stop();
}

bool Record::recording() const {
  return _recording.load();
}

Record::Bands const& Record::bands_left() const {
  return _left.bands;
}

Record::Bands const& Record::bands_right() const {
  return _right.bands;
}

std::vector<double> const& Record::buffer_left() const {
  return _left.fmtbuf;
}

std::vector<double> const& Record::buffer_right() const {
  return _right.fmtbuf;
}

std::vector<double> Record::samples_left() {
  std::scoped_lock lock {_mutex};
  return _left.samples;
}

std::vector<double> Record::samples_right() {
  std::scoped_lock lock {_mutex};
  return _right.samples;
}

std::size_t Record::size() const {
  return _inbuf.size();
}

std::size_t Record::low_pass() const {
  return _low_pass;
}

void Record::low_pass(std::size_t const hz) {
  _low_pass = hz;
  assert(_low_pass > _high_pass);
  assert(_low_pass <= _sample_rate / 2);
  filter_init();
}

std::size_t Record::high_pass() const {
  return _high_pass;
}

void Record::high_pass(std::size_t const hz) {
  _high_pass = hz;
  assert(_high_pass < _low_pass);
  filter_init();
}

void Record::process() {
  // check if audio samples are silent
  // clear the buffers if they are silent
  static bool cleared {false};
  if (_silence.load()) {
    if (!cleared) {
      _left.fmtbuf.assign(_left.fmtbuf.size(), -120);
      _right.fmtbuf.assign(_right.fmtbuf.size(), -120);
      cleared = true;
    }
    return;
  }
  cleared = false;

  if (!_update.load()) {return;}
  _update.store(false);

  if (sf::SoundRecorder::getChannelCount() == 1) {
    process_left();
  }
  else {
    process_left();
    process_right();
  }
}

void Record::process_left() {
  process_impl(_left);
}

void Record::process_right() {
  process_impl(_right);
}

void Record::process_impl(Channel& channel) {
  {
    // apply window function
    // add samples to fft in buffer
    // set imaginary part of complex number to zero
    std::scoped_lock lock {_mutex};
    for (std::size_t i = 0; i < _size; ++i) {
      _inbuf[i] = complex_type(channel.samples[i] * _hann[i], 0);
    }
  }

  _fft(_inbuf, _outbuf);

  // calculate magnitude in decibels of each output bin
  auto size = _outbuf.size() / 2;
  channel.fmtbuf.resize(size);
  double const bin {sf::SoundRecorder::getSampleRate() / static_cast<double>(_size)};
  for (std::size_t i = 0; i <= size; ++i) {
    channel.fmtbuf[i] = 20.0 * std::log10(std::sqrt(_outbuf[i].real() * _outbuf[i].real() + _outbuf[i].imag() * _outbuf[i].imag()) / ((_outbuf.size() / 2.0) / _hann_constant));
  }

  // calculate bands
  // {
  //   auto const calc_band = [&](auto& band) {
  //     band.val = 0;
  //     std::size_t const begin = static_cast<std::size_t>(band.min / bin);
  //     std::size_t const end = static_cast<std::size_t>((band.max / bin) + 1);
  //     for (std::size_t i = begin; i < end && i < channel.fmtbuf.size(); ++i) {
  //       band.val += channel.fmtbuf[i];
  //     }
  //     band.val = band.val / (end - begin);
  //   };

  //   calc_band(channel.bands.sub_bass);
  //   calc_band(channel.bands.bass);
  //   calc_band(channel.bands.low_midrange);
  //   calc_band(channel.bands.midrange);
  //   calc_band(channel.bands.upper_midrange);
  //   calc_band(channel.bands.presence);
  //   calc_band(channel.bands.brilliance);
  // }

  if (_trim_bins) {
    auto low_index = static_cast<long int>(_low_pass / bin);
    channel.fmtbuf.erase(channel.fmtbuf.begin() + low_index, channel.fmtbuf.end());

    auto high_index = static_cast<long int>(_high_pass / bin);
    channel.fmtbuf.erase(channel.fmtbuf.begin(), channel.fmtbuf.begin() + high_index);
  }
}

bool Record::onStart() {
  _recording.store(true);
  return true;
}

void Record::onStop() {
  _recording.store(false);
}

bool Record::onProcessSamples(sf::Int16 const* samples, std::size_t size) {
  if (!size) {return true;}

  if (*std::max_element(&samples[0], &samples[size]) == 0) {
    _silence.store(true);
    return true;
  }

  std::scoped_lock lock {_mutex};

  if (sf::SoundRecorder::getChannelCount() == 1) {
    // shift samples left by chunk size
    for (std::size_t i = size; i < _size; ++i) {
      _left.samples[i - size] = _left.samples[i];
    }

    // scale, filter and add new samples
    double scale_pos {1.0 / 32767.0};
    double scale_neg {1.0 / 32768.0};
    for (std::size_t i = 0, p = ((_size - size) - 1); i < size && p < _size; ++i, ++p) {
      _left.samples[p] = (samples[i] > 0 ? samples[i] * scale_pos : samples[i] * scale_neg);
      filter(_left, p);
    }
  }
  else {
    // shift samples left by chunk size
    auto const isize {size / 2};
    for (std::size_t i = isize; i < _size; ++i) {
      _left.samples[i - isize] = _left.samples[i];
      _right.samples[i - isize] = _right.samples[i];
    }

    // scale, filter and add new samples
    double scale_pos {1.0 / 32767.0};
    double scale_neg {1.0 / 32768.0};
    for (std::size_t i = 0, p = ((_size - isize) - 1); i < size && p < _size; i += 2, ++p) {
      _left.samples[p] = (samples[i] > 0 ? samples[i] * scale_pos : samples[i] * scale_neg);
      _right.samples[p] = (samples[i + 1] > 0 ? samples[i + 1] * scale_pos : samples[i + 1] * scale_neg);
      filter(_left, p);
      filter(_right, p);
    }
  }

  _silence.store(false);
  _update.store(true);

  return true;
}

void Record::filter_init() {
  double const q {1.0};
  double const s {1.0};
  double const db_gain {3.0};
  double const shelf_freq {1000.0};

  _left.high_shelf_filter.init(_sample_rate, shelf_freq, s, db_gain);
  _left.low_pass_filter.init(_sample_rate, _low_pass, q);
  _left.high_pass_filter.init(_sample_rate, _high_pass, q);

  _right.high_shelf_filter.init(_sample_rate, shelf_freq, s, db_gain);
  _right.low_pass_filter.init(_sample_rate, _low_pass, q);
  _right.high_pass_filter.init(_sample_rate, _high_pass, q);
}

void Record::filter(Channel& channel, std::size_t const pos) {
  auto& val = channel.samples[pos];
  val = channel.high_shelf_filter.process(val);
  val = channel.low_pass_filter.process(val);
  val = channel.high_pass_filter.process(val);
}
