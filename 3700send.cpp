#include <iostream>

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "3700sendrecv.h"
#include "UDP_Socket.h"
#include "BasicMessage.h"
#include "BasicSender.h"


int main(int argc, char *argv[]) {
        char *tmp = (char *) malloc(strlen(argv[1])+1);
        strcpy(tmp, argv[1]);

        char *ip_s = strtok(tmp, ":");
        char *port_s = strtok(NULL, ":");

        UDP_Socket mySocket;
        mySocket.bind();
        mySocket.set_timeout(2/*sec*/, 0 /*usec*/);

        UDP_Address peer_addr(ip_s, atoi(port_s));

        BasicSender<BasicMessage> sender(1460, peer_addr, std::move(mySocket));

        sender.timeout_handler = []
		{ mylog("[error] timeout occurred\n"); };
        sender.corrupt_ack_handler = [](int magic, int sequence)
		{ mylog("[recv corrupted ack] %x %d\n", magic, sequence); };
        sender.completed_handler = []
		{ mylog("[completed]\n"); };
        sender.recv_ack_handler = [](int sequence)
		{ mylog("[recv ack] %d\n", sequence); };
        sender.send_eof_handler = []
		{ mylog("[send eof]\n"); };
        sender.send_data_handler = [](int sequence, int data_len)
		{ mylog("[send data] %d (%d)\n", sequence, data_len); };

	sender.transmit(std::cin);

        return 0;
}
