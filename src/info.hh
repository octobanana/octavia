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

#ifndef INFO_HH
#define INFO_HH

#include "ob/parg.hh"
#include "ob/term.hh"

#include <cstddef>

#include <string>
#include <string_view>
#include <iostream>

inline int program_info(OB::Parg& pg);
inline bool program_color(std::string_view color);
inline void program_init(OB::Parg& pg);

inline void program_init(OB::Parg& pg) {
  pg.name("octavia").version("0.1.2 (04.10.2020)");
  pg.description("octobanana's customizable text-based audio visualization interactive application.");

  pg.usage("");
  pg.usage("[--colour=<on|off|auto>] -h|--help");
  pg.usage("[--colour=<on|off|auto>] -v|--version");
  pg.usage("[--colour=<on|off|auto>] --license");

  pg.info({"Key Bindings", {
    {"<ctrl-c>", "quit the program"},
    {"<ctrl-z>", "suspend the program"},
    {"<ctrl-l>", "force screen redraw"},
    {"<backspace>", "reset settings to default state"},
    {"?", "show key bindings"},
    {"q", "quit the program"},
    {"w", "increase width of bars/peaks"},
    {"W", "decrease width of bars/peaks"},
    {"e", "increase space between bars/peaks"},
    {"E", "decrease space between bars/peaks"},
    {"r", "toggle low-high/high-low order of bars/peaks"},
    {"t", "toggle left/right channel order"},
    {"y", "toggle log/note frequency sorting"},
    {"i", "increase fps"},
    {"I", "decrease fps"},
    {"o", "toggle enable/disable debug overlay"},
    {"p", "toggle start/stop audio capture"},
    {"a", "toggle vertical/horizontal layout"},
    {"s", "toggle mono/stereo audio capture"},
    {"d", "toggle shared/stacked stereo layout"},
    {"f", "toggle flipped layout"},
    {"g", "next filter type none/sgm/sg"},
    {"G", "prev filter type none/sgm/sg"},
    {"h", "increase high pass hz threshold"},
    {"H", "decrease high pass hz threshold"},
    {"j", "increase minimum db threshold"},
    {"J", "decrease minimum db threshold"},
    {"k", "decrease maximum db threshold"},
    {"K", "increase maximum db threshold"},
    {"l", "decrease low pass hz threshold"},
    {"L", "increase low pass hz threshold"},
    {"z", "toggle draw bars"},
    {"Z", "toggle always draw bars"},
    {"x", "toggle draw peaks"},
    {"X", "toggle always draw peaks"},
    {"c", "toggle enable/disable colour"},
    {"C", "toggle enable/disable alpha compositing"},
    {"v", "randomize colour"},
    {"V", "toggle enable/disable colour shift"},
    {"b", "toggle full/fixed bar height"},
    {"n", "toggle enable/disable x-axis colour gradient"},
    {"N", "toggle enable/disable y-axis colour gradient"},
    {"m", "swap colour order of both channels"},
    {"M", "swap colour order of left channel"},
    {"??????????", "secret 1"},
    {"??????????", "secret 2"},
  }});

  pg.info({"Examples", {
    {"octavia",
      "run the program"},
    {"octavia --help --colour=off",
      "print the help output, without colour"},
    {"octavia --help",
      "print the help output"},
    {"octavia --version",
      "print the program version"},
    {"octavia --license",
      "print the program license"},
  }});

  pg.info({"Exit Codes", {
    {"0", "normal"},
    {"1", "error"},
  }});

  pg.info({"Meta", {
    {"", "The version format is 'major.minor.patch (day.month.year)'."},
  }});

  pg.info({"Repository", {
    {"", "https://github.com/octobanana/octavia.git"},
  }});

  pg.info({"Homepage", {
    {"", "https://octobanana.com/software/octavia"},
  }});

  pg.author("Brett Robinson (octobanana) <octobanana.dev@gmail.com>");

  // general flags
  pg.set("help,h", "Print the help output.");
  pg.set("version,v", "Print the program version.");
  pg.set("license", "Print the program license.");

  // options
  pg.set("colour", "auto", "on|off|auto", "Print the program output with colour either on, off, or auto based on if stdout is a tty, the default value is 'auto'.");

  // allow and capture positional arguments
  // pg.set_pos();
}

inline bool program_color(std::string_view color) {
  return color == "auto" ?
    OB::Term::is_term(STDOUT_FILENO) :
    color == "on";
}

inline int program_info(OB::Parg& pg) {
  // init info/options
  program_init(pg);

  // parse options
  auto const status {pg.parse()};

  // set output color choice
  pg.color(program_color(pg.get<std::string>("colour")));

  if (status < 0) {
    // an error occurred
    std::cerr
    << pg.usage()
    << "\n"
    << pg.error();

    return -1;
  }

  if (pg.get<bool>("help")) {
    // show help output
    std::cout << pg.help();

    return 1;
  }

  if (pg.get<bool>("version")) {
    // show version output
    std::cout << pg.version();

    return 1;
  }

  if (pg.get<bool>("license")) {
    // show license output
    std::cout << pg.license();

    return 1;
  }

  // success
  return 0;
}

#endif // INFO_HH
