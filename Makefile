CC=gcc
MAKE=make

C_FLAGS=-Wall -Wextra -pedantic -std=c23 -g2 -march=x86-64-v3

BINARY_NAME=VoxoR_gameboy_emulator
BIN_FOLDER=bin
BINARY_FULLNAME=${BIN_FOLDER}/${BINARY_NAME}

SDL3_PATH=/home/mathieu/Documents/dev/tools/SDL/bin
SDL3_INCLUDES=${SDL3_PATH}/include
SDL3_LIB=${SDL3_PATH}/lib

SDL3_TTF_PATH=/home/mathieu/Documents/dev/tools/SDL_ttf/bin
SDL3_TTF_INCLUDES=${SDL3_TTF_PATH}/include
SDL3_TTF_LIB=${SDL3_TTF_PATH}/lib

PWD=${shell pwd}

INCLUDES=-I${PWD}/lib/log/inc -I${SDL3_INCLUDES} -I${SDL3_TTF_INCLUDES}

LINKER_PATH=-L${SDL3_LIB} -L${SDL3_TTF_LIB} -Llib/lib
LINKER_FLAGS=-fuse-ld=gold -lSDL3 -lSDL3_ttf '-Wl,-rpath,${SDL3_LIB}' '-Wl,-rpath,${SDL3_TTF_LIB}' -llog

export CC MAKE C_FLAGS INCLUDES

all: ${BINARY_FULLNAME}

${BINARY_FULLNAME}: lib vge
	mkdir -p ${BIN_FOLDER}
	${CC} src/obj/vge.a -o ${BINARY_FULLNAME} ${LINKER_PATH} ${LINKER_FLAGS}

lib: FORCE
	cd lib && ${MAKE} all

FORCE: ;

vge:
	cd src && ${MAKE} all

clean:
	cd lib && ${MAKE} clean
	cd src && ${MAKE} clean

clean-all: 
	rm -rf ${BIN_FOLDER}
	cd lib && ${MAKE} clean-all
	cd src && ${MAKE} clean-all
