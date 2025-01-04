# Compilation

## Linux

TODO

## Windows

go to https://winlibs.com/ and download the latest GCC Zip archive (use UCRT runtime unless you are using Windows 8.1 or older).  
you will find inside everything needed to compile! (gcc, make, cmake ...)

clone the project:
```
git clone https://github.com/VoxoR7/gameboy_emulator_v2.git
cd gameboy_emulator_v2
git submodule update --init --recursive
```

Then, you will need SDL3 and SDL_ttf.

To do so, first get ninja: https://github.com/ninja-build/ninja/releases

For all the ommands below, be sure to replace <path_to> with the acutal path in your FS.  
Also notice the flag "-march=x86-64-v3" which should be supported by all processor nowadays.  
But to compile on older computer, you might need to remove this flag.

Then let's go with SDL:
```
git clone https://github.com/libsdl-org/SDL
cd SDL
mkdir bin build
cd build
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe -DCMAKE_MAKE_PROGRAM=C:<path_to>\ninja-win\ninja.exe -DCMAKE_C_COMPILER=C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\gcc.exe -DCMAKE_C_FLAGS="-march=x86-64-v3" -DCMAKE_CXX_FLAGS="-march=x86-64-v3" -DCMAKE_BUILD_TYPE=Release --install-prefix=C:<path_to>\SDL\bin -G Ninja ..
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe --build . --config Release --parallel
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe --install . --config Release
```

And you should be done for SDL, let's get SDl_ttf now!  
but first we'll need freetype:

get freetype https://download.savannah.gnu.org/releases/freetype/

```
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\mingw32-make.exe CC=C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\gcc.exe
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\mingw32-make.exe CC=C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\gcc.exe
```
(yes, do it two time!)

Then, you can compile SDL_ttf:

```
git clone https://github.com/libsdl-org/SDL_ttf.git
cd SDL_ttf
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe -DCMAKE_C_COMPILER=C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\gcc.exe -DCMAKE_C_FLAGS="-march=x86-64-v3" -DCMAKE_CXX_FLAGS="-march=x86-64-v3" -DCMAKE_BUILD_TYPE=Release --install-prefix=C:<path_to>\SDL_ttf\bin -DSDL3_DIR=C:<path_to>\SDL\bin\lib\cmake\SDL3 -DFREETYPE_INCLUDE_DIRS=C:<path_to>\freetype-2.13.3\freetype-2.13.3\include -DFREETYPE_LIBRARY=C:<path_to>\freetype-2.13.3\freetype-2.13.3\objs\freetype.a -G Ninja -DCMAKE_MAKE_PROGRAM=C:<path_to>\ninja-win\ninja.exe ..
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe --build . --config Release --parallel
C:<path_to>\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe --install . --config Release
```

Once you have compiled everything, Open all the makefile in the project and replace all tools (CC, MAKE, CMAKE, SDL, SDL_TTF ...) with the path on you computer.  
The with a simple call to `make`, everything should compile fine!

One last thing, unlike Linux, the `rpath` option for the linker does not work on windows, you will need to get the dynmaic librairies and put in in the bin folder with the final executable.  
This should be straightforward though.
