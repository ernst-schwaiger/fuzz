
#
# FIXME add include paths to fuzzed project
#
INCLUDE_PATHS= \
	-I.

#
#  FIXME add c files of project to fuzz
#
CFILES= \
	fuzz.c

#
# FIXME adapt vpath, add folders of fuzzed project which contain one or more of
# the .c files listed above
#
vpath %.c \
	.

#
# FIXME add defines, aka preprocessor symbols
#
DEFINES= \
	-DFUZZ

#
# FIXME: Add linker options. If the fuzzed project requires additional libraries
# to be lined, add them here
#
LINK_OPTS=


CC=clang
CXX=clang++
COMPILE_OPTS=-O0 -g
FUZZ_OPTS=-fsanitize=fuzzer,address,signed-integer-overflow
COV_OPTS=-fprofile-instr-generate -fcoverage-mapping
COMPILE_OPTS_ALL=$(COMPILE_OPTS) $(INCLUDE_PATHS) $(DEFINES)
OBJFILES_FUZZ := $(addprefix objfuzz/, $(notdir $(patsubst %.c,%.o,$(CFILES))))
OBJFILES_COV := $(addprefix objcov/, $(notdir $(patsubst %.c,%.o,$(CFILES))))
OBJFILES_DBG := $(addprefix objdbg/, $(notdir $(patsubst %.c,%.o,$(CFILES))))


all: fuzz cov

objfuzz:
	mkdir -p objfuzz

objcov:
	mkdir -p objcov

objdbg:
	mkdir -p objdbg

objfuzz/%.o: $(notdir %.c) | objfuzz
	$(CC) -c $(COMPILE_OPTS_ALL) $(FUZZ_OPTS) $< -o $@

objcov/%.o: $(notdir %.c) | objcov
	$(CC) -c $(COMPILE_OPTS_ALL) $(COV_OPTS) -DCOVERAGE $< -o $@

fuzz: $(OBJFILES_FUZZ)
	$(CC) $(FUZZ_OPTS) $(OBJFILES_FUZZ) -o fuzz $(LINK_OPTS)

cov: $(OBJFILES_COV)
	$(CC) $(COV_OPTS) $(OBJFILES_COV) -o cov $(LINK_OPTS)

coverage: cov
	./cov CORPUS
	llvm-profdata merge -sparse default.profraw -o default.profdata
	llvm-cov show -format=html -instr-profile=default.profdata cov -output-dir=coverage/

CORPUS:
	mkdir -p CORPUS
	cp -f dict/* CORPUS

test: fuzz CORPUS
	./fuzz CORPUS -jobs=6 -workers=6 -max_total_time=900 -use_value_profile=1 -max_len=256

clean:
	rm -rf objfuzz objcov objdbg fuzz cov coverage *.log default.profdata  default.profraw
