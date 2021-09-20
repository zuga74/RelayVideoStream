CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lpthread
SOURCES=main.cpp more.cpp ringbuf.cpp reader.cpp acceptor.cpp
# BUILD_DIR=build
OBJECTS=$(SOURCES:.cpp=.o)
# OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.cpp=.o)))
EXECUTABLE=relay2


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
#	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@

clean:
	rm -rf *.o
	rm -rf $(EXECUTABLE)

