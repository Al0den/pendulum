CXX = g++
CXXFLAGS = -std=c++17 -Wall 
LDFLAGS = -lSDL2 -lSDL2_ttf

TARGET = Pendulum
THIRD-PARTY = third-party

SRCS = $(wildcard src/*.cpp) $(shell find third-party -name "*.cpp")
BEST_SRC = mains/best.cpp
DEBUG_SRC = mains/debug.cpp
TEST_SRC = mains/test.cpp
TRAIN_SRC = mains/train.cpp

OBJS = $(SRCS:.cpp=.o)
BEST_OBJS = $(BEST_SRC:.cpp=.o)
DEBUG_OBJS = $(DEBUG_SRC:.cpp=.o)
TEST_OBJS = $(TEST_SRC:.cpp=.o)
TRAIN_OBJS = $(TRAIN_SRC:.cpp=.o)

all: $(THIRD-PARTY) $(TARGET)
	make $(TARGET)

test: $(TEST)
	$(TEST)


best: $(BEST_OBJS) $(OBJS)
	g++ $(BEST_OBJS) $(OBJS) $(CXXFLAGS) -o ./build/best.out $(LDFLAGS)
	./build/best.out

debug: $(DEBUG_OBJS) $(OBJS)
	g++ $(DEBUG_OBJS) $(OBJS) $(CXXFLAGS) -o ./build/debug.out $(LDFLAGS)
	./build/debug.out

test: $(TEST_OBJS) $(OBJS)
	g++ $(TEST_OBJS) $(OBJS) $(CXXFLAGS) -o ./build/test.out $(LDFLAGS)
	./build/test.out

train: $(TRAIN_OBJS) $(OBJS)
	g++ $(TRAIN_OBJS) $(OBJS) $(CXXFLAGS) -o ./build/train.out $(LDFLAGS)
	./build/train.out

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
	rm -rf $(OBJS) $(TARGET) ./build/*.out $(THIRD-PARTY)

third-party:
	mkdir third-party
	git clone https://github.com/Al0den/physics third-party/physics
	git clone https://github.com/Al0den/neat third-party/neat
	git clone https://github.com/Al0den/RenderEngine third-party/RenderEngine


