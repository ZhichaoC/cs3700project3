#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <stdlib.h>

class TimeoutException : public std::Exception {};

class UDP_Address {
public:
        std::string ip;
        std::uint16_t port;

	UDP_Address() : ip(std::string("0.0.0.0")), port(0) {}
	UDP_Address(std::string ip, std::uint16_t port) : ip(ip), port(port) {}

        inline static UDP_Address from_sockaddr(const sockaddr_in & sockaddr) {
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sockaddr.sin_addr.s_addr, addr, INET_ADDRSTRLEN);
		return UDP_Address(std::string(addr), ntohs(sockaddr.sin_port));
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
        UDP_Socket(UDP_Socket&) = delete;
	UDP_Socket(UDP_Socket&& rhs) { *this = std::move(rhs); }
	UDP_Socket& operator = (UDP_Socket&& rhs) {
		this->sock_fd = rhs.sock_fd;
		rhs.sock_fd = -1;
		this->local_address = rhs.local_address;
		return *this;
	}
        ~UDP_Socket(void);

        void bind(void);
        void bind(const UDP_Address & local_address);

        void set_timeout(std::uint8_t seconds, std::uint32_t microseconds);

        void sendto(const UDP_Address &to, const std::vector<std::uint8_t> &data);
        UDP_Address recvfrom(std::vector<std::uint8_t> &buffer);
};

#endif // UDP_SOCKET_H_
