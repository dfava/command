command
=======

Project involving Flex, Bison, LLVM, C/C++, type checking and compilers.

The goal was to implement a basic compiler and type checker for a simple
programming language used to describe information flow concepts on a paper
by Sabelfeld and Myers. 

* A. Sabelfeld, A. Myers., 
  "Language-based information-flow security."
  Selected Areas in Communications, IEEE Journal on, 21(1):5–19, 2003.

This work is based on a blog by Segal and on the LLVM Kaleidoscope tutorial:

* L. Segal. Writing your own compiler.
  http://gnuu.org/2009/09/18/writing-your-own-toy-compiler
  2009.

* Kaleidoscope: Implementing a Language with LLVM.
  http://llvm.org/docs/tutorial


### Requirements ###

* flex
* bison
* llvm

The versions used for the project were:

* flex 2.5.35 Apple(flex-31)
* bison (GNU Bison) 3.0.2
* LLVM version 3.5.0svn

LLVM was built from source using the following commits:

* llvm
  commit 6d4e5ab349be65305d83176ad4f3ab6dfbc08292
  Date:   Wed Apr 2 21:22:03 2014 +0000

* compiler-rt
  commit 2686e6405ef2d5f6755efc511bc455c3a82d4a2c
  Date:   Wed Apr 2 13:09:22 2014 +0000

* test-suite
  commit da359830846d3351367595ea885686404e7adee4
  Date:   Tue Mar 25 04:35:27 2014 +0000

* clang
  commit ce167d998928ace8e12dd0edf7b5104850b2b325
  Date:   Wed Apr 2 18:28:36 2014 +0000

Instructions on how to build LLVM can be found online.  They will roughly be:

    $ cd ~
    $ mkdir llvm
    $ git clone http://llvm.org/git/llvm.git
    $ cd ~/llvm/llvm/tools
    $ git clone http://llvm.org/git/clang.git
    $ cd ~/llvm/llvm/projects
    $ git clone http://llvm.org/git/compiler-rt.git
    $ git clone http://llvm.org/git/test-suite.git
    $ cd ~/llvm
    $ mkdir obj_root 
    $ mkdir build
    $ cd obj_root 
    $ ../llvm/configure --prefix=/Users/dfava/llvm/build --enable-debug-runtime --enable-jit --disable-optimized --disable-terminfo
    $ make -j 8
    $ make install
    $ make -C runtime install-bytecode


### Usage ###

Example usage:

    $ ./command -h
    $ ./command -f ./examples/example_if3.cmd -v 1
