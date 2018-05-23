.PHONY: build rebuild clean docs

CXXFLAGS=-Wall -Werror -Iinclude -g -std=c++11
CXX=clang++

SRC_FILES=$(shell find src/ -name "*.cpp")
OBJ_FILES=$(patsubst %.cpp, %.o, $(SRC_FILES))
TARGET=sonometer-datapoller

build: $(OBJ_FILES)
	$(CXX) -o $(TARGET) $^ $(CXXFLAGS) -rdynamic -lsqlite3 -lwiringPi

rebuild: clean build

clean:
	rm -rf $(OBJ_FILES) $(TARGET)

docs:
	doxygen doxygen_config
