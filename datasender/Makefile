.PHONY: build rebuild clean docs

CXXFLAGS=-Wall -Iinclude -Ijsoncpp/dist -g -std=c++11 # -Werror
CXX=clang++

SRC_FILES=$(shell find src/ jsoncpp/dist -name "*.cpp")
OBJ_FILES=$(patsubst %.cpp, %.o, $(SRC_FILES))
TARGET=datasender

build: $(OBJ_FILES)
	$(CXX) -o $(TARGET) $^ $(CXXFLAGS) -lsqlite3 -lcurl -rdynamic -pthread

rebuild: clean build

clean:
	rm -rf $(OBJ_FILES) $(TARGET)

docs:
	doxygen doxygen_config
