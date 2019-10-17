#!/usr/bin/make -f

target := chfs
srcs := $(wildcard src/*.cpp)
objs := $(srcs:%.cpp=%.o)
CXXFLAGS := -std=c++17 -I./picojson

.PHONY: all
all: $(target)

$(target): $(objs)
	$(CXX) -o $@ $^

$(objs): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	$(RM) $(target) $(objs)
