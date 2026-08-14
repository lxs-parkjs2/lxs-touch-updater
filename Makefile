# Makefile for LXS Touch Firmware Updater

CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -O2
LDFLAGS ?=

# Source files
SOURCES = main.cpp CLXSFwupdate.cpp CInterface.cpp
HEADERS = CLXSFwupdate.h CInterface.h common_define.h

# Output binary
TARGET = lxs_touch_updater

# Default build
all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)/usr/sbin/$(TARGET)

.PHONY: all clean install
