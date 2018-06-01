# Cybin

[![CYBIN_V0.0_DEMO](https://github.com/efairbanks/Cybin/blob/master/cybin.png?raw=true)](https://www.youtube.com/watch?v=aEwmsLO0rBU)

This is Cybin. This is ~~early, unstable, _**alpha**_ software~~ actually relatively stable now and comes with some decent utilities.

## Installation:
* Make sure you've got a C++ compiler
* Install [`LuaJIT`](https://github.com/LuaJIT/LuaJIT), [`libsoundio`](https://github.com/andrewrk/libsoundio), [`libsndfile`](https://github.com/erikd/libsndfile), [freeglut](http://freeglut.sourceforge.net/), and if you're on MacOS, [XQuartz](https://www.xquartz.org/) _(XQuarts has a native MacOS installer, all other dependencies are available through [brew](https://brew.sh/))_
* Install [`git`](https://git-scm.com/)
* Run `cd && git clone https://github.com/efairbanks/Cybin.git && cd Cybin`
* Run `./debian_build.sh` or `macos_build.sh` depending on your platform
* Put `export PATH=$PATH:~/Cybin` in your `.bash_profile` or other shell startup file

## Optional
* Install [`Emacs`](https://www.gnu.org/software/emacs/)
* Place the following into your Emacs init file (`~/.emacs`, `~/.emacs.el`, or `~/.emacs.d/init.el`)
```
(add-to-list 'load-path "~/Cybin")
(autoload 'cybin-mode "cybin-mode" "Cybin editing mode." t)
(add-to-list 'auto-mode-alist '("\\.cybin$" . cybin-mode))
(add-to-list 'interpreter-mode-alist '("cybin" . cybin-mode))
```
### Emacs Hotkeys
* `C-c C-c` -> `execute line`
* `C-c C-b` -> `execute file`
* `C-c C-e` -> `execute block/paragraph`

**caveats:** _There's currently a pretty low limit to the number of characters you can send to the Cybin interpreter at a time, so watch out or it'll puke all over the place. Note the separation of code blocks in demo.cybin._

## Goals:

* Few dependencies (relatively speaking) _(done)_
* Real-time, from-scratch audio synthesis in LUA _(done)_
* Real-time, from-scratch video & image synthesis with LUA and OpenGL _(tbd)_
* Seamless transition from live performance to high-quality offline rendering _(audio: done, video: tbd)_
* Loading audio, video, and image data _(audio: done, video: tbd, image: tbd)_
* On-the-fly reloading of LUA/OpenGL code with file watchers or a REPL _(tbd)_
* C++ classes wrapped in LUA interfaces with efficient implementations of common effects and techinques _(tbd)_
  * granular synthesis
  * ray-marching
  * cellular automata
  * etc...
* Mind-blowing demos and example code _(tbd unless you like breakcore with repetetive basslines)_

## Dependencies
* [LuaJIT](https://github.com/LuaJIT/LuaJIT)
* [libsoundio](https://github.com/andrewrk/libsoundio)
* [libsndfile](https://github.com/erikd/libsndfile)
* [freeglut](http://freeglut.sourceforge.net/)
* [XQuartz](https://www.xquartz.org/) _(required for MacOS freeglut compatibility)_
