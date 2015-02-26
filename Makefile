check: run

run: test.cc safe-printf.hh
	$(CXX) -std=c++11 -o run test.cc
