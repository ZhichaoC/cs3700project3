#ifndef BASIC_RECEIVER_H_
#define BASIC_RECEIVER_H_

#include <cstdint>
#include <functional>
#include <iostream>

#include "3700sendrecv.h"

template<typename MessageClass>
class BasicReciever {
        size_t packet_size, mss;
        std::uint32_t sequence;
	bool eof;
	UDP_Socket sock;
	UDP_Address peer_address;
public:
        BasicReciever(size_t mss, UDP_Socket&& sock)
		: packet_size(mss), mss(mss - sizeof(typename MessageClass::Header)), sequence(0), sock(std::move(sock)) {}

	bool is_valid(const MessageClass &message) {
                return message.is_valid() &&
                       message.get_sequence() >= this->sequence;
        }
	bool is_stale(const MessageClass &message) {
                return message.is_valid() &&
                       message.get_sequence() < this->sequence;
        }

	void send_ack(void) {
		BasicMessage ack_message(true, this->eof, this->sequence, 0);
		this->sock.sendto(this->peer_address, ack_message.data,
				  ack_message.get_total_length());
		mylog("[send ack] %d\n", this->sequence); 
	}

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

	MessageClass await_message(void) {
		MessageClass message;
		while(1) {
			try {
				message = std::move(this->recieve_message());
				break;
			}
			catch(TimeoutException &e) {
				mylog("[error] timeout occurred\n"); 
				if (!(this->sequence == 0)) {
					this->send_ack();
				}
			}
		}
		return message;
	}

	void recieve(std::ostream &stream) {
		while (!this->eof) {
			MessageClass data_message = await_message();

			if (!data_message.is_valid()) {
				mylog("[recv corrupted packet] magic=%x, seq=%d(expected >=: %d)\n",
				      data_message.get_magic(), data_message.get_sequence(), this->sequence);
			}
			else if (data_message.get_sequence() < this->sequence) {
				//this->send_ack();
			}
			else {
				this->eof = data_message.is_eof();
				this->sequence = data_message.get_sequence() + data_message.get_length();
				stream.write((char*)data_message.get_data(), data_message.get_length());

				mylog("[recv data] %d (%d) %s\n", data_message.get_sequence(),
				      data_message.get_length(), "ACCEPTED (in-order)");

				this->send_ack();
			}
		}

		mylog("[recv eof]\n");

		// If we still can recieve messages, sender didn't recieve EOF-ACK, keep
		// resending to them
		unsigned int timeout_counter = 0;	
		while(timeout_counter < 3) {
			try {
				this->recieve_message();
				this->send_ack();
				timeout_counter = 0;
			}
			catch(TimeoutException &e) {
				timeout_counter++;
			}
		}

		mylog("[completed]\n"); 
	}
};

#endif //BASIC_RECIEVER_H_
