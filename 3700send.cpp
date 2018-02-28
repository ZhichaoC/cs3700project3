/*
 * CS3700, Spring 2015
 * Project 2 Starter Code
 */
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
        mySocket.set_timeout(30, 0);

        UDP_Address peer_addr(ip_s, atoi(port_s));

        BasicSender<BasicMessage> sender;
        sender.timeout_handler = []{ mylog("[error] timeout occurred\n"); };
        sender.corrupt_ack_handler = [](int magic, int sequence) {
                mylog("[recv corrupted ack] %x %d\n", magic, sequence);
        };
        sender.completed_handler = []{ mylog("[completed]\n"); };
        sender.ack_handler = [](int sequence) {
                mylog("[recv ack] %d\n", sequence);
        };
        sender.eof_handler = []{ mylog("[send eof]\n"); };
        sender.data_handler = [](int sequence, int data_len) {
                mylog("[send data] %d (%d)\n", sequence, data_len);
        };
	sender.transmit(std::cin, mySocket);


        return 0;
}
