SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := main.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ $(OBJS) -o $(TARGET) -lpthread

%.o: %.cpp
	g++ -Wall -Wextra -g -O3 -std=c++17 -MMD -MP -c $< -o $@

clean:
	rm -rf *.a *.o *.d $(TARGET)
