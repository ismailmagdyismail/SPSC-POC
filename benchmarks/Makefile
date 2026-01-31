SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := main.exe

GOOGLE_BENCHMARK_DIR := $(GOOGLE_BENCHMARK_DIR)/google-benchmark

INCLUDE_FLAGS += -I. \
-I ../SpscQueue_Poc \
-I$(GOOGLE_BENCHMARK_DIR)/include \


LD_FLAGS := -lpthread  -L$(GOOGLE_BENCHMARK_DIR)/build/src/ -lbenchmark

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ $(OBJS) -o $(TARGET) $(LD_FLAGS)

%.o: %.cpp
	g++ -Wall -Wextra -g -O3 -std=c++17 -MMD -MP -c $< -o $@ $(INCLUDE_FLAGS)

clean:
	rm -rf *.a *.o *.d $(TARGET)
