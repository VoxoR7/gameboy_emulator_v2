CMAKE=C:\Users\mvaud\Documents\info\tools\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2\mingw64\bin\cmake.exe

LIB_FOLDER=lib

all: prepare log
	${CP} $(subst /,${SEP},log/build/liblog.a) ${LIB_FOLDER}

prepare:
	mkdir ${LIB_FOLDER} ${DISCARD_ERROR}

log: FORCE
	cd log && (mkdir build ${DISCARD_ERROR}) && cd build && ${CMAKE} -G"Unix Makefiles" -DCMAKE_MAKE_PROGRAM=${MAKE} -DCMAKE_C_COMPILER=${CC} .. && ${MAKE}

FORCE: ;

clean:
	cd log && ${RM} build ${DISCARD_ERROR}

clean-all: clean
	${RM} ${LIB_FOLDER} ${DISCARD_ERROR}