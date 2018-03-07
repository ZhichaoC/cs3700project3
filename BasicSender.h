#ifndef BASIC_SENDER_H_
#define BASIC_SENDER_H_

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>

template<typename MessageClass>
class BasicSender {
        size_t packet_size, mss;
        //current sequence number
        std::uint32_t sequence;
  //UDP socket
	UDP_Socket sock;
 //UDP peer address
	UDP_Address peer_addr;
   //using map to keep track of sent packets
  std::map<std::uint32_t,std::uint8_t*> packets;
public:
        BasicSender(size_t mss, UDP_Address peer_addr, UDP_Socket&& sock)
		: packet_size(mss), mss(mss - sizeof(typename MessageClass::Header)), sequence(0), sock(std::move(sock)), peer_addr(peer_addr) {}

        //handlers to log different events
        std::function<void(void)> timeout_handler, send_eof_handler, completed_handler;
        std::function<void(int)> recv_ack_handler;
        std::function<void(int, int)> corrupt_ack_handler, send_data_handler;
        
        //is the given message valid
        inline bool is_valid(const MessageClass &message) {
                return message.is_valid() && (message.is_ack());
        }

        //creates packets from given user input
        MessageClass extract_packet(std::istream &stream) {
                const bool ack = false, eof = false;
        	      MessageClass message(ack, eof, this->sequence, this->mss);

                stream.read(((char*)message.get_data()), this->mss);
                message.set_length(stream.gcount());

                return message;
        }
      
        //transmit and receives packets
        void transmit(std::istream &stream) {
                //an array of data
                std::uint8_t** messages;
                messages = new std::uint8_t*[1000];
                int i = 0;
                while (!stream.eof()) {
                  int c = 0;
                  //send at miximum 50 packets without ack
                  for (int k = 0; k < 50; k++) {
                        //if the user input is empty, stop creating/sending packets
                        if (stream.eof()) {
                          break;
                        }
                        
                        MessageClass data_message(false, false, 0, 0);
                        data_message = std::move(this->extract_packet(stream));
                        
                        //send packets and update sequence number
                        this->sock.sendto(this->peer_addr, data_message.data, data_message.get_total_length());
                        this->send_data_handler(this->sequence, data_message.get_length());
                        this->sequence = this->sequence + data_message.get_total_length();
                        
                        //copy the data into the array
                        messages[i] = new std::uint8_t[data_message.get_total_length()];
                        memcpy(messages[i], data_message.data, data_message.get_total_length());
                        packets[data_message.get_sequence()] = messages[i];
                        
                        i++;
                        c++;
                  }
                  //ack the packets that were sent from above loop
                  for (int k = 0; k < c; k++) {    
                        //if no packets need to be acked, exit
                        if(packets.size() == 0) {
                          break;
                        }
                        std::int64_t buffer_read = this->packet_size;
                        std::uint8_t *buffer = (std::uint8_t *) malloc(this->packet_size);
                        try {
                                this->sock.recvfrom(buffer, &buffer_read);
                        }
                        catch (TimeoutException &e) {
                                //if timeout, iterate through the map and resend all the packets stored in the map
                                this->timeout_handler();
                                std::map<std::uint32_t,std::uint8_t*>::iterator it;
                                for(it = packets.begin(); it != packets.end(); it++) {
                                    MessageClass m(it->second);
                                    std::uint8_t* d = new std::uint8_t[m.get_total_length()];
                                    memcpy(d, it->second, m.get_total_length());
                                   this->sock.sendto(this->peer_addr, m.data, m.get_total_length());
                                   this->send_data_handler(it->first, m.get_length());
                                   it->second = d;
                                   }
                                continue;
                        }
                        //receive message
			                  MessageClass resp(buffer);
                        //if is invalid message
                			  if (!this->is_valid(resp)) {
				                  this->corrupt_ack_handler(resp.get_magic(), this->sequence);
                			  } else {
                          //erase the packet from the map if exist in the map
                         if (packets.find(resp.get_sequence()) != packets.end()){
                          packets.erase(resp.get_sequence());
                          this->recv_ack_handler(resp.get_sequence());
                          }
                        }
                }
    
              }
                //similar to congestion window, using AIMD
                int j = 2;
    
          		  while(packets.size() != 0) {
			                  std::int64_t buffer_read = this->packet_size;
                        std::uint8_t *buffer = (std::uint8_t *) malloc(this->packet_size);
                        try {
                                this->sock.recvfrom(buffer, &buffer_read);
                        }
                        catch (TimeoutException &e) {
                                //if time out, send all the packets j times, increase j by 1
                                this->timeout_handler();
               			            std::map<std::uint32_t,std::uint8_t*>::iterator it;
                                for(it = packets.begin(); it != packets.end(); it++) {
                                    MessageClass m(it->second);
                                    std::uint8_t* d = new std::uint8_t[m.get_total_length()];
                                    memcpy(d, it->second, m.get_total_length());
                                for (int f = 0; f < j; f++) {
                                   this->sock.sendto(this->peer_addr, m.data, m.get_total_length());
                                   }
                                   this->send_data_handler(it->first, m.get_length());
                                   it->second = d;
                                   }
                                   j++;
                                 continue;
			                  }

			                  MessageClass resp(buffer);
                			  if (!this->is_valid(resp)) {
				                  this->corrupt_ack_handler(resp.get_magic(), this->sequence);
                			  } else {
                            if (packets.find(resp.get_sequence()) != packets.end()){
                              packets.erase(resp.get_sequence());
                              this->recv_ack_handler(resp.get_sequence());
                              //if receive a valid ack, divide j by 2, make sure j is at minimum 2
                              j = j/2;
                              if(j <= 1) {
                                j = 2;
                              }
                          }
                        }
                }
                
                //send eof message, no need to ack, so send 50 of this message
                MessageClass eof_message(false, true, this->sequence++, 0);
                for (int l = 0; l < 50; l++) {
                  this->sock.sendto(this->peer_addr, eof_message.data, eof_message.get_total_length());
                }
                this->send_eof_handler();
                this->completed_handler();
                //free the array
                for(int g = 0; g <= i; g++) {
                  free(messages[g]);
                }
                free(messages);
        }

};

#endif//BASIC_SENDER_H_
