include config.mk

# === Configurable Variables ===
LIBEVDEV_VERSION := 1.12.1
LIBEVDEV_DIR := libevdev-$(LIBEVDEV_VERSION)
LIBEVDEV_TAR := $(LIBEVDEV_DIR).tar.xz
LIBEVDEV_URL := https://www.freedesktop.org/software/libevdev/$(LIBEVDEV_TAR)
LDLIBS := ./$(LIBEVDEV_DIR)/libevdev/.libs/libevdev.a
CC := gcc
INCDIR := $(CURDIR)/include
CFLAGS := -I$(INCDIR) -I./libevdev-$(LIBEVDEV_VERSION)
CFLAGS_SO := -fPIC -Wall -Wextra -Wpedantic -shared -I$(INCDIR)
SRC := $(wildcard src/*.c)
OUT := haka.out
# 1 for Info logs, 2 for Debug logs
LOG_LEVEL := -DLOG=2

.PHONY: all clean plugins

# === Targets ===
all: $(OUT) plugins

plugins:
	$(MAKE) -C plugins CFLAGS_SO="$(CFLAGS_SO)"

$(OUT): $(SRC) $(LDLIBS)
	$(CC) $(LOG_LEVEL) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(LDLIBS): $(LIBEVDEV_TAR)
	tar -xf $(LIBEVDEV_TAR)
	cd $(LIBEVDEV_DIR) && ./configure --enable-static --disable-shared
	$(MAKE) -C $(LIBEVDEV_DIR)

$(LIBEVDEV_TAR):
	wget $(LIBEVDEV_URL)

clean:
	rm -f $(OUT)
	rm -rf prevFile.txt
	rm -rf $(LIBEVDEV_DIR)
