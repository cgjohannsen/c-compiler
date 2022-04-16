# tool macros
CC := gcc

# path macros
OBJ_PATH := Target
SRC_PATH := Source
DOC_PATH := Documentation

SRC_DIRS := $(SRC_PATH) $(SRC_PATH)/util $(SRC_PATH)/parse 
OBJ_DIRS := $(OBJ_PATH) $(addprefix $(OBJ_PATH), $(subst $(SRC_PATH),,$(SRC_DIRS)))

# compile macros
CFLAGS := 
#-fdata-sections -ffunction-sections -fno-common \
-fsanitize=undefined -fsanitize=address\
-pedantic -Waggregate-return -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wdisabled-optimization -Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wextra -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-truncation -Wformat-y2k -Wformat=2 -Wimplicit -Wimport -Winit-self -Winline -Winvalid-pch -Wlogical-op -Wlong-long -Wmisleading-indentation -Wmissing-declarations -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wmissing-prototypes -Wnested-externs -Wnoexcept -Wnon-virtual-dtor -Wnull-dereference -Wodr -Wold-style-cast -Woverloaded-virtual -Wpacked -Wpedantic -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstack-protector -Wstrict-aliasing=2 -Wstrict-null-sentinel -Wstrict-overflow=5 -Wstrict-prototypes -Wswitch-default -Wundef -Wundef -Wunreachable-code -Wunused -Wunused-parameter -Wuseless-cast -Wvariadic-macros -Wwrite-strings -Wzero-as-null-pointer-constant -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes \
-Wno-switch-enum -Wno-unknown-warning-option -Wno-gnu-binary-literal \
--coverage
CCOBJFLAG := $(CFLAGS) -c

TARGET_NAME := mycc
MAIN_SRC := $(SRC_PATH)/$(TARGET_NAME).c

# src files & obj files
SRC := $(foreach x, $(SRC_DIRS), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ_RELES := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(subst $(SRC_PATH)/,,$(SRC)))))

default: all

# non-phony targets
$(TARGET_NAME): $(OBJ_RELES)
	$(CC) $(CFLAGS) -O3 -o $@ $?

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* | $(OBJ_PATH)
	$(CC) $(CCOBJFLAG) -o $@ $<

developers: $(DOC_PATH)/developers.tex
	pdflatex -output-directory Documentation Documentation/developers.tex

# Path rules
$(OBJ_PATH):
	mkdir -p $(OBJ_DIRS)

CLEAN_LIST := $(TARGET_NAME) \
			  $(OBJ_PATH) \
			  $(DOC_PATH)/*.pdf

# phony rules
.PHONY: all
all: $(TARGET_RELES) developers

.PHONY: mycc
mycc: $(TARGET_RELES)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -rf $(CLEAN_LIST)