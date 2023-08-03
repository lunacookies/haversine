CC=clang

CFLAGS=\
	-std=c99 \
	-g3 \
	-O3 \
	-Wall \
	-Wpedantic \
	-Wconversion \
	-Wdouble-promotion

NAME=haversine
BUILD_DIR=out
SOURCES=$(wildcard *.c)
EXECUTABLES=$(addprefix $(BUILD_DIR)/, $(SOURCES:.c=))

all: $(EXECUTABLES) tidy

$(BUILD_DIR)/%: %.c
	@ mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

tidy: *.c
	clang-format -i $^

clean:
	rm -r $(BUILD_DIR)
