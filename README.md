# Cybin

v _(thumbnail links to intro video)_ v

[![Cybin: An Introduction](https://img.youtube.com/vi/4lTUlEk2jl0/0.jpg)](https://www.youtube.com/watch?v=4lTUlEk2jl0)

Cybin is a dependency-light, LuaJIT-based audio programming environment suitable for realtime performance and offline rendering. Unlike many popular multimedia programming environments, it strives to do everything in one place all at once, and its strength and weakness lies in completely reinventing the wheel in order to offer the user sample-accurate timing and complete control.

The best way to understand Cybin is to understand what it does, and by extension, what it does not do.

What it does is provide the user with:
* a Lua interpreter that calls a yet-to-be-defined function called `__process` that recieves an arbitrary number of audio samples _(one per channel)_ and outputs an arbitrary number of audio samples _(one per channel)_ every 1/`cybin.samplerate` seconds
* a global table called `cybin` that provides some basic audio/music utilties that would be difficult or impossible to provide adequately using a Lua interpreter alone _(audio system info, MIDI I/O, audio file handling, etc...)_
* a set of command line arguments that may be used to configure your Cybin environment
* multiple optional libraries that contain opinionated sets of tools for creating music and manipulating audio in Lua
* a seamless way to transition from realtime performance and production to offline rendering 

What it does not do is:
* tell the user how to make music
* tell the user what is or is not music
* tell the user it should be used to make music
* tell the user it should or should not be used for any particular purpose
* anything else

## Installation:
* Make sure you've got a C++ compiler
* Install [`LuaJIT`](https://github.com/LuaJIT/LuaJIT), [`JACK`](https://github.com/jackaudio), and [`libsndfile`](https://github.com/erikd/libsndfile) _(MacOS dependencies are available through [brew](https://brew.sh/))_
* Install [`git`](https://git-scm.com/)
* Run `cd && git clone https://github.com/efairbanks/cybin.git && cd cybin`
* Run `./LINUX_BUILD.SH` or `MACOS_BUILD.SH` depending on your platform _(Windows support hopefully coming soon)_
* Put `export PATH=$PATH:~/cybin` in your `.bash_profile` or other shell startup file _(or symlink to the `cybin` executable, whatever floats your boat)_

## Optional
* Install [`Emacs`](https://www.gnu.org/software/emacs/)
* Place the following into your Emacs init file (`~/.emacs`, `~/.emacs.el`, or `~/.emacs.d/init.el`)
```
(add-to-list 'load-path "~/cybin")
(autoload 'cybin-mode "cybin-mode" "Cybin editing mode." t)
(add-to-list 'auto-mode-alist '("\\.cybin$" . cybin-mode))
(add-to-list 'interpreter-mode-alist '("cybin" . cybin-mode))
```
### Emacs Hotkeys
* `C-c C-c` -> `execute line`
* `C-c C-b` -> `execute file`
* `C-c C-e` -> `execute block/paragraph`

**caveats:** _The way emacs traditionally sends blocks of code to subprocesses has something like a 1kB character limit, so large blocks should be broken up or imported using `dofile()` or `require()`._

## Goals:

* Few dependencies (relatively speaking) _(done)_
* Real-time, from-scratch audio synthesis in LUA _(done)_
* ~~Real-time, from-scratch video & image synthesis with LUA and OpenGL _(tbd)_~~ _(this coming eventually as a separate-but-related utility)_
* Seamless transition from live performance to high-quality offline rendering _(done)_
* A robust standard library of real-time-capable audio manipulation and synthesis utilities all written in Lua _(done)_

## Dependencies
* [LuaJIT](https://github.com/LuaJIT/LuaJIT)
* [libsndfile](https://github.com/erikd/libsndfile)
* [JACK](https://github.com/jackaudio)
