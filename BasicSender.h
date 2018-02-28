#include <cstdint>
#include <functional>
#include <iostream>

template<typename MessageClass>
class BasicSender {
        size_t packet_size, mss;
        std::uint32_t sequence;
public:
        BasicSender(size_t mss) : packet_size(mss), mss(mss - sizeof(MessageClass::Header)), sequence(0) {}

        std::function<void(void)> timeout_handler, send_eof_handler, completed_handler;

        std::function<void(int, int)> corrupt_ack_handler, send_data_handler;
        std::function<void(int)> recv_ack_handler;

        inline bool BasicSender::is_valid(const MessageClass &message) {
                return message.is_valid() &&
                       (messsage.get_sequence() >= sequence) &&
                       (message.is_ack() == true);
        }

        MessageClass extract_packet(std::istream &stream) {
                const bool ack = false, eof = false;
                BasicMessage message(ack, eof, sequence, this->mss);

                auto data_len = stream.read(message.get_data(), this->mss);
                message.set_length(stream.gcount());

                return message;
        }

        void send_eof(void) {
                MessageClass eof_message;
                eof_message.set_eof(true);
                eof_message.set_sequence(this->sequence++);

                std::vector<std::uint8_t> data(((std::uint8_t*) myheader),
                                               ((std::uint8_t*) myheader) + sizeof(header));
                sock.sendto(peer_addr, data);

                this->eof_handler();
        }



        void transmit(std::istream &stream, UDP_Socket &sock) {
                while (!stream.eof()) {
                        auto data_message = this->extract_packet(stream);
                        // EOF is thrown when EOF is read, so possible for last read
                        // to have read just up to the last availible byte (but EOF
                        // wasn't read)
                        if (data_message.get_length() == 0) {
                                break;
                        }
                        std::vector<std::uint8_t> data(message.data, message.data + sizeof(BasicMessage::Header)+message.get_length());
                        sock.sendto(peer_addr, data);
                        this->data_handler(this->sequence, message.get_length());

                        while (true) {
                                std::vector<std::uint8_t> buf;
                                buf.reserve(this->packet_size);
                                try {
                                        mySocket.recvfrom(buf);
                                }
                                catch (TimeoutException &e) {
                                      	this->timeout_handler();
                                }

                                MessageClass recv_message(buf.data());

                                if (!this->is_valid(recv_message)) {
                                        this->corrupt_ack_handler(header->magic, sequence);
                                }
                                else {
                                        this->ack_handler(myheader->sequence);
                                        sequence = myheader->sequence;
                                        done = true;

                                        break;
                                }
                        }
                }
                this->send_eof();

                this->completed_handler();
        }

};
