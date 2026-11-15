CXX      = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic -Iinclude

all: velorent velorent_api

velorent: src/main.cpp include/*.h
	$(CXX) $(CXXFLAGS) src/main.cpp -o velorent

velorent_api: src/api_main.cpp include/*.h
	$(CXX) $(CXXFLAGS) src/api_main.cpp -o velorent_api

api: velorent_api

tests/run_tests: tests/tests.cpp include/*.h
	$(CXX) $(CXXFLAGS) tests/tests.cpp -o tests/run_tests

test: tests/run_tests
	./tests/run_tests

clean:
	rm -f velorent velorent.exe velorent_api velorent_api.exe tests/run_tests tests/run_tests.exe ledger.bin test_ledger.bin

.PHONY: all api test clean
