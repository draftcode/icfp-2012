OBJS = main.o game.o
TARGET = lifter

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ -g

clean:
	rm -f $(OBJS) $(TARGET) simulator.o simulator

simulator: game.o simulator.o
	$(CXX) -o simulator $^
