CXX = g++
CXXFLAGS = -Wall -W -O3
LDFLAGS = -static
SRCS = src/ai.cc
OBJS = src/ai.o
TARGET = lifter
DIST = w8-8f.tgz
README = README

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS)

clean:
	$(RM) $(OBJS) $(TARGET)

dist: $(DIST)

$(DIST): $(TARGET)
	tar zcf $(DIST) $(SRCS) $(TARGET) $(README)
