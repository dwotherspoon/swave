CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -O0 -Isrc/

OBJS =  src/main.o src/director/director.o src/director/imap.o src/director/mmap.o src/director/keys.o

EXECNAME = swave

# Rules
all: swave
	@echo "Build complete."
	
swave: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECNAME) $(OBJS)
	@echo "SWave built."

.PHONY: clean
clean:
	rm -f $(EXECNAME) $(OBJS)