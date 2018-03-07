#include "3700sendrecv.h"
#include "UDP_Socket.h"
#include "BasicMessage.h"
#include "BasicReciever.h"

int main() {
  //create a socket, bind to the port, set time out
	UDP_Socket mySocket;
	mySocket.bind();  
	mySocket.set_timeout(30, 0);
 
  mylog("[bound] %d\n", mySocket.local_address.port);
  //initialize the receiver and start receiving
	BasicReciever<BasicMessage> reciever(1460, std::move(mySocket));
	reciever.recieve(std::cout);

	return 0;
}
