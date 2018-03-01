#include "3700sendrecv.h"
#include "UDP_Socket.h"
#include "BasicMessage.h"
#include "BasicReciever.h"

int main() {
	UDP_Socket mySocket;
	mySocket.bind();  
	mySocket.set_timeout(2, 0);

  	mylog("[bound] %d\n", mySocket.local_address.port);
	BasicReciever<BasicMessage> reciever(1460, std::move(mySocket));
	reciever.recieve(std::cout);

	return 0;
}
