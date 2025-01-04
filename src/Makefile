OBJ_FOLDER=obj

all: prepare ${OBJ_FOLDER}/vge.a

${OBJ_FOLDER}/vge.a: ${OBJ_FOLDER}/main.o ${OBJ_FOLDER}/screen.o ${OBJ_FOLDER}/rom_select.o ${OBJ_FOLDER}/input.o ${OBJ_FOLDER}/cartridge.o ${OBJ_FOLDER}/memory.o ${OBJ_FOLDER}/cpu.o ${OBJ_FOLDER}/interrupt.o ${OBJ_FOLDER}/timer.o ${OBJ_FOLDER}/cpu_debug.o
	ar r $@ $^

prepare:
	mkdir ${OBJ_FOLDER} ${DISCARD_ERROR}

clean:
	${RM} ${OBJ_FOLDER} ${DISCARD_ERROR}

clean-all: clean

${OBJ_FOLDER}/main.o: main.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/screen.o: screen.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/rom_select.o: rom_select.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/input.o: input.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/cartridge.o: cartridge.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/memory.o: memory.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/cpu.o: cpu.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/interrupt.o: interrupt.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/timer.o: timer.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@

${OBJ_FOLDER}/cpu_debug.o: cpu_debug.c
	${CC} ${C_FLAGS} ${INCLUDES} -c $^ -o $@
