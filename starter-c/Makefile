SEND = 3700send
RECV = 3700recv
SENDRECV = 3700sendrecv

all: $(SEND) $(RECV)

$(SENDRECV).o: $(SENDRECV).c
	g++ -c -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $<

$(SEND): $(SEND).c $(SENDRECV).o
	g++ -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $< $(SENDRECV).o

$(RECV): $(RECV).c $(SENDRECV).o
	g++ -std=c++11 -O0 -g -lm -Wall -pedantic -Wextra -o $@ $< $(SENDRECV).o

test: all
	./test

clean:
	rm $(SEND) $(RECV) $(SENDRECV).o

