all: build

build: lc.o
	$(CC) $(CFLAGS) -o lc lc.o
