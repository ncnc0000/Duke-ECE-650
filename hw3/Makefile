SOURCES=potato.h
OBJS=$(patsubst %.cpp, %.o, $(SOURCES))
CPPFLAGS=-ggdb3 -std=gnu++98 -Wall -Werror -pedantic

all: player ringmaster

player: $(OBJS)
	g++ $(CPPFLAGS) -o player $(SOURCES) player.cpp

ringmaster: $(OBJS)
	g++ $(CPPFLAGS) -o ringmaster $(SOURCES) ringmaster.cpp

clean:
	rm -f *~ *.o player ringmaster
