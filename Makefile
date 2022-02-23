# tool macros
CC := gcc

# path macros
BIN_PATH := Bin
OBJ_PATH := Target
SRC_PATH := Source
DOC_PATH := Documentation

SRC_DIRS := $(SRC_PATH) $(SRC_PATH)/util $(SRC_PATH)/parse 
OBJ_DIRS := $(OBJ_PATH) $(addprefix $(OBJ_PATH), $(subst $(SRC_PATH),,$(SRC_DIRS)))

# compile macros
CFLAGS := -Wall
CCOBJFLAG := $(CFLAGS) -c

TARGET_NAME := mycc
TARGET_RELES := $(BIN_PATH)/$(TARGET_NAME)
MAIN_SRC := $(SRC_PATH)/$(TARGET_NAME).c

# src files & obj files
SRC := $(foreach x, $(SRC_DIRS), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ_RELES := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(basename $(subst $(SRC_PATH)/,,$(SRC)))))

default: all

# non-phony targets
$(TARGET_RELES): $(OBJ_RELES) | $(BIN_PATH)
	$(CC) $(CFLAGS) -O3 -o $@ $?

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c* | $(OBJ_PATH)
	$(CC) $(CCOBJFLAG) -o $@ $<

developers: $(DOC_PATH)/developers.tex
	pdflatex -output-directory Documentation Documentation/developers.tex

# Path rules
$(BIN_PATH):
	mkdir -p $(BIN_PATH)

$(OBJ_PATH):
	mkdir -p $(OBJ_DIRS)

CLEAN_LIST := $(BIN_PATH) \
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