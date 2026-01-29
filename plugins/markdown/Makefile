SRC := $(wildcard *.c)
OUT := plugin.so
CFLAGS_SO := -fPIC -Wall -Wextra -I../../include/ -shared

$(OUT): $(SRC)
	$(CC) $(CFLAGS_SO) -o $@ $^ $(LDFLAGS_SO)

clean:
	rm -f plugin.so

