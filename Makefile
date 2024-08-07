CXX = g++
CXXFLAGS = -std=c++17 -Wall 
LDFLAGS = -lrenderer -lSDL2 -lSDL2_ttf

TARGET = Pendulum
TEST = ./build/test.out
TRAIN = ./build/train.out
DEBUG = ./build/debug.out

SRCS = $(wildcard src/*.cpp) $(shell find third-party -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)

build:
	git clone https://github.com/Al0den/physics third-party/physics
	git clone https://github.com/Al0den/neat third-party/neat

all: $(TARGET)
test: $(TEST)
	$(TEST)
train: $(TRAIN)
	$(TRAIN)
debug: $(DEBUG)
	$(DEBUG)

$(TARGET): $(OBJS)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) mains/test.cpp $(LDFLAGS) -o ./build/test.out

$(TRAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) mains/train.cpp $(LDFLAGS) -o ./build/train.out

$(DEBUG): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) mains/debug.cpp $(LDFLAGS) -o ./build/debug.out

train_debug: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) mains/train.cpp $(LDFLAGS) -o ./build/train.out

clean:
	rm -rf $(OBJS) $(TARGET) ./build/*.out

	
