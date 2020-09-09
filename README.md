# octavia
octobanana's customizable text-based audio visualization interactive application.

![octavia](https://raw.githubusercontent.com/octobanana/octavia/master/res/octavia.png)

## Contents
* [About](#about)
  * [Features](#features)
  * [Future Features](#future-features)
* [Usage](#usage)
  * [Getting Started](#getting-started)
* [Pre-Build](#pre-build)
  * [Environments](#environments)
  * [Compilers](#compilers)
  * [Dependencies](#dependencies)
  * [Linked Libraries](#linked-libraries)
  * [Included Libraries](#included-libraries)
  * [macOS](#macos)
* [Build](#build)
* [Install](#install)
* [Troubleshooting](#troubleshooting)
* [License](#license)

## About
__OCTAVIA__, **O**ctobanana's **C**ustomizable **T**ext-based **A**udio **V**isualization **I**nteractive **A**pplication.

__octavia__ is an audio visualizer for the terminal.

### Features
* Capture and visualize audio in real-time
* Keybindings allow interactive configuration during runtime
* Displays the frequency spectrum as bars and peaks
* Each bar represents a logarithmically spaced frequency range
* The height of each bar represents the magnitude of the frequency range in decibels
* The bars and peaks move at different speeds
* Uses unicode block elements for smooth transitions
* Mono or stereo audio capture
* Vertical or horizontal layout
* Shared or stacked stereo layout
* Fixed height bar mode
* True colour support
* Low-high or high-low frequency ordering
* Left/right stereo channel layout can be swapped
* Configurable bar width and spacing
* Configurable min/max decibel thresholds
* Configurable min/max frequency thresholds
* Savitzky–Golay filter for smoothing
* IIR low pass filter
* IIR high pass filter
* IIR high shelf filter

### Future Features
* UI overlay
* Custom colours
* Custom keybindings
* Command prompt
* Config file support
* Config file live reload
* Output raw characters to stdout or a file

## Usage
View the usage and help output with the `-h|--help` flag,
or as a plain text file in `./doc/help.txt`.

### Getting Started
Run `octavia` and play some audio!

Press `?` to view the keybindings section of the help output, press `q` to return to the program.  
Press `q` to quit.  
Press `o` to toggle the debug overlay.  
Press `a` to toggle vertical/horizontal layout.  
Press `s` to toggle mono/stereo audio capture.  
Press `d` to toggle shared/stacked layout.  
Press `f` to toggle flipped layout.  
Press `z` or `x` to toggle bars/peaks on/off.  
Press `g` to cycle through available smoothing filters.  
Press `h` or `H` to change the minimum frequency threshold.  
Press `j` or `J` to change the minimum decibel threshold.  
Press `j` or `J` to change the maximum decibel threshold.  
Press `l` or `L` to change the maximum frequency threshold.  

## Pre-Build
This section describes what environments this program may run on,
any prior requirements or dependencies needed, and any third party libraries used.

> #### Important
> Any shell commands using relative paths are expected to be executed in the
> root directory of this repository.

### Environments
* __Linux__ (supported)
* __BSD__ (supported)
* __macOS__ (supported)

### Compilers
* __GCC__ >= 8.0.0 (supported)
* __Clang__ >= 7.0.0 (supported)
* __Apple Clang__ >= 11.0.0 (untested)

### Dependencies
* __CMake__ >= 3.8
* __Boost__ >= 1.72.0
* __ICU__ >= 62.1
* __SFML__ >= 2.5.1
* __PThread__

### Linked Libraries
* __pthread__ (libpthread) POSIX threads library
* __icuuc__ (libicuuc) part of the ICU library
* __icui18n__ (libicui18n) part of the ICU library
* __sfml-audio__ (libsfml-audio) part of the SFML library
* __sfml-system__ (libsfml-system) part of the SFML library
* __boost_coroutine__ (libboost_coroutine) Boost coroutine library

### Included Libraries
* [__Belle__](https://github.com/octobanana/belle):
  Asynchronous input and signal handling, modified and included as `./src/ob/belle`
* [__Parg__](https://github.com/octobanana/parg):
  CLI arg parser, modified and included as `./src/ob/parg.hh`
* [__kissfft__](https://github.com/mborgerding/kissfft):
  A mixed-radix Fast Fourier Transform, modified and included as `./src/ob/fft.hh`

### macOS
Using a new version of __GCC__ or __Clang__ is __required__, as the default
__Apple Clang compiler__ does __not__ support C++17 Standard Library features such as `std::filesystem`.

A new compiler can be installed through a third-party package manager such as __Brew__.
Assuming you have __Brew__ already installed, the following commands should install
the latest __GCC__.

```sh
brew install gcc
brew link gcc
```

The following CMake argument will then need to be appended to the end of the line when running the shell script.
Remember to replace the placeholder `<path-to-g++>` with the canonical path to the new __g++__ compiler binary.

```sh
./RUNME.sh build -- -DCMAKE_CXX_COMPILER='<path-to-g++>'
```

## Build
The included shell script will build the project in release mode using the `build` subcommand:

```sh
./RUNME.sh build
```

## Install
The included shell script will install the project in release mode using the `install` subcommand:

```sh
./RUNME.sh install
```

## Troubleshooting
If __octavia__ is not responding to audio being played, make sure your systems default audio capture stream is set to monitor of built-in audio analog stereo, or something similar. On Linux, this can be done with the program __pavucontrol__. While __octavia__ is running, launch __pavucontrol__. Within __pavucontrol__, select the recording tab, then choose monitor of built-in audio analog stereo in the drop down for the capture stream of __octavia__.

## License
This project is licensed under the MIT License.

Copyright (c) 2020 [Brett Robinson](https://octobanana.com/)

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
