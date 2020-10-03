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

#include "app/app.hh"
#include "app/filter.hh"
#include "app/util.hh"
#include "ob/string.hh"
#include "ob/term.hh"
#include "ob/prism.hh"

#include <unistd.h>

#include <cmath>
#include <cstdlib>

#include <array>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <utility>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

template<typename T>
T mean(std::vector<T> const& v, std::size_t const size) {
  return std::accumulate(v.begin(), v.begin() + size, static_cast<T>(0)) / size;
}

template<typename T>
T stdev(std::vector<T> const& v, std::size_t const size, T const mean) {
  std::vector<T> dif (size);
  std::transform(v.begin(), v.begin() + size, dif.begin(), [mean](auto const n) {return n - mean;});
  return std::sqrt(std::inner_product(dif.begin(), dif.end(), dif.begin(), static_cast<T>(0)) / size);
}

template<typename T>
static std::enable_if_t<!std::numeric_limits<T>::is_integer, bool>
almost_equal(T x, T y, int ulp = 2) {
  return std::abs(x - y) <= std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp || std::abs(x - y) < std::numeric_limits<T>::min();
}

template<typename T>
T clamp(T const val, T const min, T const max) {
  assert(!(max < min));
  if (val < min) {return min;}
  if (val > max) {return max;}
  return val;
}

template<typename T>
T clampc(T const val, T const min, T const max) {
  assert(!(max < min));
  if (min == max) {return min;}
  if (val < min) {
    if ((min - val) > ((max - min) + 1)) {
      return max - (((min - val) % ((max - min + 1))) - 1);
    }
    else {
      return max - ((min - val) - 1);
    }
  }
  if (val > max) {
    if ((val - max) > ((max - min) + 1)) {
      return min + (((val - max) % ((max - min + 1))) - 1);
    }
    else {
      return min + ((val - max) - 1);
    }
  }
  return val;
}

template<typename T>
T clampcf(T const val, T const min, T const max) {
  assert(!(max < min));
  if (almost_equal(min, max)) {return min;}
  if (val < min) {
    if ((min - val) > ((max - min) + 1)) {
      return max - (std::fmod((min - val), ((max - min + 1))) - 1);
    }
    else {
      return max - ((min - val) - 1);
    }
  }
  if (val > max) {
    if ((val - max) > ((max - min) + 1)) {
      return min + (std::fmod((val - max), ((max - min + 1))) - 1);
    }
    else {
      return min + ((val - max) - 1);
    }
  }
  return val;
}

template<typename T, typename F>
constexpr T lerp(T const a, T const b, F const t) {
  return (a + (t * (b - a)));
}

template<typename T>
OB::Prism::HSLA lerp(OB::Prism::HSLA a, OB::Prism::HSLA b, T t) {
  OB::Prism::HSLA c;

  // clockwise distance between a and b
  // auto d = b.h() - a.h();
  // if (d == 0) {
  //   c = a;
  // }
  // else {
  //   c.h(clampc(lerp(0, clampc(b.h() - a.h(), 0, 359), t) + a.h(), 0, 359));
  // }

  // shortest distance between a and b
  auto d = b.h() - a.h();
  if (std::abs(d) > 180) {
    if (d < 0) {
      d += 360;
    }
    else {
      d -= 360;
    }
  }
  c.h(static_cast<int>((a.h() + (t * d)) + 360) % 360);

  if (a.h() > b.h()) {
    t = 1.0 - t;
    std::swap(a, b);
  }

  c.s(lerp(a.s(), b.s(), t));
  c.l(lerp(a.l(), b.l(), t));
  c.a(lerp(a.a(), b.a(), t));

  return c;
}

template<typename T>
T scale(T const val, T const in_min, T const in_max, T const out_min, T const out_max) {
  assert(in_min <= in_max);
  assert(out_min <= out_max);
  return (out_min + (out_max - out_min) * ((val - in_min) / (in_max - in_min)));
}

template<typename T>
T scale_log(T const val, T const in_min, T const in_max, T const out_min, T const out_max) {
  assert(in_min <= in_max);
  assert(out_min <= out_max);
  auto const b = std::log(out_max / out_min) / (in_max - in_min);
  auto const a = out_max / std::exp(b * in_max);
  return a * std::exp(b * val);
}

class Note {
public:
  Note(double const freq, std::size_t const scale = 12, double const a4 = 440.0) {
    assert(freq > 1);
    assert(scale == 12 || scale == 24);
    _scale = scale;
    int const tones {static_cast<int>(std::round(std::log(freq / a4) / std::log(std::pow(2.0, 1.0 / static_cast<double>(_scale)))) + (57 * (_scale / 12)))};
    _tone = static_cast<std::size_t>(tones % _scale);
    _octave = static_cast<std::size_t>(tones / _scale);
  }

  friend bool operator==(Note const& lhs, Note const& rhs);
  friend bool operator!=(Note const& lhs, Note const& rhs);

  std::string str() {
    if (_scale == 12) {
      return _semi_tones.at(_tone) + std::to_string(_octave);
    }
    return _quarter_tones.at(_tone) + std::to_string(_octave);
  }

  std::size_t scale() {
    return _scale;
  }

  std::size_t tone() {
    return _tone;
  }

  std::size_t octave() {
    return _octave;
  }

private:
  std::size_t _scale {0};
  std::size_t _tone {0};
  std::size_t _octave {0};

  inline static std::unordered_map<std::size_t, std::string> const _semi_tones {
    { 0, "C"}, { 1, "C#"},
    { 2, "D"}, { 3, "D#"},
    { 4, "E"},
    { 5, "F"}, { 6, "F#"},
    { 7, "G"}, { 8, "G#"},
    { 9, "A"}, {10, "A#"},
    {11, "B"},
  };

  inline static std::unordered_map<std::size_t, std::string> const _quarter_tones {
    { 0, "C  "}, { 1, "C+ "}, { 2, "C #"}, { 3, "C#+"},
    { 4, "D  "}, { 5, "D+ "}, { 6, "D #"}, { 7, "D#+"},
    { 8, "E  "}, { 9, "E+ "},
    {10, "F  "}, {11, "F+ "}, {12, "F #"}, {13, "F#+"},
    {14, "G  "}, {15, "G+ "}, {16, "G #"}, {17, "G#+"},
    {18, "A  "}, {19, "A+ "}, {20, "A #"}, {21, "A#+"},
    {22, "B  "}, {23, "B+ "},
  };
};

bool operator==(Note const& lhs, Note const& rhs) {
  return (lhs._scale == rhs._scale) && (lhs._tone == rhs._tone) && (lhs._octave == rhs._octave);
}

bool operator!=(Note const& lhs, Note const& rhs) {
  return !(lhs == rhs);
}

App::App(OB::Parg const& pg) : _pg {pg} {
  // prevent SFML from writing to std::cerr
  sf::err().rdbuf(nullptr);
}

App::~App() {
}

void App::screen_init() {
  std::cout
  << aec::cursor_hide
  << aec::screen_push
  << aec::cursor_hide
  << aec::screen_clear
  << aec::cursor_home
  << std::flush;
  if (!_raw_output) {
    _term_mode->set_raw();
  }
}

void App::screen_deinit() {
  std::cout
  << aec::nl
  << aec::screen_pop
  << aec::cursor_show
  << std::flush;
  if (!_raw_output) {
    _term_mode->set_cooked();
  }
}

void App::await_tick() {
  _tick_timer.stop();
  _timer.expires_at(_tick_timer.end() + (_tick - (_tick_timer.time<Tick>() % _tick)));

  _timer.async_wait([&](auto ec) {
    if (ec) {return;}
    _tick_end = Clock::now();
    _tick_timer.clear().start(_tick_end);
    auto delta = std::chrono::duration_cast<Tick>(_tick_end - _tick_begin);
    _time += delta;
    _tick_begin = _tick_end;

    if (delta.count() > 0) {
      _fps_actual = static_cast<int>(std::round(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(delta).count()));
    }

    if (delta > _tick) {
      int const dropped {static_cast<int>((delta.count() / _tick.count())) - 1};
      _fps_dropped += dropped;
    }

    on_tick(std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() / 1000.0);
    await_tick();
  });
}

void App::on_tick(double const dt) {
  update(dt);
  draw();
  render();
}

void App::await_signal() {
  _sig.on_signal({SIGINT, SIGTERM}, [&](auto const& ec, auto sig) {
    // std::cerr << "\nEvent: " << Belle::Signal::str(sig) << "\n";
    _io.stop();
  });

  _sig.on_signal(SIGWINCH, [&](auto const& ec, auto sig) {
    // std::cerr << "\nEvent: " << Belle::Signal::str(sig) << "\n";
    _sig.wait();
    on_winch();
  });

  _sig.on_signal(SIGTSTP, [&](auto const& ec, auto sig) {
    // std::cerr << "\nEvent: " << Belle::Signal::str(sig) << "\n";
    _sig.wait();
    _rec.stop();
    _timer.cancel();
    screen_deinit();
    kill(getpid(), SIGSTOP);
  });

  _sig.on_signal(SIGCONT, [&](auto const& ec, auto sig) {
    // std::cerr << "\nEvent: " << Belle::Signal::str(sig) << "\n";
    _sig.wait();
    screen_init();
    on_winch();
    _tick_timer.clear();
    _tick_begin = Clock::now();
    await_tick();
    _rec.start();
  });

  _sig.wait();
}

void App::on_winch() {
  if (!_fixed_size) {
    OB::Term::size(_width, _height);
  }

  _win.size = {_width, _height};
  _win.winch();
}

void App::await_read() {
  _read.on_read([&](auto const& ctx) {
    auto nctx = ctx;
    std::visit([&](auto& e) {on_read(e);}, nctx);
  });

  _read.run();
}

bool App::on_read(Read::Null& ctx) {
  return false;
}

bool App::on_read(Read::Mouse& ctx) {
  ctx.pos.x -= 1;
  ctx.pos.y = _height - ctx.pos.y;
  return false;
}

bool App::on_read(Read::Key& ctx) {
  // TODO add keybind to lock input
  // add a timed prompt info string so that a message describing how to exit locked input mode can be shown on keypress when locked
  // if (ctx.ch == OB::Term::ctrl_key(' ')) {
  //   _lock_input = !_lock_input;
  // }

  // if (_lock_input) {
  //   return true;
  // }

  {
    _code.erase(_code.begin());
    _code.emplace_back(std::make_pair(ctx.ch, _time));
    if (_code.back().second - _code.front().second < 3000ms) {
      if (
          _code[0].first == Key::Up &&
          _code[1].first == Key::Up &&
          _code[2].first == Key::Down &&
          _code[3].first == Key::Down &&
          _code[4].first == Key::Left &&
          _code[5].first == Key::Right &&
          _code[6].first == Key::Left &&
          _code[7].first == Key::Right &&
          _code[8].first == 'b' &&
          _code[9].first == 'a'
          ) {
        _draw_peak = true;
        _peak_reverse = true;
      }
      else if (
          _code[0].first == Key::Up &&
          _code[1].first == Key::Down &&
          _code[2].first == Key::Left &&
          _code[3].first == Key::Right &&
          _code[4].first == Key::Down &&
          _code[5].first == Key::Up &&
          _code[6].first == Key::Right &&
          _code[7].first == Key::Left &&
          _code[8].first == '0' &&
          _code[9].first == '8'
          ) {
        {
          _fps = 60;
          _interval = (1000.0 / _fps) * 0.2;
          _mono = false;
          _size = 2048;
          _sample_rate = 16000;
          _low_pass = 2200;
          _high_pass = 20;
          _threshold_min = -54.0;
          _threshold_max = -24.0;
          _sort_log = false;
          _octave_scale = 24;
          _filter = Filter_Type::sg;
          _filter_threshold = 0.10;
          _filter_size = 3;

          _overlay = false;

          _bar_swap = false;
          _block_flip = false;
          _block_stack = true;
          _block_reverse = false;
          _block_vertical = true;

          _block_width_dynamic = false;
          _block_height_dynamic = true;
          _block_height_full = true;
          _block_height_linear = true;
          _block_width = 1.0;
          _block_height = 0.25;
          _block_padding = 0;

          _draw_freq = true;
          _draw_freq_always = false;
          _speed_freq_unique = true;
          _speed_freq_up = 0.999999;
          _speed_freq_down = 0.999999;

          _draw_peak = false;
          _draw_peak_always = false;
          _speed_peak_unique = false;
          _speed_peak_down = 0.20;
          _peak_reverse = false;

          _color = true;
          _alpha = false;
          _alpha_blend = 0.3;
          _cl_shift = 0.0;
          _cl_swap = false;
          _cl_gradient_x = true;
          _cl_gradient_y = true;
          _cfg.style.bg = OB::Prism::Hex("1b1e24");
          _cfg.style.freq = OB::Prism::Hex("4feae7");
          _cfg.style.freq2 = OB::Prism::Hex("f34b7d");
          _cfg.style.freq3 = _cfg.style.freq2;
          _cfg.style.freq4 = _cfg.style.freq;
        }

        {
          _style_base = Style{Style::Bit_24, Style::Null, _cfg.style.bg, _cfg.style.bg};
          _win.style_base = _style_base;
          _win.refresh();

          _tick = static_cast<Tick>(1000000000 / _fps);
          _interval = (1000.0 / _fps) * 0.2;
          _rec.process_interval(sf::milliseconds(_interval));

          _rec.low_pass(_low_pass);
          _rec.high_pass(_high_pass);

          auto const recording = _rec.recording();
          if (recording) {
            _rec.stop();
          }
          // _channel = clampc(_channel + 1, static_cast<int>(Channel_Type::mono_mixed), static_cast<int>(Channel_Type::size - 1));
          if (_mono) {
            _rec.channels(1);
          }
          else {
            _rec.channels(2);
          }
          if (recording) {
            _rec.start();
          }
        }
      }
    }
  }

  switch (ctx.ch) {
    case OB::Term::ctrl_key('c'): {
      kill(getpid(), SIGINT);
      return true;
    }

    case OB::Term::ctrl_key('z'): {
      kill(getpid(), SIGTSTP);
      return true;
    }

    case OB::Term::ctrl_key('l'): {
      kill(getpid(), SIGWINCH);
      return true;
    }

    case '?': {
      auto const recording = _rec.recording();
      if (recording) {
        _rec.stop();
      }
      _timer.cancel();
      screen_deinit();

      std::system(("$(which less) -Ri '+/Key Bindings' <<'EOF'\n" + _pg.help() + "EOF").c_str());

      screen_init();
      on_winch();
      _tick_timer.clear();
      _tick_begin = Clock::now();
      await_tick();
      if (recording) {
        _rec.start();
      }

      return true;
    }

    case Key::Backspace: {
      {
        _fps = 30;
        _interval = (1000.0 / _fps) * 0.2;
        _mono = false;
        _size = 2048;
        _sample_rate = 16000;
        _low_pass = 4000;
        _high_pass = 40;
        _threshold_min = -60.0;
        _threshold_max = -20.0;
        _sort_log = true;
        _octave_scale = 24;
        _filter = Filter_Type::none;
        _filter_threshold = 0.10;
        _filter_size = 3;

        _overlay = false;

        _bar_swap = false;
        _block_flip = false;
        _block_stack = true;
        _block_reverse = false;
        _block_vertical = true;

        _block_width_dynamic = false;
        _block_height_dynamic = true;
        _block_height_full = true;
        _block_height_linear = true;
        _block_width = 1.0;
        _block_height = 0.25;
        _block_padding = 1;

        _draw_freq = true;
        _draw_freq_always = true;
        _speed_freq_unique = true;
        _speed_freq_up = 0.999999;
        _speed_freq_down = 0.98;

        _draw_peak = true;
        _draw_peak_always = false;
        _speed_peak_unique = false;
        _speed_peak_down = 0.20;
        _peak_reverse = false;

        _color = false;
        _alpha = false;
        _alpha_blend = 0.3;
        _cl_shift = 0.0;
        _cl_swap = false;
        _cl_gradient_x = true;
        _cl_gradient_y = true;
        _cfg.style.bg = OB::Prism::Hex("1b1e24");
        _cfg.style.freq = OB::Prism::Hex("4feae7");
        _cfg.style.freq2 = OB::Prism::Hex("f34b7d");
        _cfg.style.freq3 = _cfg.style.freq2;
        _cfg.style.freq4 = _cfg.style.freq;
      }

      {
        _style_base = Style{Style::Bit_24, Style::Null, _cfg.style.bg, _cfg.style.bg};
        _win.style_base = _style_base;
        _win.refresh();

        _tick = static_cast<Tick>(1000000000 / _fps);
        _interval = (1000.0 / _fps) * 0.2;
        _rec.process_interval(sf::milliseconds(_interval));

        _rec.low_pass(_low_pass);
        _rec.high_pass(_high_pass);

        auto const recording = _rec.recording();
        if (recording) {
          _rec.stop();
        }
        // _channel = clampc(_channel + 1, static_cast<int>(Channel_Type::mono_mixed), static_cast<int>(Channel_Type::size - 1));
        if (_mono) {
          _rec.channels(1);
        }
        else {
          _rec.channels(2);
        }
        if (recording) {
          _rec.start();
        }
      }
      return true;
    }

    case 'q': {
      _io.stop();
      return true;
    }

    case 'w': {
      _block_width = clampc(static_cast<int>(_block_width) + 1, 1, 32);
      return true;
    }

    case 'W': {
      _block_width = clampc(static_cast<int>(_block_width) - 1, 1, 32);
      return true;
    }

    case 'e': {
      _block_padding = clampc(static_cast<int>(_block_padding) + 1, 0, 8);
      return true;
    }

    case 'E': {
      _block_padding = clampc(static_cast<int>(_block_padding) - 1, 0, 8);
      return true;
    }

    case 'r': {
      _block_reverse = ! _block_reverse;
      return true;
    }

    case 't': {
      _bar_swap = ! _bar_swap;
      return true;
    }

    case 'y': {
      _sort_log = ! _sort_log;
      return true;
    }

    case 'i': {
      _fps = clamp(_fps + 2, 10, 60);
      _tick = static_cast<Tick>(1000000000 / _fps);
      _interval = (1000.0 / _fps) * 0.2;
      _rec.process_interval(sf::milliseconds(_interval));
      return true;
    }

    case 'I': {
      _fps = clamp(_fps - 2, 10, 60);
      _tick = static_cast<Tick>(1000000000 / _fps);
      _interval = (1000.0 / _fps) * 0.2;
      _rec.process_interval(sf::milliseconds(_interval));
      return true;
    }

    case 'o': {
      _overlay = ! _overlay;
      return true;
    }

    case 'p': {
      if (_rec.recording()) {
        _rec.stop();
      }
      else {
        _rec.start();
      }
      return true;
    }

    case 'a': {
      _block_vertical = ! _block_vertical;
      return true;
    }

    case 's': {
      auto const recording = _rec.recording();
      if (recording) {
        _rec.stop();
      }
      _mono = ! _mono;
      // _channel = clampc(_channel + 1, static_cast<int>(Channel_Type::mono_mixed), static_cast<int>(Channel_Type::size - 1));
      if (_mono) {
        _rec.channels(1);
      }
      else {
        _rec.channels(2);
      }
      if (recording) {
        _rec.start();
      }
      return true;
    }

    case 'd': {
      _block_stack = ! _block_stack;
      return true;
    }

    case 'f': {
      _block_flip = ! _block_flip;
      return true;
    }

    case 'g': {
      _filter = clampc(_filter + 1, static_cast<int>(Filter_Type::none), static_cast<int>(Filter_Type::size - 1));
      return true;
    }

    case 'G': {
      _filter = clampc(_filter - 1, static_cast<int>(Filter_Type::none), static_cast<int>(Filter_Type::size - 1));
      return true;
    }

    case 'h': {
      _high_pass = static_cast<std::size_t>(clamp(static_cast<int>(_high_pass) + 20, 20, static_cast<int>(_sample_rate) / 2));
      if (static_cast<int>(_high_pass) > static_cast<int>(_low_pass)) {_low_pass = _high_pass;}
      _rec.low_pass(_low_pass);
      _rec.high_pass(_high_pass);
      return true;
    }

    case 'H': {
      _high_pass = static_cast<std::size_t>(clamp(static_cast<int>(_high_pass) - 20, 20, static_cast<int>(_sample_rate) / 2));
      if (static_cast<int>(_high_pass) > static_cast<int>(_low_pass)) {_low_pass = _high_pass;}
      _rec.low_pass(_low_pass);
      _rec.high_pass(_high_pass);
      return true;
    }

    case 'j': {
      _threshold_min = clamp(_threshold_min + 2.0, -120.0, 0.0);
      if (_threshold_min > _threshold_max) {_threshold_max = _threshold_min;}
      return true;
    }

    case 'J': {
      _threshold_min = clamp(_threshold_min - 2.0, -120.0, 0.0);
      return true;
    }

    case 'k': {
      _threshold_max = clamp(_threshold_max - 2.0, -120.0, 0.0);
      if (_threshold_max < _threshold_min) {_threshold_min = _threshold_max;}
      return true;
    }

    case 'K': {
      _threshold_max = clamp(_threshold_max + 2.0, -120.0, 0.0);
      return true;
    }

    case 'l': {
      _low_pass = static_cast<std::size_t>(clamp(static_cast<int>(_low_pass) - 20, 20, static_cast<int>(_sample_rate) / 2));
      if (static_cast<int>(_low_pass) < static_cast<int>(_high_pass)) {_high_pass = _low_pass;}
      _rec.low_pass(_low_pass);
      _rec.high_pass(_high_pass);
      return true;
    }

    case 'L': {
      _low_pass = static_cast<std::size_t>(clamp(static_cast<int>(_low_pass) + 20, 20, static_cast<int>(_sample_rate) / 2));
      if (static_cast<int>(_low_pass) < static_cast<int>(_high_pass)) {_high_pass = _low_pass;}
      _rec.low_pass(_low_pass);
      _rec.high_pass(_high_pass);
      return true;
    }

    case 'z': {
      _draw_freq = ! _draw_freq;
      return true;
    }

    case 'Z': {
      _draw_freq_always = ! _draw_freq_always;
      return true;
    }

    case 'x': {
      _draw_peak = ! _draw_peak;
      return true;
    }

    case 'X': {
      _draw_peak_always = ! _draw_peak_always;
      return true;
    }

    case 'c': {
      _color = ! _color;
      if (_color) {
        _style_base = Style{Style::Bit_24, Style::Null, _cfg.style.bg, _cfg.style.bg};
      }
      else {
        _style_base = Style{Style::Default, Style::Null, {}, {}};
      }
      _win.style_base = _style_base;
      _win.refresh();
      return true;
    }

    case 'C': {
      _alpha = ! _alpha;
      return true;
    }

    case 'v': {
      _cfg.style.freq = OB::Prism::HSLA{random_range(0, 359), static_cast<float>(random_range(40, 100)), static_cast<float>(random_range(20, 80)), 1.0};
      _cfg.style.freq2 = OB::Prism::HSLA{clampc(_cfg.style.freq.h() + random_range(6, 120), 0, 359), static_cast<float>(random_range(50, 100)), static_cast<float>(random_range(30, 70)), 1.0};
      // _cfg.style.freq2 = OB::Prism::HSLA{clampc(_cfg.style.freq.h() + random_range(30, 150), 0, 359), static_cast<float>(random_range(40, 100)), static_cast<float>(random_range(20, 80)), 1.0};

      if (_cfg.style.freq.l() > _cfg.style.freq2.l()) {
        std::swap(_cfg.style.freq, _cfg.style.freq2);
      }

      // TODO use 2 or 4 gradient colors
      // maybe use fixed sat and lum
      _cfg.style.freq3 = OB::Prism::HSLA{clampc(_cfg.style.freq2.h() + random_range(0, 30), 0, 359), static_cast<float>(clampcf(_cfg.style.freq2.s() + random_range(0, 20), 0.f, 100.f)), static_cast<float>(clampcf(_cfg.style.freq2.l() + random_range(0, 20), 0.f, 100.f)), 1.0};
      _cfg.style.freq4 = OB::Prism::HSLA{clampc(_cfg.style.freq.h() + random_range(0, 30), 0, 359), static_cast<float>(clampcf(_cfg.style.freq.s() + random_range(0, 20), 0.f, 100.f)), static_cast<float>(clampcf(_cfg.style.freq.l() + random_range(0, 20), 0.f, 100.f)), 1.0};

      // _cfg.style.freq3 = _cfg.style.freq2;
      // _cfg.style.freq4 = _cfg.style.freq;

      return true;
    }

    case 'V': {
      _cl_shift = _cl_shift > 0.0 ? 0.0 : 250.0;
      return true;
    }

    case 'b': {
      _block_height_full = ! _block_height_full;
      return true;
    }

    case 'n': {
      _cl_gradient_x = ! _cl_gradient_x;
      return true;
    }

    case 'N': {
      _cl_gradient_y = ! _cl_gradient_y;
      return true;
    }

    case 'm': {
      swap_colors();
      return true;
    }

    case 'M': {
      _cl_swap = ! _cl_swap;
      return true;
    }
  }

  return false;
}

void App::swap_colors() {
  std::swap(_cfg.style.freq, _cfg.style.freq2);
  std::swap(_cfg.style.freq3, _cfg.style.freq4);
}

void App::shift_colors(double const dt) {
  if (_color && _cl_shift > 0.0) {
    _cl_delta += dt * 1000.0;
    while (_cl_delta >= _cl_shift) {
      _cl_delta -= _cl_shift;
      _cfg.style.freq.h(clampc(static_cast<int>(_cfg.style.freq.h()) - 1, 0, 359));
      _cfg.style.freq2.h(clampc(static_cast<int>(_cfg.style.freq2.h()) - 1, 0, 359));
      _cfg.style.freq3.h(clampc(static_cast<int>(_cfg.style.freq3.h()) - 1, 0, 359));
      _cfg.style.freq4.h(clampc(static_cast<int>(_cfg.style.freq4.h()) - 1, 0, 359));
    }
  }
}

std::size_t App::bar_calc_height(double const val, std::size_t height) const {
  std::size_t res {0};
  auto const nheight = (height * 8.0) - 1.0;
  if (nheight < 0.0) {
    return 0;
  }
  if (_block_height_linear) {
    res = static_cast<std::size_t>(std::trunc(scale(val, _threshold_min, _threshold_max, 0.0, nheight)));
  }
  else {
    res = static_cast<std::size_t>((std::trunc(scale_log(val, _threshold_min, _threshold_max, 1.0, nheight + 0.001)) - 1.0) * 2.0);
  }
  if (res > nheight) {
    res = nheight;
  }
  return res;
}

void App::bar_calc_dimensions(Bars& bars) {
  // calc bar width
  // either a fixed size or dynamic based on output width
  if (_block_width_dynamic) {
    bars.bar_width = std::max(1ul, static_cast<std::size_t>(std::trunc(bars.width * _block_width))); // _block_width 0.0<->1.0
  }
  else {
    bars.bar_width = _block_width;
    if (!_block_vertical) {
      bars.bar_width = std::max(1ul, bars.bar_width / 2ul);
    }
    if (bars.bar_width >= bars.width) {bars.bar_width = bars.width;}
  }

  // calc bar height
  // either a fixed size or dynamic based on output height
  if (_block_height_dynamic) {
    bars.bar_height = std::max(1ul, static_cast<std::size_t>(std::trunc(bars.height * _block_height)));
  }
  else {
    bars.bar_height = _block_height;
    if (!_block_vertical) {
      bars.bar_height = std::max(1ul, bars.bar_height * 2ul);
    }
    if (bars.bar_height >= bars.height) {bars.bar_height = bars.height;}
  }

  // calc number of bars to output
  if (bars.width == 0) {
    bars.size = 1;
  }
  else {
    bars.size = std::max(1ul, (bars.width - (bars.margin_lhs + bars.margin_rhs)) / (bars.bar_width + bars.padding) + ((bars.width - (bars.margin_lhs + bars.margin_rhs)) % (bars.bar_width + bars.padding) ? 1ul : 0ul));
  }
}

void App::bar_process(std::vector<double> const& bins, Bars& bars) {
  auto const bin_freq_res = _rec.sample_rate() / static_cast<double>(_size);
  auto const low = _rec.high_pass() + std::fmod(_rec.high_pass(), bin_freq_res);
  _info.resize(bars.size);

  if (bins.size()) {
    if (_sort_log) {
      std::size_t T {0};
      for (std::size_t x = 0, p = 0, i = 0; x < bars.size; ++x) {
        i = static_cast<std::size_t>(std::trunc(scale_log(static_cast<double>(x + 1), 1.0, static_cast<double>(bars.size), 1.0, static_cast<double>(bins.size())) + 0.001));
        if (p >= i) {i = p + 1;}
        if (i >= bins.size()) {i = bins.size() - 1;}
        auto const begin = bins.begin() + p;
        auto const end = bins.begin() + i;

        // value is based on average of elements in the range
        // double v = std::accumulate(begin, end, 0.0) / (i - p);
        // bars.raw[x] = v;

        // value is based on max element in the range
        auto const ptr = std::max_element(begin, end);

        bars.raw[x] = *ptr;

        _info[x] = Info{bin_freq_res * static_cast<double>(std::distance(bins.begin(), ptr)) + static_cast<std::size_t>(std::trunc(low))};
        T += i - p;
        p = i;
      }
    }
    else {
      std::size_t T {0};
      std::vector<std::pair<double, double>> raw;
      for (std::size_t x = 0, p = 0, i = 0; p + 1 < bins.size(); ++x) {
        Note const note {bin_freq_res * p + low, _octave_scale};
        for (++i; i < bins.size(); ++i) {
          if (note != Note(bin_freq_res * i + low, _octave_scale)) {
            break;
          }
        }
        if (p >= i) {i = p + 1;}
        if (i >= bins.size()) {i = bins.size() - 1;}
        auto const begin = bins.begin() + p;
        auto const end = bins.begin() + i;

        // value is based on average of elements in the range
        // double v = std::accumulate(begin, end, 0.0) / (i - p);
        // value is based on max element in the range
        auto const ptr = std::max_element(begin, end);

        raw.emplace_back(*ptr, bin_freq_res * static_cast<double>(std::distance(bins.begin(), ptr)) + static_cast<std::size_t>(std::trunc(low)));

        T += i - p;
        p = i;
      }

      // TODO make each octave as equally represented as possible
      // depending on frequency resolution, low octaves may have considerably less notes than the higher octaves
      // split bars by octave
      // resample each octave to either 12 or 24 notes
      // resample all bars to final size
      auto const db_hz = Filter::resample(raw, bars.size, raw.size());
      for (std::size_t i = 0; i < bars.size; ++i) {
        auto const& e = db_hz[i];
        bars.raw[i] = e.first;
        _info[i] = Info{e.second};
      }
    }

    for (auto& v : bars.raw) {
      if ((!std::isfinite(v) && std::signbit(v)) || v < _threshold_min) {
        v = _threshold_min;
      }
      else if ((!std::isfinite(v) && !std::signbit(v)) || v > _threshold_max) {
        v = _threshold_max;
      }
    }

    switch (_filter) {
      case Filter_Type::sg: {
        Filter::savitzky_golay(bars.raw, bars.size, _filter_size, 0.0);
        break;
      }
      case Filter_Type::sgm: {
        Filter::savitzky_golay(bars.raw, bars.size, _filter_size, std::abs(_threshold_max - _threshold_min) * _filter_threshold);
        break;
      }
      default: {
        break;
      }
    }
  }
  else {
    bars.raw.assign(bars.size, _threshold_min);
  }
}

void App::bar_movement(double const dt, Bars& bars) {
  for (std::size_t i = 0; i < bars.size; ++i) {
    if (bars.freq[i] < bars.raw[i]) {
      if (_speed_freq_unique) {
        bars.freq[i] = lerp(bars.freq[i], bars.raw[i], 1.0 - std::pow(1.0 - _speed_freq_up, dt));
      }
      else {
        bars.freq[i] += std::min((std::abs(_threshold_max - _threshold_min) * _speed_freq_up) * dt, std::abs(bars.raw[i] - bars.freq[i]));
      }
    }
    else if (bars.freq[i] > bars.raw[i]) {
      if (_speed_freq_unique) {
        bars.freq[i] = lerp(bars.freq[i], bars.raw[i], 1.0 - std::pow(1.0 - _speed_freq_down, dt));
      }
      else {
        bars.freq[i] -= std::min((std::abs(_threshold_max - _threshold_min) * _speed_freq_down) * dt, std::abs(bars.freq[i] - bars.raw[i]));
      }
    }

    if (bars.freq[i] - 0.1 < _threshold_min) {
      bars.freq[i] = _threshold_min;
    }
    else if (bars.freq[i] > _threshold_max) {
      bars.freq[i] = _threshold_max;
    }

    if (bars.peak[i] > bars.freq[i]) {
      if (_speed_peak_unique) {
        if (_peak_reverse) {
          bars.peak[i] = lerp(bars.peak[i], _threshold_max, 1.0 - std::pow(1.0 - _speed_peak_down, dt));
        }
        else {
          bars.peak[i] = lerp(bars.peak[i], bars.freq[i], 1.0 - std::pow(1.0 - _speed_peak_down, dt));
        }
      }
      else {
        if (_peak_reverse) {
          bars.peak[i] += std::abs(_threshold_max - _threshold_min) * _speed_peak_down * dt;
        }
        else {
          bars.peak[i] -= std::min((std::abs(_threshold_max - _threshold_min) * _speed_peak_down) * dt, std::abs(bars.peak[i] - bars.freq[i]));
        }
      }
    }
    else {
      bars.peak[i] = bars.freq[i];
    }

    if (_peak_reverse) {
      if (bars.peak[i] + 0.1 > _threshold_max) {
        bars.peak[i] = _threshold_min;
      }
    }

    if (bars.peak[i] - 0.1 < _threshold_min) {
      bars.peak[i] = _threshold_min;
    }
    else if (bars.peak[i] > _threshold_max) {
      bars.peak[i] = _threshold_max;
    }
  }
}

void App::update(double const dt) {
  update_visualizer(dt);
}

void App::update_visualizer(double const dt) {
  _rec.process();

  _bars_left.margin_lhs = 0;
  _bars_right.margin_lhs = 0;
  _bars_left.margin_rhs = 0;
  _bars_right.margin_rhs = 0;

  if (_block_padding > 1 && !_block_vertical) {
    _bars_left.padding = _block_padding / 2ul;
    _bars_right.padding = _block_padding / 2ul;
  }
  else {
    _bars_left.padding = _block_padding;
    _bars_right.padding = _block_padding;
  }

  if (_mono) {
    _bars_left.x = 0;
    _bars_left.y = 0;
    _bars_left.width = _width;
    _bars_left.height = _height;
    if (!_block_vertical) {std::swap(_bars_left.width, _bars_left.height);}

    bar_calc_dimensions(_bars_left);

    bar_process(_rec.buffer_left(), _bars_left);
    bar_movement(dt, _bars_left);
  }
  else {
    std::size_t width {_width};
    std::size_t height {_height};
    if (!_block_vertical) {std::swap(width, height);}

    if (_block_stack) {
      auto const height_half = height / 2;
      auto const height_rem = height % 2;

      _bars_left.width = width;
      _bars_left.height = height_half + height_rem;

      _bars_right.width = width;
      _bars_right.height = height_half;

      if (_block_vertical) {
        _bars_left.x = 0;
        _bars_left.y = height_half;

        _bars_right.x = 0;
        _bars_right.y = 0;
      }
      else {
        _bars_left.x = 0;
        _bars_left.y = 0;

        _bars_right.x = 0;
        _bars_right.y = height_half + height_rem;
      }

      bar_calc_dimensions(_bars_left);
      bar_calc_dimensions(_bars_right);
    }
    else {
      // stereo shared
      auto const width_half = width / 2;
      auto const width_rem = width % 2;
      auto const padding_half = _bars_left.padding / 2;
      auto const padding_rem = _bars_left.padding % 2;

      _bars_left.width = width_half + width_rem;
      _bars_left.height = height;
      _bars_left.margin_lhs = padding_half + padding_rem;

      _bars_right.width = width_half;
      _bars_right.height = height;
      _bars_right.margin_lhs = padding_half;

      if (_block_vertical) {
        _bars_left.x = 0;
        _bars_left.y = 0;

        _bars_right.x = width_half + width_rem;
        _bars_right.y = 0;
      }
      else {
        _bars_left.x = width_half + width_rem;
        _bars_left.y = 0;

        _bars_right.x = 0;
        _bars_right.y = 0;

        std::swap(_bars_left.width, _bars_right.width);
        std::swap(_bars_left.margin_lhs, _bars_right.margin_lhs);
      }

      bar_calc_dimensions(_bars_left);
      bar_calc_dimensions(_bars_right);
    }

    bar_process(_rec.buffer_left(), _bars_left);
    bar_process(_rec.buffer_right(), _bars_right);
    if (_bar_swap) {std::swap(_bars_left.raw, _bars_right.raw);}
    bar_movement(dt, _bars_left);
    bar_movement(dt, _bars_right);
  }

  shift_colors(dt);
}

void App::draw() {
  draw_visualizer();
  draw_overlay();
}

void App::draw_overlay() {
  if (_overlay) {
    if ((_mono || _block_stack) && _block_vertical && !_block_reverse) {
      std::size_t index {0};
      Pos pos {0, _height - 1};
      for (auto const& info : _info) {
        std::string const info_str {std::to_string(static_cast<std::size_t>(std::trunc(info.freq))) + " " + Note{info.freq, _octave_scale}.str()};
        for (auto const& e : info_str) {
          if (_color) {
            _win.buf(pos, Cell{1, Style{Style::Bit_24, 0, index % 2 ? OB::Prism::Hex("c0c0c0") : OB::Prism::Hex("f0f0f0"), _cfg.style.bg}, std::string(1, e)});
          }
          else {
            _win.buf(pos, Cell{1, _style_default, std::string(1, e)});
          }
          if (pos.y == 0) {break;}
          --pos.y;
        }
        pos.x += _bars_left.bar_width + _block_padding;
        pos.y = _height - 1;
        ++index;
      }
    }

    {
      std::string title {
        "frequency "s + std::to_string(_high_pass) + ":"s + std::to_string(_low_pass)
        + " | decibels "s + std::to_string(static_cast<int>(_threshold_min)) + ":"s + std::to_string(static_cast<int>(_threshold_max))
        + " | filter "s + std::to_string(_filter)
        + " | sort "s + (_sort_log ? "log"s : "note"s)
        + " | fps "s + std::to_string(_fps)
      };
      if (title.size() > _width) {
        title = title.substr(0, _width - 1);
        title += ">";
      }
      auto style = _color ? Style{Style::Bit_24, 0, OB::Prism::Hex("f0f0f0"), _cfg.style.bg} : Style{Style::Default, 0, {}, {}};
      _win.buf.put(Pos{(_width / 2) - (title.size() / 2), 0}, Cell{1, style, title});
    }
  }
}

void App::draw_visualizer() {
  if (_mono) {
    if (_cl_swap) {swap_colors();}
    draw_visualizer_impl(_bars_left.x, _bars_left.y, _bars_left.width, _bars_left.height, _bars_left);
    if (_cl_swap) {swap_colors();}
  }
  else {
    if (_block_vertical) {
      if (_block_stack) {
        if (_cl_swap) {swap_colors();}
        draw_visualizer_impl(_bars_left.x, _bars_left.y, _bars_left.width, _bars_left.height, _bars_left);
        if (_cl_swap) {swap_colors();}
        _block_flip = !_block_flip;
        draw_visualizer_impl(_bars_right.x, _bars_right.y, _bars_right.width, _bars_right.height, _bars_right);
        _block_flip = !_block_flip;
      }
      else {
        if (_cl_swap) {swap_colors();}
        draw_visualizer_impl(_bars_left.x, _bars_left.y, _bars_left.width, _bars_left.height, _bars_left, true);
        if (_cl_swap) {swap_colors();}
        draw_visualizer_impl(_bars_right.x, _bars_right.y, _bars_right.width, _bars_right.height, _bars_right);
      }
    }
    else {
      if (_block_stack) {
          _block_flip = !_block_flip;
          if (_cl_swap) {swap_colors();}
          draw_visualizer_impl(_bars_left.x, _bars_left.y, _bars_left.width, _bars_left.height, _bars_left);
          if (_cl_swap) {swap_colors();}
          _block_flip = !_block_flip;
          draw_visualizer_impl(_bars_right.x, _bars_right.y, _bars_right.width, _bars_right.height, _bars_right);
      }
      else {
        if (_cl_swap) {swap_colors();}
          draw_visualizer_impl(_bars_left.x, _bars_left.y, _bars_left.width, _bars_left.height, _bars_left);
          if (_cl_swap) {swap_colors();}
          draw_visualizer_impl(_bars_right.x, _bars_right.y, _bars_right.width, _bars_right.height, _bars_right, true);
      }
    }
  }
}

void App::draw_visualizer_impl(std::size_t x_begin, std::size_t y_begin, std::size_t width, std::size_t height, Bars& bars, bool const draw_reverse) {
  if (!((_draw_freq || (_draw_freq && _draw_freq_always)) || (_draw_peak || (_draw_peak && _draw_peak_always)))) {
    return;
  }

  // select vertical or horizontal character set
  auto const& bar_char {_block_vertical ? _bar_vertical : _bar_horizontal};

  // TODO improve alpha blending
  // TODO fix corner color issue
  // TODO use x color gradient along each block width
  // TODO add stacked view but for mono channel
  // TODO add single bar per channel mode
  for (std::size_t x = 0; x < bars.size; ++x) {
    auto const index = _block_reverse ? bars.size - 1 - x : x;
    auto c = bars.freq[index];
    auto p = bars.peak[index];

    // calculate x position and bar width
    // bar_width will be less than bars.bar_width if it partially overflows at the end or at the beginning if draw_reverse is true
    int x_pos {0};
    std::size_t bar_width {0};
    if (draw_reverse) {
      x_pos = width - bars.bar_width - bars.margin_lhs - ((x * bars.bar_width) + (x * bars.padding));
      bar_width = x_pos + bars.bar_width > width ? width - x_pos : bars.bar_width;
      if (x_pos < 0) {
        bar_width += x_pos;
        x_pos = 0;
      }
    }
    else {
      x_pos = bars.margin_lhs + (x * bars.bar_width) + (x * bars.padding);
      bar_width = x_pos + bars.bar_width > width ? width - x_pos : bars.bar_width;
    }

    // draw bar
    if (bar_width && ((_draw_freq && _draw_freq_always) || (_draw_freq && c > _threshold_min))) {
      auto const vheight = bar_calc_height(c, height);
      auto const bar_height = vheight / 8;
      std::size_t const tip_index = (_block_flip ? 7 - (vheight % 8 ? vheight % 8 : 1) : vheight % 8);
      bool y_bottom_drawn {false};

      OB::Prism::HSLA init_color {_cfg.style.freq};
      OB::Prism::HSLA init_color2 {_cfg.style.freq3};
      if (_color) {
        if (_cl_gradient_x && bars.size > 1 && _cfg.style.freq != _cfg.style.freq2) {
          init_color = lerp(_cfg.style.freq, _cfg.style.freq2, static_cast<double>(x) / static_cast<double>(bars.size - 1));

          if (_cl_gradient_y && _cfg.style.freq3 != _cfg.style.freq4) {
            init_color2 = lerp(_cfg.style.freq3, _cfg.style.freq4, static_cast<double>(x) / static_cast<double>(bars.size - 1));
          }
        }
      }
      auto color = init_color;

      // draw bar except for the top
      for (std::size_t y = 0; y < bar_height; ++y) {
        std::size_t const y_pos {_block_flip ? ((height - 1) - y) : y};

        if (_color) {
          if (_cl_gradient_y && height > 1) {
            color = lerp(init_color, init_color2, static_cast<double>(y) / static_cast<double>(height - 1));
          }

          if (_alpha) {
            auto const d = (static_cast<double>(y) + _alpha_blend) / static_cast<double>(height);
            color.a(d >= _alpha_blend ? 255 : clamp(static_cast<int>(std::round(((d / _alpha_blend) * 255.0) + 16.0)), 16, 255));
          }
        }

        if (_block_height_full) {
          // draw full
          if (_color) {
            for (std::size_t i = 0; i < bar_width; ++i) {
              auto xp = x_begin + x_pos + i;
              auto yp = y_begin + y_pos;
              if (!_block_vertical) {std::swap(xp, yp);}
              _win.buf(Pos{xp, yp}, Cell{1, Style{Style::Bit_24, 0, color, _cfg.style.bg}, bar_char[7]});
            }
          }
          else {
            for (std::size_t i = 0; i < bar_width; ++i) {
              auto xp = x_begin + x_pos + i;
              auto yp = y_begin + y_pos;
              if (!_block_vertical) {std::swap(xp, yp);}
              _win.buf(Pos{xp, yp}, Cell{1, _style_default, bar_char[7]});
            }
          }
        }
        else if (bar_height <= bars.bar_height || y >= bar_height - bars.bar_height) {
          // draw fixed height
          if (!y_bottom_drawn) {
            y_bottom_drawn = true;

            if (y == 0 && bars.bar_height > bar_height) {
              // draw regular
              if (_color) {
                for (std::size_t i = 0; i < bar_width; ++i) {
                  auto xp = x_begin + x_pos + i;
                  auto yp = y_begin + y_pos;
                  if (!_block_vertical) {std::swap(xp, yp);}
                  _win.buf(Pos{xp, yp}, Cell{1, Style{Style::Bit_24, 0, color, _cfg.style.bg}, bar_char[7]});
                }
              }
              else {
                for (std::size_t i = 0; i < bar_width; ++i) {
                  auto xp = x_begin + x_pos + i;
                  auto yp = y_begin + y_pos;
                  if (!_block_vertical) {std::swap(xp, yp);}
                  _win.buf(Pos{xp, yp}, Cell{1, _style_default, bar_char[7]});
                }
              }
            }
            else {
              // draw partial bottom
              if (_color) {
                OB::Prism::RGBA fg;
                OB::Prism::RGBA bg;

                if (_block_flip) {
                  fg = color;
                  bg = _cfg.style.bg;
                }
                else {
                  fg = _cfg.style.bg;
                  bg = color;
                }

                for (std::size_t i = 0; i < bar_width; ++i) {
                  auto xp = x_begin + x_pos + i;
                  auto yp = y_begin + y_pos;
                  if (!_block_vertical) {std::swap(xp, yp);}
                  _win.buf(Pos{xp, yp}, Cell{1, Style{Style::Bit_24, 0, fg, bg}, bar_char[tip_index]});
                }
              }
              else {
                auto const style_attr = _block_flip ? Style::Null : Style::Reverse;
                auto style = _style_default;
                style.attr = style_attr;

                for (std::size_t i = 0; i < bar_width; ++i) {
                  auto xp = x_begin + x_pos + i;
                  auto yp = y_begin + y_pos;
                  if (!_block_vertical) {std::swap(xp, yp);}
                  _win.buf(Pos{xp, yp}, Cell{1, style, bar_char[tip_index]});
                }
              }
            }
          }
          else {
            // draw regular
            if (_color) {
              for (std::size_t i = 0; i < bar_width; ++i) {
                auto xp = x_begin + x_pos + i;
                auto yp = y_begin + y_pos;
                if (!_block_vertical) {std::swap(xp, yp);}
                _win.buf(Pos{xp, yp}, Cell{1, Style{Style::Bit_24, 0, color, _cfg.style.bg}, bar_char[7]});
              }
            }
            else {
              for (std::size_t i = 0; i < bar_width; ++i) {
                auto xp = x_begin + x_pos + i;
                auto yp = y_begin + y_pos;
                if (!_block_vertical) {std::swap(xp, yp);}
                _win.buf(Pos{xp, yp}, Cell{1, _style_default, bar_char[7]});
              }
            }
          }
        }
      }

      // draw partial top
      {
        if (_color) {
          if (_cl_gradient_y) {
            color = lerp(init_color, init_color2, static_cast<double>(vheight) / static_cast<double>(height * 8));
          }

          if (_alpha) {
            auto const d = (static_cast<double>(bar_height) + _alpha_blend) / static_cast<double>(height);
            color.a(d >= _alpha_blend ? 255 : clamp(static_cast<int>(std::round(((d / _alpha_blend) * 255.0) + 16.0)), 16, 255));
          }
        }

        std::size_t const y_pos {_block_flip ? (height - 1) - bar_height : bar_height};


        if (_color) {
          OB::Prism::RGBA fg;
          OB::Prism::RGBA bg;

          if (_block_flip) {
            fg = _cfg.style.bg;
            bg = color;
          }
          else {
            fg = color;
            bg = _cfg.style.bg;
          }

          for (std::size_t i = 0; i < bar_width; ++i) {
            auto xp = x_begin + x_pos + i;
            auto yp = y_begin + y_pos;
            if (!_block_vertical) {std::swap(xp, yp);}
            _win.buf(Pos{xp, yp}, Cell{1, Style{Style::Bit_24, 0, fg, bg}, bar_char[tip_index]});
          }
        }
        else {
          auto const style_attr = _block_flip ? Style::Reverse : Style::Null;
          auto style = _style_default;
          style.attr = style_attr;

          for (std::size_t i = 0; i < bar_width; ++i) {
            auto xp = x_begin + x_pos + i;
            auto yp = y_begin + y_pos;
            if (!_block_vertical) {std::swap(xp, yp);}
            _win.buf(Pos{xp, yp}, Cell{1, style, bar_char[tip_index]});
          }
        }
      }
    }

    // draw peak
    if (bar_width && ((_draw_peak && _draw_peak_always) || (_draw_peak && p > _threshold_min))) {
      if (p >= c) {
        auto vheight = bar_calc_height(p, height);
        auto bar_height = vheight / 8;
        std::size_t const tip_index = (_block_flip ? 6 : 0);
        std::size_t const y_pos {_block_flip ? (height - 1) - bar_height : bar_height};
        if (y_begin + y_pos >= (_block_vertical ?_win.buf.size().y : _win.buf.size().x)) {continue;}
        auto xp = x_begin + x_pos;
        auto yp = y_begin + y_pos;
        if (!_block_vertical) {std::swap(xp, yp);}
        auto const& cell = _win.buf.col(Pos{xp, yp});

        // draw only if peak does not overlap bar
        if (cell.text.empty() || cell.text == " ") {
          if (_color) {
            OB::Prism::HSLA init_color {_cfg.style.freq3};
            OB::Prism::HSLA init_color2 {_cfg.style.freq};
            if (_cl_gradient_x && bars.size > 1 && _cfg.style.freq != _cfg.style.freq2) {
              init_color = lerp(_cfg.style.freq2, _cfg.style.freq, static_cast<double>(x) / static_cast<double>(bars.size - 1));

              if (_cl_gradient_y && _cfg.style.freq3 != _cfg.style.freq4) {
                init_color2 = lerp(_cfg.style.freq4, _cfg.style.freq3, static_cast<double>(x) / static_cast<double>(bars.size - 1));
              }
            }
            OB::Prism::HSLA color {init_color};

            if (_cl_gradient_y && height > 1) {
              color = lerp(init_color, init_color2, static_cast<double>(bar_height) / static_cast<double>(height - 1));
            }

            if (_alpha) {
              auto const d = (static_cast<double>(bar_height) + _alpha_blend) / static_cast<double>(height);
              color.a(d >= _alpha_blend ? 255 : clamp(static_cast<int>(std::round(((d / _alpha_blend) * 255.0) + 16.0)), 16, 255));
            }

            OB::Prism::RGBA fg;
            OB::Prism::RGBA bg;

            if (_block_flip) {
              fg = _cfg.style.bg;
              bg = color;
            }
            else {
              fg = color;
              bg = _cfg.style.bg;
            }

            for (std::size_t i = 0; i < bar_width; ++i) {
              auto xp = x_begin + x_pos + i;
              auto yp = y_begin + y_pos;
              if (!_block_vertical) {std::swap(xp, yp);}
              _win.buf(Pos{xp, yp}, Cell{1, Style{Style::Bit_24, 0, fg, bg}, bar_char[tip_index]});
            }
          }
          else {
            auto const style_attr = _block_flip ? Style::Reverse : Style::Null;
            auto style = _style_default;
            style.attr = style_attr;

            for (std::size_t i = 0; i < bar_width; ++i) {
              auto xp = x_begin + x_pos + i;
              auto yp = y_begin + y_pos;
              if (!_block_vertical) {std::swap(xp, yp);}
              _win.buf(Pos{xp, yp}, Cell{1, style, bar_char[tip_index]});
            }
          }
        }
      }
    }
  }
}

void App::render() {
  if (_raw_output) {
    // _raw_file.open(_raw_filename, std::ios::out | std::ios::trunc);
    // if (!_raw_file.is_open()) {throw std::runtime_error("open failed");}
    _win.render_file(_raw_file);
    // _raw_file.close();
  }
  else {
    _win.render();
  }
}

void App::run() {
  await_signal();

  if (!_raw_output) {
    auto const is_term = Term::is_term(STDOUT_FILENO);
    if (!is_term) {throw std::runtime_error("stdout is not a tty");}
  }

  if (! _rec.available()) {
    throw std::runtime_error("recording device is unavailable");
  }

  if (_mono) {
    _rec.channels(1);
  }
  else {
    _rec.channels(2);
  }
  _rec.low_pass(_low_pass);
  _rec.high_pass(_high_pass);
  _rec.sample_rate(_sample_rate);
  _rec.device(_rec.device_default());
  // for (auto const& device : _rec.devices()) {
  //   if (OB::String::assert_rx(device, std::regex(".*monitor.*", std::regex::icase))) {
  //     _rec.device(device);
  //     break;
  //   }
  // }
  _rec.process_interval(sf::milliseconds(_interval));

  auto const buf_size = _size / 2;
  _bars_left.raw.assign(buf_size, _threshold_min);
  _bars_left.freq.assign(buf_size, _threshold_min);
  _bars_left.peak.assign(buf_size, _threshold_min);
  _bars_right.raw.assign(buf_size, _threshold_min);
  _bars_right.freq.assign(buf_size, _threshold_min);
  _bars_right.peak.assign(buf_size, _threshold_min);

  if (_color) {
    _style_base = Style{Style::Bit_24, Style::Null, _cfg.style.bg, _cfg.style.bg};
  }
  else {
    _style_base = Style{Style::Default, Style::Null, {}, {}};
  }
  _win.style_base = _style_base;

  if (!_raw_output) {
    _term_mode = std::make_unique<OB::Term::Mode>();
  }
  screen_init();
  on_winch();
  if (!_raw_output) {
    await_read();
  }
  _tick_begin = Clock::now();
  await_tick();
  _rec.start();
  _io.run();
  _rec.stop();
  screen_deinit();
}
