CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
SRC_DIR = src
INCLUDE_DIR = include
TARGET = super

# Find all .c files in the source directory
SRC = $(wildcard $(SRC_DIR)/*.c)
# Create a list of object files
OBJS = $(SRC:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f super_out.c super_out
