CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
LDFLAGS ?=

INCDIR  ?= $(CURDIR)/include
CFLAGS_SO  := -fPIC -Wall -Wextra -Wpedantic -shared -I$(INCDIR)
