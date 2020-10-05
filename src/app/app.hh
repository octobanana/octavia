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

#ifndef APP_HH
#define APP_HH

#include "app/util.hh"
#include "app/window.hh"
#include "app/record.hh"

#include "ob/parg.hh"
#include "ob/text.hh"
#include "ob/term.hh"
#include "ob/timer.hh"
#include "ob/prism.hh"
#include "ob/belle/belle.hh"

#include <boost/asio.hpp>

#include <cmath>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <array>
#include <mutex>
#include <tuple>
#include <deque>
#include <regex>
#include <atomic>
#include <chrono>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <iomanip>
#include <complex>
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <unordered_map>

using Read = OB::Belle::IO::Read;
using Key = OB::Belle::IO::Read::Key;
using Mouse = OB::Belle::IO::Read::Mouse;
using Tick = std::chrono::nanoseconds;
using Clock = std::chrono::steady_clock;
using Timer = OB::Belle::asio::steady_timer;
using namespace std::chrono_literals;
using namespace std::string_literals;
namespace Text = OB::Text;
namespace Term = OB::Term;
namespace Belle = OB::Belle;
namespace iom = OB::Term::iomanip;
namespace aec = OB::Term::ANSI_Escape_Codes;
namespace fs = std::filesystem;
namespace asio = boost::asio;

class App {
public:
  App(OB::Parg const& pg);
  ~App();

  void run();

private:
  struct Bars {
    // number of bars to output
    std::size_t size {0};
    // space between each bar
    std::size_t padding {0};
    // space before bars begin
    std::size_t margin_lhs {0};
    // space after bars end
    std::size_t margin_rhs {0};
    // x position
    std::size_t x {0};
    // y position
    std::size_t y {0};
    // width of output
    std::size_t width {0};
    // height of output
    std::size_t height {0};
    // width of each bar
    std::size_t bar_width {0};
    // height of each bar
    std::size_t bar_height {0};
    // raw target values
    std::vector<double> raw;
    // frequency values
    std::vector<double> freq;
    // peak values
    std::vector<double> peak;
  };

  OB::Parg const& _pg;

  void screen_init();
  void screen_deinit();
  void await_signal();
  void on_tick(double const dt);
  void await_tick();
  void on_winch();
  void await_read();
  bool on_read(Read::Null& ctx);
  bool on_read(Read::Mouse& ctx);
  bool on_read(Read::Key& ctx);

  void swap_colors();
  void shift_colors(double const dt);

  std::size_t bar_calc_height(double const val, std::size_t height) const;

  void bar_calc_dimensions(Bars& bars);
  void bar_process(std::vector<double> const& bins, Bars& bars);
  void bar_movement(double const dt, Bars& bars);

  void update(double const dt);
  void update_visualizer(double const dt);

  void draw();
  void draw_overlay();
  void draw_visualizer();
  void draw_visualizer_impl(std::size_t x_begin, std::size_t y_begin, std::size_t width, std::size_t height, Bars& bars, bool const draw_reverse = false);

  void render();

  // TODO
  // struct Channel_Type {
  //   enum {
  //     mono_mixed = 0,
  //     mono_left,
  //     mono_right,
  //     stereo,
  //     stereo_left,
  //     stereo_right,
  //     size
  //   };
  // };

  struct Filter_Type {
    enum {
      none = 0,
      sgm,
      sg,
      size
    };
  };

  struct Config {
    int fps                   {30};
    // int channel            {Channel_Type::stereo};
    bool sort_log             {true};
    std::size_t octave_scale  {24};
    int filter                {Filter_Type::none};
    double filter_threshold   {0.10};
    std::size_t filter_size   {3};
    bool mono                 {false};
    bool overlay              {false};
    bool color                {false};
    bool alpha                {false};
    double alpha_blend        {0.3};
    double cl_shift           {0.0};
    bool cl_gradient_x        {true};
    bool cl_gradient_y        {true};
    bool cl_swap              {false};
    bool bar_swap             {false};
    bool block_flip           {false};
    bool block_stack          {true};
    bool block_reverse        {false};
    bool block_vertical       {true};
    bool peak_reverse         {false};
    bool draw_freq            {true};
    bool draw_peak            {true};
    bool draw_freq_always     {true};
    bool draw_peak_always     {false};
    bool block_width_dynamic  {false};
    bool block_height_dynamic {true};
    bool block_height_full    {true};
    bool block_height_linear  {true};
    double block_width        {1.0};
    double block_height       {0.25};
    std::size_t block_padding {1};
    bool speed_freq_unique    {true};
    bool speed_peak_unique    {false};
    double speed_freq_up      {0.999999};
    double speed_freq_down    {0.99};
    double speed_peak_down    {0.20};
    double threshold_min      {-60.0};
    double threshold_max      {-20.0};
    double interval           {(1000.0 / fps) * 0.2};
    std::size_t size          {2048};
    std::size_t sample_rate   {16000};
    std::size_t low_pass      {4000};
    std::size_t high_pass     {40};

    struct Style {
      // NOTE for using 4 color gradients
      // when the colors are converted to HSLA, all hues must be within 180 degrees of each other, else the gradient will become distorted

      // background
      OB::Prism::RGBA bg      {OB::Prism::Hex("1b1e24")};
      // bottom left
      OB::Prism::HSLA freq    {OB::Prism::Hex("4feae7")};
      // bottom right
      OB::Prism::HSLA freq2   {OB::Prism::Hex("f34b7d")};
      // top left
      OB::Prism::HSLA freq3   {freq2};
      // top right
      OB::Prism::HSLA freq4   {freq};
    } style;
  } _cfg;

  Style _style_base {Style::Bit_24, Style::Null, _cfg.style.bg, _cfg.style.bg};
  Style _style_default {Style::Default, Style::Null, {}, {}};

  double _cl_delta {0.0};

  std::size_t _overlay_index {0};

  std::array<std::string, 8> _bar_vertical {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
  std::array<std::string, 8> _bar_horizontal {"▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};

  bool _raw_output {false};
  std::string _raw_filename;
  std::ofstream _raw_file;

  bool _fixed_size {false};
  std::size_t _width {80};
  std::size_t _height {1};

  asio::io_context _io {1};
  Belle::Signal _sig {_io};
  Read _read {_io};

  Tick _time {0ms};
  OB::Timer<Clock> _tick_timer;
  std::chrono::time_point<Clock> _tick_begin {(Clock::time_point::min)()};
  std::chrono::time_point<Clock> _tick_end {(Clock::time_point::min)()};
  int _fps_actual {0};
  int _fps_dropped {0};
  Tick _tick {static_cast<Tick>(1000000000 / _cfg.fps)};
  Timer _timer {_io};

  Record _rec {_cfg.size};

  std::unique_ptr<OB::Term::Mode> _term_mode;
  Window _win;

  Bars _bars_left;
  Bars _bars_right;

  // bool _lock_input {false};
  std::vector<std::pair<char32_t, Tick>> _code {{0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}, {0, 0ms}};

  struct Info {
    double freq {0};
  };
  using Bar_Info = std::vector<Info>;
  Bar_Info _info;
};

#endif // APP_HH
