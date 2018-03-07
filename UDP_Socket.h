#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include <cstdint>
#include <exception>
#include <string>

#include <unistd.h> // close
#include <arpa/inet.h> // ntons, ..., inet_ntop, ...

class TimeoutException : public std::exception {};
class BadFdException : public std::exception {};
class InvalidArgsException : public std::exception {};
class SendToException : public std::exception {};
class RecvFromException : public std::exception {};

class UDP_Address {
public:
        std::string ip;
        std::uint16_t port;

	UDP_Address() : ip(std::string("0.0.0.0")), port(0) {}
	UDP_Address(std::string ip, std::uint16_t port) : ip(ip), port(port) {}

        //what address does the messge come from
        inline static UDP_Address from_sockaddr(const sockaddr_in & sockaddr) {
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sockaddr.sin_addr.s_addr, addr, INET_ADDRSTRLEN);
		return UDP_Address(std::string(addr), ntohs(sockaddr.sin_port));
        }

        //create sockaddr
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

public:
        UDP_Address local_address;
        UDP_Socket(void) : sock_fd(socket(AF_INET, SOCK_DGRAM, 0)),
		local_address(UDP_Address()) {}
        UDP_Socket(UDP_Socket&) = delete;
	UDP_Socket(UDP_Socket&& rhs) { *this = std::move(rhs); }
	UDP_Socket& operator = (UDP_Socket&& rhs) {
		this->sock_fd = rhs.sock_fd;
		rhs.sock_fd = -1;
		this->local_address = rhs.local_address;
		return *this;
	}
        ~UDP_Socket(void) {
		if (this->sock_fd < 0) {
			close(this->sock_fd);
		}
	}

        inline void bind(void)
		{ this->bind(this->local_address); }
        void bind(const UDP_Address & local_address);

        void set_timeout(std::uint8_t seconds, std::uint32_t microseconds);

        void sendto(const UDP_Address &to, std::uint8_t *data, size_t data_len);
        UDP_Address recvfrom(std::uint8_t *buffer, std::int64_t *buffer_len);
};

#endif // UDP_SOCKET_H_
