SRC := $(wildcard *.c)
OUT := plugin.so
CFLAGS_SO := -fPIC -Wall -Wextra -I../../include/ -shared
LOG_LEVEL := -DLOG=2

$(OUT): $(SRC)
	$(CC) $(LOG_LEVEL) $(CFLAGS_SO) -o $@ $^ $(LDFLAGS_SO)

clean:
	rm -f plugin.so

