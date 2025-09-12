CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

SRC_DIR = src
INCLUDE_DIR = include
TARGET = super

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRC:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f super_out.c super_out
