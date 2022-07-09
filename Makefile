CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -O0 -Isrc/
CFLAGS += $(shell pkg-config --cflags glfw3)
LDFLAGS = $(shell pkg-config glfw3 --libs)

RENDERER_OBJS = src/renderer/renderer.o
DIRECTOR_OBJS = src/director/director.o src/director/imap.o src/director/mmap.o src/director/keys.o src/director/config.o\
				src/director/rect.o src/director/util.o
GLAD_OBJS = src/glad/glad.o
SWAVE_OBJS = src/main.o

EXECNAME = swave

# Rules
all: swave
	@echo "Build complete."
	
swave: director renderer $(SWAVE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SWAVE_OBJS) $(DIRECTOR_OBJS) $(RENDERER_OBJS) $(GLAD_OBJS) -o $(EXECNAME)
	@echo "SWave built."

director: $(DIRECTOR_OBJS)
	@echo "SWave director module built."

renderer: $(RENDERER_OBJS) $(GLAD_OBJS)
	@echo "SWave renderer module built."

.PHONY: clean
clean:
	rm -f $(EXECNAME) $(DIRECTOR_OBJS) $(RENDERER_OBJS) $(GLAD_OBJS) $(SWAVE_OBJS)