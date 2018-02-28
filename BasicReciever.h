#ifndef BASIC_RECEIVER_H_
#define BASIC_RECEIVER_H_

#include <cstdint>
#include <functional>
#include <iostream>

template<typename MessageClass>
class BasicReciever {
        size_t packet_size, mss;
        std::uint32_t sequence;
	UDP_Socket sock;
public:
        BasicReciever(size_t mss, UDP_Socket&& sock)
		: packet_size(mss), mss(mss - sizeof(typename MessageClass::Header)), sequence(0), sock(std::move(sock)) {}

        std::function<void(void)> corrupt_handler, timeout_handler, recv_eof_handler, completed_handler;
        std::function<void(int)> sent_ack_handler;
        std::function<void(int, int)> recv_data_handler;

	void recieve(std::ostream &stream) {
		while (1) {
			try {
				std::int64_t buffer_len = packet_size;
				std::uint8_t* buffer = (std::uint8_t*)malloc(buffer_len);
				auto peer_address = this->sock.recvfrom(buffer, &buffer_len);
				BasicMessage data_message(buffer);
	  
				if (data_message.is_valid()) {
					stream.write((char*)data_message.get_data(), data_message.get_length());

					this->recv_data_handler(data_message.get_sequence(), data_message.get_length());
					this->sent_ack_handler(data_message.get_sequence() + data_message.get_length());
	
					BasicMessage ack_message(true, data_message.is_eof(), data_message.get_sequence()+data_message.get_length(), 0);
					
					this->sock.sendto(peer_address, ack_message.data, ack_message.get_total_length());
	
					if (ack_message.is_eof()) {
						this->recv_eof_handler();
						this->completed_handler();
						break;
					}
				} else {
					this->corrupt_handler();
				}
			}
			catch(TimeoutException &e) {
				this->timeout_handler();
				break;
			}
		}
	}
};

#endif //BASIC_RECIEVER_H_
