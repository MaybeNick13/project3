CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -g

SRCS = main.cpp GNSS.cpp lshgraph.cpp mrng.cpp utils.cpp hashing.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = main

$(TARGET): $(OBJS) hashing.h utils.h
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

lsh: lsh.o utils.o hashing.o hashing.h utils.h
	$(CXX) $(CXXFLAGS) lsh.o utils.o hashing.o -o lsh

%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) lsh.o lsh
