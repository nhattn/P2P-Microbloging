CC = g++
CFLAGS = -c -O3 -std=c++11 -Wall -I. -ldl
LDFLAGS = -I. -ldl
SOURCES = sha256.cpp block.cpp blockchain.cpp blockchain_test.cpp
OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS := $(addprefix objects/,$(OBJECTS))
EXECUTABLE = blockchain

all: objects $(SOURCES) $(EXECUTABLE)

objects:
	@mkdir -p objects

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

objects/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -rf objects/*.o $(EXECUTABLE)
