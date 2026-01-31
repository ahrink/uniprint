# ---- Toolchain ----
CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -pedantic -std=c99
INCLUDES = -Iincludes

# ---- Layout ----
SRC_DIR = src
INC_DIR = includes
BIN     = uniprint

# ---- Sources ----
SRCS = \
        $(SRC_DIR)/ahr_utf8.c \
        $(SRC_DIR)/uniprint.c

OBJS = $(SRCS:.c=.o)

# ---- Default ----
all: $(BIN)

# ---- Link ----
$(BIN): $(OBJS)
	$(CC) -o $@ $^

# ---- Compile ----
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/ahr_utf8.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# ---- Hygiene ----
clean:
	rm -f $(SRC_DIR)/*.o $(BIN)

.PHONY: all clean
