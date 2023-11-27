# Variables
CC = g++
CFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -Os -fdata-sections -ffunction-sections -fvisibility=hidden -fvisibility-inlines-hidden -flto -s -fno-ident -fno-asynchronous-unwind-tables
LDFLAGS = -Wl,--gc-sections -Bsymbolic -Wl,--exclude-libs,ALL
INCLUDES = -I headers
SRC = src/main.cpp src/server.cpp src/client.cpp src/router.cpp
OBJ = $(SRC:.cpp=.o)
EXECUTABLE = bin/server

# Rules
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $(EXECUTABLE)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXECUTABLE)
