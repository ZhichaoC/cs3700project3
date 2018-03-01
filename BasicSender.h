#ifndef BASIC_SENDER_H_
#define BASIC_SENDER_H_

#include <cstdint>
#include <functional>
#include <iostream>

template<typename MessageClass>
class BasicSender {
        size_t packet_size, mss;
        std::uint32_t sequence;
	UDP_Socket sock;
	UDP_Address peer_addr;
public:
        BasicSender(size_t mss, UDP_Address peer_addr, UDP_Socket&& sock)
		: packet_size(mss), mss(mss - sizeof(typename MessageClass::Header)), sequence(0), sock(std::move(sock)), peer_addr(peer_addr) {}

        std::function<void(void)> timeout_handler, send_eof_handler, completed_handler;
        std::function<void(int)> recv_ack_handler;
        std::function<void(int, int)> corrupt_ack_handler, send_data_handler;

        inline bool is_valid(const MessageClass &message) {
                return message.is_valid() &&
                       (message.get_sequence() >= this->sequence) &&
                       (message.is_ack());
        }

        MessageClass extract_packet(std::istream &stream) {
                const bool ack = false, eof = false;
                BasicMessage message(ack, eof, sequence, this->mss);

                stream.read(((char*)message.get_data()), this->mss);
                message.set_length(stream.gcount());

                return message;
        }

        void send_eof(void) {
                MessageClass eof_message(false, true, this->sequence++, 0);
                this->sock.sendto(this->peer_addr, eof_message.data, eof_message.get_total_length());
                this->send_eof_handler();
        }

        void transmit(std::istream &stream) {
                while (!stream.eof()) {
                        auto data_message = this->extract_packet(stream);
                        // EOF is thrown when EOF is read, so possible for last read
                        // to have read just up to the last availible byte (but EOF
                        // wasn't read)
                        if (data_message.get_length() == 0) {
                                break;
                        }
                        this->sock.sendto(this->peer_addr, data_message.data, data_message.get_total_length());
                        this->send_data_handler(this->sequence, data_message.get_length());

                        while (true) {
				std::int64_t buffer_read = this->packet_size;
                                std::uint8_t *buffer = (std::uint8_t *) malloc(this->packet_size);
                                try {
                                        this->sock.recvfrom(buffer, &buffer_read);
                                }
                                catch (TimeoutException &e) {
                                      	this->timeout_handler();
                                        this->sock.sendto(this->peer_addr, data_message.data, data_message.get_total_length());
                                        this->send_data_handler(this->sequence, data_message.get_length());
                                        continue;
                                }

                                MessageClass recv_message(buffer);

                                if (!this->is_valid(recv_message)) {
                                        this->corrupt_ack_handler(recv_message.get_magic(), sequence);
                                }
                                else {
                                        this->recv_ack_handler(recv_message.get_sequence());
                                        sequence = recv_message.get_sequence();

                                        break;
                                }
                        }
                }
                this->send_eof();

                this->completed_handler();
        }

};

#endif//BASIC_SENDER_H_
