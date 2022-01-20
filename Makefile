# tool macros
CC := gcc

# path macros
BIN_PATH := bin
OBJ_PATH := target/release
SRC_PATH := Source
DBG_PATH := target/debug

SRC_DIRS := $(SRC_PATH) $(SRC_PATH)/TL $(SRC_PATH)/binParser $(SRC_PATH)/AT $(SRC_DIRS)
IDIR=$(SRC_DIRS)
OBJ_DIRS := $(OBJ_PATH) $(addprefix $(OBJ_PATH), $(subst $(SRC_PATH),,$(SRC_DIRS)))
OBJ_DBG_DIRS := $(DBG_PATH) $(addprefix $(DBG_PATH), $(subst $(SRC_PATH),,$(SRC_DIRS)))

CFLAGS := $(CFLAGS)
DBGFLAG := -g -O0 \
	-fdata-sections -ffunction-sections -fno-common -fsanitize=undefined -fsanitize=address -pedantic -Waggregate-return -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wdisabled-optimization -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wextra -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-truncation -Wformat-y2k -Wformat=2 -Wimplicit -Wimport -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wlong-long -Wmisleading-indentation -Wmissing-declarations -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs -Wnoexcept -Wnon-virtual-dtor -Wnull-dereference -Wodr -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpedantic -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstack-protector -Wstrict-aliasing=2 -Wstrict-null-sentinel -Wstrict-overflow=5 -Wstrict-prototypes -Wswitch-default -Wundef -Wundef -Wunreachable-code -Wunused -Wunused-parameter -Wuseless-cast -Wvariadic-macros -Wwrite-strings -Wzero-as-null-pointer-constant -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wno-switch-enum -Wno-unknown-warning-option -Wno-gnu-binary-literal
CCOBJFLAG := $(CFLAGS) -c

TARGET_NAME := mycc
TARGET_RELES := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(BIN_PATH)/$(TARGET_NAME)_debug
ifeq ($(OS),Windows_NT)
	TARGET_RELES := $(addsuffix .exe,$(TARGET_RELES))
	TARGET_DEBUG := $(addsuffix .exe,$(TARGET_DEBUG))
endif
MAIN_SRC := src/$(TARGET_NAME).c

# src files & obj files
SRC := $(foreach x, $(SRC_DIRS), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ_RELES := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(subst src/,,$(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(basename $(subst src/,,$(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ_PATH) \
				  $(DBG_PATH)
CLEAN_LIST := $(TARGET_RELES) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST) \
				src/binParser/config.c

# default rule
default: all

# non-phony targets
$(TARGET_RELES): $(OBJ_RELES) | $(BIN_PATH)
	$(CC) $(CFLAGS) -O3 -o $@ $? $(LDFLAGS)

$(OBJ_PATH)/%.o: src/%.c* | $(OBJ_PATH)
	$(CC) $(CCOBJFLAG) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG) | $(DBG_PATH)
	$(CC) $(CFLAGS) $(DBGFLAG) -o $@ $? $(LDFLAGS)

$(DBG_PATH)/%.o: src/%.c* | $(DBG_PATH)
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $<

# Path rules
$(BIN_PATH):
	mkdir -p $(BIN_PATH)

$(OBJ_PATH):
	mkdir -p $(OBJ_DIRS)

$(DBG_PATH):
	mkdir -p $(OBJ_DBG_DIRS)

# phony rules
.PHONY: all
all: $(TARGET_RELES)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -rf $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(DISTCLEAN_LIST)
	@rm -rf $(DISTCLEAN_LIST)