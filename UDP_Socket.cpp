#include <cstddef>

#include <errno.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "UDP_Socket.h"


UDP_Socket::UDP_Socket(void)
        : sock_fd(socket(AF_INET, SOCK_DGRAM, 0)),
	  local_address(UDP_Address())
          {}

UDP_Socket::~UDP_Socket(void) {
	if (this->sock_fd < 0) {
		close(this->sock_fd);
	}
}

void UDP_Socket::bind(void) {
        this->bind(this->local_address);
}

void UDP_Socket::bind(const UDP_Address &address) {
        auto mySockaddr = address.to_sockaddr();
        if (::bind(this->sock_fd, (sockaddr*) &mySockaddr, sizeof(mySockaddr)) != 0) {
                throw 1;
        }

        socklen_t sockaddr_size = sizeof(mySockaddr);
        if (getsockname(this->sock_fd, (sockaddr*) &mySockaddr, &sockaddr_size) != 0) {
                throw 2;
        }

        this->local_address = UDP_Address::from_sockaddr(mySockaddr);
}

void UDP_Socket::set_timeout(std::uint8_t seconds, std::uint32_t microseconds) {
        timeval tv = {seconds, microseconds};
        if (setsockopt(this->sock_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            throw 5;
        }
}

void UDP_Socket::sendto(const UDP_Address &to, const std::vector<std::uint8_t> &data) {
        auto sockaddr = to.to_sockaddr();
        if (::sendto(this->sock_fd, data.data(), data.size(), 0,
               (struct sockaddr *) &sockaddr, (socklen_t) sizeof(sockaddr)) == -1) {
               throw 3;
       }
}

UDP_Address UDP_Socket::recvfrom(std::vector<std::uint8_t> &buffer) {
        // Use maximum availible capacity in the vector for buffering
        buffer.resize(buffer.capacity());

        struct sockaddr_in sockaddr;
	socklen_t sockaddr_size = sizeof(sockaddr_in);
        if (::recvfrom(this->sock_fd, buffer.data(), buffer.size(), 0,
                 (struct sockaddr *) &sockaddr, &sockaddr_size) == -1) {
                if (errno == ETIMEDOUT) {
                        throw 9;
                } else {
                        throw 4;
                }
         }

	return UDP_Address::from_sockaddr(sockaddr);
}
