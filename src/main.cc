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

#include "info.hh"
#include "ob/parg.hh"
#include "ob/term.hh"
#include "app/app.hh"

#include <cstddef>
#include <cstdlib>

#include <string>
#include <sstream>
#include <iostream>

using Parg = OB::Parg;
namespace Term = OB::Term;
namespace iom = OB::Term::iomanip;
namespace aec = OB::Term::ANSI_Escape_Codes;

int main(int argc, char** argv) {
  std::ios_base::sync_with_stdio(false);

  Parg pg {argc, argv};
  auto const pg_status {program_info(pg)};
  if (pg_status > 0) return 0;
  if (pg_status < 0) return 1;

  auto const is_term = Term::is_term(STDOUT_FILENO);

  auto const color = pg.get<std::string>("colour") == "auto" ?
    Term::is_term(STDOUT_FILENO) : pg.get<std::string>("colour") == "on";

  try {
    App app {pg};
    app.run();

    // TODO new api idea
    {
      // asio::io_context io {1};

      // Cfg cfg;
      // cfg.style.fg = Prism::Hex("f0f0f0");
      // cfg.style.bg = Prism::Hex("1b1e24");

      // App app {io, cfg};

      // Tui tui {io};
      // tui.fps(30);
      // tui.win.size = {0, 0};
      // tui.win.style_base = Style{Style::Default, Style::Null, {}, {}};

      // tui.on_error([&](auto const& ec) {
      //   // std::cerr << "Error: " << ec << "\n";
      //   io.stop();
      //   app.stop();
      //   tui.stop();
      // });

      // tui.on_signal({SIGINT, SIGTERM}, [&](int sig) {
      //   // std::cerr << "Signal: " << Belle::Signal::str(sig) << "\n";
      //   io.stop();
      //   app.stop();
      //   tui.stop();
      // });

      // tui.on_pause([&]() {
      //   app.stop();
      //   tui.stop();
      // });

      // tui.on_continue([&]() {
      //   app.run();
      //   tui.run();
      // });

      // tui.on_resize([&](Size const& size) {
      //   tui.win.size = size;
      // });

      // tui.on_mouse([&](Read::Mouse const& ctx) {
      // });

      // tui.on_key([&](Read::Key const& ctx) {
      // });

      // tui.on_tick([&](double dt) {
      //   app.update(dt);
      //   app.draw(tui.)
      // });

      // app.run();
      // tui.run();
      // io.run();
    }
  }
  catch(std::exception const& e) {
    std::cerr
    << "\n"
    << aec::wrap("Error: ", pg.style.error, color)
    << e.what()
    << "\n";

    return 1;
  }
  catch(...) {
    std::cerr
    << "\n"
    << aec::wrap("Error: ", pg.style.error, color)
    << "an unexpected error occurred"
    << "\n";

    return 1;
  }

  return 0;
}
