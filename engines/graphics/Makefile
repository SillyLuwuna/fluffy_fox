CPPC = g++
CFLAGS = -std=c++17 -O2
# CFLAGS = -std=c++17 -O2 -DNDEBUG
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
SRCFILES = $(wildcard src/*.cpp)

build/fluffy_graphics_engine: $(SRCFILES)
	$(CPPC) $(CFLAGS) -o build/fluffy_graphics_engine $(SRCFILES) $(LDFLAGS) -I include

.PHONY: run clean

run: build/fluffy_graphics_engine
	./compile_shaders.sh
	./build/fluffy_graphics_engine

clean:
	rm -f build/fluffy_graphics_engine
