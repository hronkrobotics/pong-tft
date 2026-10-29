CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -I../firmware/Pong

.PHONY: test clean

test: test_game
	./test_game

test_game: test_game.cpp ../firmware/Pong/game.cpp ../firmware/Pong/game.h ../firmware/Pong/config.h
	$(CXX) $(CXXFLAGS) -o test_game test_game.cpp ../firmware/Pong/game.cpp

clean:
	rm -f test_game
