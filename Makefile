# Makefile for LXS Touch Firmware Updater
# This can be used for standalone builds outside of ChromeOS

CXX     = g++
CXX_ARM = aarch64-linux-gnu-g++
CXXFLAGS = -Wall -Wextra -O2
LDFLAGS  = -static-libstdc++ -static-libgcc

# Source files
SOURCES = main.cpp CLXSFwupdate.cpp CInterface.cpp
HEADERS = CLXSFwupdate.h CInterface.h common_define.h

# Output binaries
TARGET     = lxs_touch_updater_x64
TARGET_ARM = lxs_touch_updater_arm

# Default: x86_64 build
all: $(TARGET)

# ARM64 (aarch64) build
# Requires: sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
arm: $(TARGET_ARM)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Binary: $(TARGET)"

$(TARGET_ARM): $(SOURCES) $(HEADERS)
	@command -v $(CXX_ARM) >/dev/null 2>&1 || \
		{ echo "ARM64 cross-compiler not found. Install with:"; \
		  echo "  sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"; \
		  exit 1; }
	$(CXX_ARM) $(CXXFLAGS) $(SOURCES) -o $(TARGET_ARM) $(LDFLAGS)
	@echo "ARM64 build successful! Binary: $(TARGET_ARM)"

clean:
	rm -f $(TARGET) $(TARGET_ARM) *.o

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)/usr/sbin/$(TARGET)
	install -D -m 644 chromeos/lxs-touch-updater.policy $(DESTDIR)/usr/share/policy/lxs-touch-updater.policy

test: $(TARGET)
	@echo "Testing version check speed..."
	@time ./$(TARGET) --get_current_version || true
	@echo ""
	@echo "Testing help..."
	./$(TARGET) --help

.PHONY: all arm clean install test
