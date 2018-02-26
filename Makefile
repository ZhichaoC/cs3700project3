SEND = 3700send
RECV = 3700recv
SENDRECV = 3700sendrecv
SOCK = UDP_Socket

all: $(SEND) $(RECV)

$(SOCK).o: $(SOCK).cpp
	g++ -c -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $<

$(SENDRECV).o: $(SENDRECV).c
	g++ -c -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $<

$(SEND): $(SEND).cpp $(SENDRECV).o $(SOCK).o
	g++ -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $< $(SENDRECV).o $(SOCK).o

$(RECV): $(RECV).c $(SENDRECV).o $(SOCK).o
	g++ -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $< $(SENDRECV).o $(SOCK).o

test: all
	./test

clean:
	rm $(SEND) $(RECV) $(SENDRECV).o $(SOCK).o

