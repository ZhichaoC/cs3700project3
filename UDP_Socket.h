#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <stdlib.h>

class UDP_Address {
public:
        std::string ip;
        std::uint16_t port;

	UDP_Address(std::string ip, std::uint16_t port) : ip(ip), port(port) {}

        inline static UDP_Address from_sockaddr(const sockaddr_in & sockaddr) {
		// MEMORY LEAK
                return UDP_Address(inet_ntop(AF_INET, &sockaddr.sin_addr.s_addr, (char*)malloc(sizeof(char)*INET_ADDRSTRLEN), INET_ADDRSTRLEN), ntohs(sockaddr.sin_port));
        }

        inline sockaddr_in to_sockaddr(void) const {
                sockaddr_in sockaddr;
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port = htons(this->port);
                sockaddr.sin_addr.s_addr = inet_addr( this->ip.c_str() );
                return sockaddr;
        }
};

class UDP_Socket {
        int sock_fd;
        UDP_Address local_address;

public:
        UDP_Socket(void);
        ~UDP_Socket(void);

        void bind(void);
        void bind(const UDP_Address & local_address);

        void set_timeout(std::uint8_t seconds, std::uint32_t microseconds);

        void sendto(const UDP_Address &to, std::vector<std::uint8_t> data);
        UDP_Address recvfrom(std::vector<std::uint8_t> &buffer);
};

#endif // UDP_SOCKET_H_
