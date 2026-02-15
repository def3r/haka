# include ../config.mk

.PHONY: all clean

CFLAGS_SO := -fPIC -Wall -Wextra -I../include/ -shared
SUBDIRS := $(wildcard */)
SRC := $(wildcard *.c)
SO  := $(SRC:.c=.so)
LOG_LEVEL := -DLOG=2

all: $(SO) subdirs

%.so: %.c
	$(CC) $(LOG_LEVEL) $(CFLAGS_SO) $< -o $@ $(LDFLAGS_SO)

subdirs:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d; \
	done

clean:
	rm -f *.so *.o
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d clean; \
	done

