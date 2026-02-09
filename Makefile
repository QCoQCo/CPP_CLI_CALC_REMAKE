CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = calculator
SOURCE = main.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: clean
