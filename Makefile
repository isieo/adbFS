CXXFLAGS=-Wall -D_FILE_OFFSET_BITS=64
LDFLAGS=-lfuse

TARGET=adbfs

all:	$(TARGET)

.PHONY: clean
clean:
	rm -rf $(TARGET)
