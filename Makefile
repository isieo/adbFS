CXXFLAGS=$(shell pkg-config fuse --cflags)
LDFLAGS=$(shell pkg-config fuse --libs)

TARGET=adbfs

all:	$(TARGET)

adbfs.o: adbfs.cpp utils.h
	$(CXX) -c -o adbfs.o adbfs.cpp $(CXXFLAGS) ${CPPFLAGS}

$(TARGET): adbfs.o
	$(CXX) -o $(TARGET) adbfs.o $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o html/ latex/ $(TARGET)

doc:
	doxygen Doxyfile
