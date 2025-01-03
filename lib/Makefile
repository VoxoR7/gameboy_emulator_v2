LIB_FOLDER=lib

all: prepare log
	cp ./log/build/liblog.a ${LIB_FOLDER}

prepare:
	mkdir -p ${LIB_FOLDER}

log: FORCE
	cd log && mkdir -p build && cd build && cmake .. && make

FORCE: ;

clean:
	rm -rf ./log/build

clean-all: clean
	rm -rf ${LIB_FOLDER}