#include "3700sendrecv.h"
#include "UDP_Socket.h"
#include "BasicMessage.h"
#include "BasicReciever.h"

int main() {
	UDP_Socket mySocket;
	mySocket.bind();  
	mySocket.set_timeout(30, 0);

  	mylog("[bound] %d\n", mySocket.local_address.port);

	BasicReciever<BasicMessage> reciever(1460, std::move(mySocket));
	reciever.recv_data_handler = [](int sequence, int length)
		{ mylog("[recv data] %d (%d) %s\n", sequence, length, "ACCEPTED (in-order)"); };
	reciever.sent_ack_handler = [](int sequence)
		{ mylog("[send ack] %d\n", sequence); };
	reciever.recv_eof_handler = []{ mylog("[recv eof]\n"); };
	reciever.completed_handler = []{ mylog("[completed]\n"); };
	reciever.corrupt_handler = []{ mylog("[recv corrupted packet]\n"); };
	reciever.timeout_handler = []{ mylog("[error] timeout occurred\n"); };
	reciever.recieve(std::cout);

	return 0;
}
