# Cybin

[![CYBIN_V0.0_DEMO](https://img.youtube.com/vi/aEwmsLO0rBU/0.jpg)](https://www.youtube.com/watch?v=aEwmsLO0rBU)

This is Cybin. This is early, unstable, _*alpha*_ software. You've been warned.

## Installation:
* Make sure you've got a C++ compiler
* Install [`LuaJIT`](https://github.com/LuaJIT/LuaJIT), [`libsoundio`](https://github.com/andrewrk/libsoundio), and [`LibSndFile`](https://github.com/erikd/libsndfile)
* Run `./debian_build.sh` or `macos_build.sh` depending on your platform
* Put `export PATH=$PATH:/path/to/cybin/root/directory` in your `.bash_profile` or other shell startup file

## Optional
* Install Emacs
* Place the following into your Emacs init file (`~/.emacs`, `~/.emacs.el`, or `~/.emacs.d/init.el`)
```
(add-to-list 'load-path "/Users/ericfai/Cybin")
(autoload 'cybin-mode "cybin-mode" "Cybin editing mode." t)
(add-to-list 'auto-mode-alist '("\\.cybin$" . cybin-mode))
(add-to-list 'interpreter-mode-alist '("cybin" . cybin-mode))
```

## Goals:

* Few dependencies (relatively speaking) _(done)_
* Real-time, from-scratch audio synthesis in LUA _(done)_
* Real-time, from-scratch video & image synthesis with LUA and OpenGL _(tbd)_
* Seamless transition from live performance to high-quality offline rendering _(tbd)_
* Loading audio, video, and image data _(tbd)_
* On-the-fly reloading of LUA/OpenGL code with file watchers or a REPL _(tbd)_
* C++ classes wrapped in LUA interfaces with efficient implementations of common effects and techinques _(tbd)_
  * granular synthesis
  * ray-marching
  * cellular automata
  * etc...
* Mind-blowing demos and example code _(tbd)_

## Dependencies
* [LuaJIT](https://github.com/LuaJIT/LuaJIT)
* [libsoundio](https://github.com/andrewrk/libsoundio)
* [libsndfile](https://github.com/erikd/libsndfile)
