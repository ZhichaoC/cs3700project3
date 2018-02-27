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

static int DATA_SIZE = 1460;

unsigned int sequence = 0;

/**
 * Reads the next block of data from stdin
 */
int get_next_data(char *data, int size) {
  return read(0, data, size);
}

/**
 * Builds and returns the next packet, or NULL
 * if no more data is available.
 */
void *get_next_packet(int sequence, int *len) {
  char *data = (char *) malloc(DATA_SIZE);
  int data_len = get_next_data(data, DATA_SIZE);

  if (data_len == 0) {
    free(data);
    return NULL;
  }

  header *myheader = make_header(sequence, data_len, 0, 0);
  void *packet = malloc(sizeof(header) + data_len);
  memcpy(packet, myheader, sizeof(header));
  memcpy(((char *) packet) +sizeof(header), data, data_len);

  free(data);
  free(myheader);

  *len = sizeof(header) + data_len;

  return packet;
}

int send_next_packet(UDP_Socket &sock, UDP_Address peer_addr) {
        int packet_len = 0;
        std::uint8_t *packet = (std::uint8_t *) get_next_packet(sequence, &packet_len);

        if (packet == NULL)
                return 0;

        mylog("[send data] %d (%d)\n", sequence, packet_len - sizeof(header));

        try {
                std::vector<std::uint8_t> data(packet, packet+packet_len);
                sock.sendto(peer_addr, data);
        } catch (...) {
                perror("sendto");
                exit(1);
        }

        return 1;
}

void send_final_packet(UDP_Socket &sock, UDP_Address peer_addr) {
  header *myheader = make_header(sequence+1, 0, 1, 0);
  mylog("[send eof]\n");

  try {
	  std::vector<std::uint8_t> data(((std::uint8_t*) myheader), ((std::uint8_t*) myheader) + sizeof(header));
          sock.sendto(peer_addr, data);
  } catch (...) {
          perror("sendto");
          exit(1);
  }
}

int main(int argc, char *argv[]) {
        char *tmp = (char *) malloc(strlen(argv[1])+1);
        strcpy(tmp, argv[1]);

        char *ip_s = strtok(tmp, ":");
        char *port_s = strtok(NULL, ":");

        UDP_Socket mySocket;
        mySocket.bind();
        mySocket.set_timeout(30, 0);

        UDP_Address peer_addr(ip_s, atoi(port_s));

        while (send_next_packet(mySocket, peer_addr)) {
                int done = 0;

                while (! done) {
                        std::vector<std::uint8_t> buf;
                        buf.reserve(10000);
                        try {
                                mySocket.recvfrom(buf);
                        }
                        catch (...) {
                              	mylog("[error] timeout occurred\n");
                                // OR
				perror("recvfrom");
                                exit(1);
                        }

                        header *myheader = get_header(buf.data());

                        if ((myheader->magic == MAGIC) && (myheader->sequence >= sequence) && (myheader->ack == 1)) {
                                mylog("[recv ack] %d\n", myheader->sequence);
                                sequence = myheader->sequence;
                                done = 1;
                        }
                        else {
                                mylog("[recv corrupted ack] %x %d\n", MAGIC, sequence);
                        }
                }
        }

        send_final_packet(mySocket, peer_addr);

        mylog("[completed]\n");

        return 0;
}
