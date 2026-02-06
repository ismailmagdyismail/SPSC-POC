SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := main.exe

INCLUDES = -I. \
-I../SpscQueue_Poc

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ $(OBJS) -o $(TARGET) -lpthread

%.o: %.cpp
	g++ -std=c++17 -Wall -Wextra -g -O0 -std=c++17 -MMD -MP -c $< -o $@ $(INCLUDES)

clean:
	rm -rf *.a *.o *.d $(TARGET)
