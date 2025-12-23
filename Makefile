CC := clang
CLANG := clang

SRC_DIR := src
BUILD_DIR := build

SRC := \
	endian.c \
	mbadu_ascii.c \
	mbadu_tcp.c \
	mbadu.c \
	mbcoil.c \
	mbcrc.c \
	mbfn_coils.c \
	mbfn_digs.c \
	mbfn_files.c \
	mbfn_regs.c \
	mbfn_serial.c \
	mbfile.c \
	mbinst.c \
	mbpdu.c \
	mbreg.c \
	mbsupp.c \
	mbtest.c

OBJ := ${addprefix ${BUILD_DIR}/, ${SRC:.c=.o}}
DEP_FILES := ${OBJ:.o=.d}

CFLAGS := \
	-std=c11 \
	-Wall -Wextra -Wmissing-include-dirs \
	-Wswitch-default -Wpedantic \
	-Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

# Generate dependency information
CFLAGS += -MP -MMD

.PHONY: all test clean analyze

all: ${OBJ}

test:
	${MAKE} -C test test

# Requires Clang/LLVM be installed
analyze:
	@echo "Running clang static analyzer..."
	@for src in ${SRC}; do \
		echo "Analyzing $$src..."; \
		output=$$(${CLANG} --analyze \
			-Xanalyzer -analyzer-output=text \
			${SRC_DIR}/$$src 2>&1); \
		if [ -n "$$output" ]; then \
			echo "$$output"; \
			exit 1; \
		fi; \
	done
	@echo "Static analysis complete - no issues found."

clean:
	@rm -rf ${BUILD_DIR}/
	${MAKE} -C test clean

${BUILD_DIR}/%.o: ${SRC_DIR}/%.c Makefile | ${BUILD_DIR}
	${CC} ${CFLAGS} -o $@ -c $<

${BUILD_DIR}:
	@mkdir $@

-include ${DEP_FILES}
