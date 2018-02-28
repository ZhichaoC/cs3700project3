#include <cstddef>

#include <errno.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "UDP_Socket.h"

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

void UDP_Socket::sendto(const UDP_Address &to, std::uint8_t *data, size_t data_len) {
        auto sockaddr = to.to_sockaddr();
        if (::sendto(this->sock_fd, data, data_len, 0,
               (struct sockaddr *) &sockaddr, (socklen_t) sizeof(sockaddr)) == -1) {
               throw 3;
       }
}

UDP_Address UDP_Socket::recvfrom(std::uint8_t *buffer, std::int64_t *buffer_len) {
        struct sockaddr_in sockaddr;
	socklen_t sockaddr_size = sizeof(sockaddr_in);
	*buffer_len = ::recvfrom(this->sock_fd, buffer, *buffer_len, 0,
                 (struct sockaddr *) &sockaddr, &sockaddr_size); 
        if (*buffer_len == -1) {
                if (errno == ETIMEDOUT) {
                        throw TimeoutException();
                } else {
                        throw 4;
                }
         }

	return UDP_Address::from_sockaddr(sockaddr);
}
