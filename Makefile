# Compiler and flags
CC = gcc
# Add GTK 4 flags to CFLAGS
# 'pkg-config --cflags gtk4' provides include paths
# 'pkg-config --libs gtk4' provides library paths and links
CFLAGS = -Wall -Wextra -std=c11 `pkg-config --cflags gtk4`
LIBS = `pkg-config --libs gtk4` -lm

# --- Original CLI Target ---

# Source files for the backend logic
BACKEND_SRCS = minigit.c search_engine.c ranking.c autocomplete.c
BACKEND_OBJS = $(BACKEND_SRCS:.c=.o)

# Source file for the CLI
CLI_SRC = cli.c
CLI_OBJ = $(CLI_SRC:.c=.o)

# CLI Target executable
TARGET_CLI = minigitsearch

# --- New GUI Target ---

# Source file for the GUI
GUI_SRC = gui.c
GUI_OBJ = $(GUI_SRC:.c=.o)

# GUI Target executable
TARGET_GUI = minigitgui

# --- Build Rules ---

# Default rule: build both targets
all: $(TARGET_CLI) $(TARGET_GUI)

# Rule to build the CLI target
$(TARGET_CLI): $(CLI_OBJ) $(BACKEND_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET_CLI) $(CLI_OBJ) $(BACKEND_OBJS) $(LIBS)

# Rule to build the GUI target
# Note: It links the GUI object and backend objects, but NOT the CLI object
$(TARGET_GUI): $(GUI_OBJ) $(BACKEND_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET_GUI) $(GUI_OBJ) $(BACKEND_OBJS) $(LIBS)

# Generic rule to build .o files from .c files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(BACKEND_OBJS) $(CLI_OBJ) $(GUI_OBJ) $(TARGET_CLI) $(TARGET_GUI)

# Phony targets
.PHONY: all clean