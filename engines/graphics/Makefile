CPPC = g++
CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
SRCFILES = $(wildcard src/*.cpp)
INCLUDE = $(wildcard include/*.cpp)

build/fluffy_graphics_engine: $(SRCFILES)
	$(CPPC) $(CFLAGS) -o build/fluffy_graphics_engine $(SRCFILES) -I $(INCLUDE) $(LDFLAGS)

.PHONY: run clean

run: build/fluffy_graphics_engine
	./build/fluffy_graphics_engine

clean:
	rm -f build/fluffy_graphics_engine
