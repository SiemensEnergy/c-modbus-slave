CC := clang
LD := clang

TEST_SRC_DIR := src
BUILD_DIR := build

LIB_SRC := \
	endian.c \
	mbadu_ascii.c \
	mbadu_tcp.c \
	mbadu.c \
	mbcoil.c \
	mbcrc.c \
	mbfn_coils.c \
	mbfn_digs.c \
	mbfn_regs.c \
	mbfn_serial.c \
	mbinst.c \
	mbpdu.c \
	mbreg.c \
	mbsupp.c \
	mbtest.c

TEST_SRC := ${sort ${wildcard ${TEST_SRC_DIR}/*.c}}
TESTS := ${patsubst ${TEST_SRC_DIR}/%.c,${BUILD_DIR}/${TEST_SRC_DIR}/%,${TEST_SRC}}

TEST_OBJ := ${addprefix ${BUILD_DIR}/, ${TEST_SRC:.c=.o}}
LIB_OBJ := ${addprefix ${BUILD_DIR}/lib/, ${LIB_SRC:.c=.o}}

DEP_FILES := ${TEST_OBJ:.o=.d} ${LIB_OBJ:.o=.d}

CFLAGS := -std=c11 -I../src
# Generate dependency information
CFLAGS += -MP -MMD

# Clang Undefined Behavior Sanitizer
ifeq (${CC}, clang)
CFLAGS += \
	-fsanitize=undefined \
	-fsanitize-trap=all \
	-fsanitize=implicit-conversion \
	-fsanitize=integer \
	-fsanitize=nullability
endif

LDFLAGS :=

.PHONY: test clean

test: ${TESTS}
	${subst ${} ${}, && ,${^:%=./%}}

clean:
	@rm -rf ${BUILD_DIR}/

${TESTS}: ${LIB_OBJ} ${TEST_OBJ}
	@mkdir -p ${dir $@}
	${LD} -o $@ $@.o ${LIB_OBJ} ${LDFLAGS}

${BUILD_DIR}/%.o: %.c Makefile | ${BUILD_DIR}
	@mkdir -p ${dir $@}
	${CC} ${CFLAGS} -o $@ -c $<

${BUILD_DIR}/lib/%.o: ../src/%.c Makefile | ${BUILD_DIR}
	@mkdir -p ${dir $@}
	${CC} ${CFLAGS} -o $@ -c $<

${BUILD_DIR}:
	@mkdir $@

-include ${DEP_FILES}
