CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude

all: velorent

velorent: src/main.cpp include/*.h
	$(CXX) $(CXXFLAGS) src/main.cpp -o velorent

tests/run_tests: tests/tests.cpp include/*.h
	$(CXX) $(CXXFLAGS) tests/tests.cpp -o tests/run_tests

test: tests/run_tests
	./tests/run_tests

clean:
	rm -f velorent tests/run_tests ledger.bin test_ledger.bin

.PHONY: all test clean
