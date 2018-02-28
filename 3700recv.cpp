/*
 * CS3700, Spring 2015
 * Project 2 Starter Code
 */

#include "3700sendrecv.h"
#include "UDP_Socket.h"
#include "BasicMessage.h"
#include "BasicSender.h"

int main() {
	UDP_Socket mySocket;
	mySocket.bind();  
	mySocket.set_timeout(30, 0);

  	mylog("[bound] %d\n", mySocket.local_address.port);

	while (1) {
		try {
			std::int64_t buffer_len = 1460;
			std::uint8_t* buffer = (std::uint8_t*)malloc(buffer_len);
			auto peer_address = mySocket.recvfrom(buffer, &buffer_len);
			BasicMessage data_message(buffer);
  
			if (data_message.is_valid()) {
				write(1, data_message.get_data(), data_message.get_length());

				mylog("[recv data] %d (%d) %s\n", data_message.get_sequence(), data_message.get_length(), "ACCEPTED (in-order)");
				mylog("[send ack] %d\n", data_message.get_sequence() + data_message.get_length());

				BasicMessage ack_message(true, data_message.is_eof(), data_message.get_sequence()+data_message.get_length(), 0);
				
				mySocket.sendto(peer_address, ack_message.data, ack_message.get_total_length());

				if (ack_message.is_eof()) {
					mylog("[recv eof]\n");
					mylog("[completed]\n");
					exit(0);
				}
			} else {
				mylog("[recv corrupted packet]\n");
			}
		}
		catch(TimeoutException &e) {
			mylog("[error] timeout occurred\n");
			exit(1);
		}
	}

	return 0;
}
