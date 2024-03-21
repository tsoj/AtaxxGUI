<div align="center">
<p><h1>AtaxxGUI</h1>
<i><h4>A graphical user interface for playing Ataxx between engines and humans</h4></i>
<img src="./screenshot.png" height="384px">
</h1>
</div>

## Compile

You need to install Boost, Qt6, git, CMake, and a C++ compiler

### Linux

```bash
git clone https://github.com/tsoj/AtaxxGUI.git
cd AtaxxGUI
mkdir build
cd build
export CC=gcc
export CXX=g++
cmake .. && make -j
```

### Windows

```bash
git clone https://github.com/tsoj/AtaxxGUI.git
cd AtaxxGUI
mkdir build
cd build
cmake ..
msbuild .\AtaxxGUI.sln /p:Configuration=Release
```

There are also windows binaries available under releases.

## Credits

- A lot of the board visualization in [src/boardview/](src/boardview/) is taken and modified from [Cute Chess](https://github.com/cutechess/cutechess)
- [Cuteataxx](https://github.com/kz04px/cuteataxx) and [libataxx](https://github.com/kz04px/libataxx) by kz04px
- [Lidraughts](https://github.com/RoepStoep/lidraughts) and [Lichess](https://github.com/lichess-org/lila) for some of the board and piece themes
- The inventors of Ataxx
- The engine programming community
