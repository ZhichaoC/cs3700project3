#ifndef BASIC_RECEIVER_H_
#define BASIC_RECEIVER_H_

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <cstring>

#include "3700sendrecv.h"

template<typename MessageClass>
class BasicReciever {
        size_t packet_size, mss;
        std::uint32_t sequence;
  //receive eof yet?
	bool eof;
 //udp socket
	UDP_Socket sock;
 //UDP address to sent packet to
	UDP_Address peer_address;
   //using map to keep track of all packets
  std::map<std::uint32_t,std::uint8_t*> packets;
public:
        BasicReciever(size_t mss, UDP_Socket&& sock)
		: packet_size(mss), mss(mss - sizeof(typename MessageClass::Header)), sequence(0), sock(std::move(sock)) {}

  //is the given message valid
	bool is_valid(const MessageClass &message) {
                return message.is_valid() &&
                       message.get_sequence() >= this->sequence;
        }
  
  //send ack message
	void send_ack(void) {
		BasicMessage ack_message(true, this->eof, this->sequence, 0);
		this->sock.sendto(this->peer_address, ack_message.data,
				  ack_message.get_total_length());
		mylog("[send ack] %d\n", this->sequence); 
	}

  //receive the message
	MessageClass recieve_message(void) {
		std::int64_t buffer_len = this->packet_size;
		std::uint8_t* buffer = (std::uint8_t*)malloc(buffer_len);
		try {		
			this->peer_address = this->sock.recvfrom(buffer, &buffer_len);
		}
		catch(...) {
			free(buffer);
			throw;
		}
		return MessageClass(buffer);
	}

  //wait for the message
	MessageClass await_message(void) {
		MessageClass message;
		while(1) {
			try {
				message = std::move(this->recieve_message());
				break;
			}
			catch(TimeoutException &e) {
				mylog("[error] timeout occurred\n"); 
			}
		}
		return message;
	}

  //handles all the incoming messages
	void recieve(std::ostream &stream) {
     //an array to keep track of all messages
     std::uint8_t** messages;
     messages = new std::uint8_t*[1000];
     int i = 0;
     //while not receiving eof message
		while (!this->eof) {
      //wait for the message
			MessageClass data_message = await_message();
      //if the message is corrupted
			if (!data_message.is_valid()) {
				mylog("[recv corrupted packet] magic=%x, seq=%d(expected >=: %d)\n",
				      data_message.get_magic(), data_message.get_sequence(), this->sequence);
			}
			else {
        //check if it is eof message
				this->eof = data_message.is_eof();
        //update the sequence
				this->sequence = data_message.get_sequence() + data_message.get_length();
        //if is not duplicate packet, update to the map
        if(packets.find(data_message.get_sequence()) == packets.end() && !this->eof) {
          messages[i] = new std::uint8_t[data_message.get_total_length()];
          memcpy(messages[i], data_message.data, data_message.get_total_length());
          packets[data_message.get_sequence()] = messages[i];
          i++;
        }
        mylog("[recv data] %d (%d) %s\n", data_message.get_sequence(),
				      data_message.get_length(), "ACCEPTED (in-order)");
        //send ack message 20 times to make sure sender receive it
        BasicMessage ack_message(true, data_message.is_eof(), data_message.get_sequence(), 0);
        for(int k = 0; k < 20; k++) {
          this->sock.sendto(this->peer_address, ack_message.data,
          ack_message.get_total_length());
        }
		    mylog("[send ack] %d\n", this->sequence); 
			}
		}

		mylog("[recv eof]\n");
    //iterates through the map and print out the data
    std::map<std::uint32_t,std::uint8_t*>::iterator it;
    for(it = packets.begin(); it != packets.end(); it++) {
      MessageClass m(it->second);
      stream.write((char*)m.get_data(), m.get_length());
      stream.flush();
    }
		mylog("[completed]\n"); 
    //free the message
   free(messages);
	}
};

#endif //BASIC_RECIEVER_H_
