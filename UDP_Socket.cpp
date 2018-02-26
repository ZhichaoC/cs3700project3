#include <cstddef>
#include <cstdint>

#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "UDP_Socket.h"


UDP_Socket::UDP_Socket(void)
        : local_address(UDP_Address("0.0.0.0", 0)),
          sock_fd(socket(AF_INET, SOCK_DGRAM, 0)) {}

UDP_Socket::~UDP_Socket(void) { close(this->sock_fd); }

void UDP_Socket::bind(void) {
        this->bind(this->local_address);
}

void UDP_Socket::bind(const UDP_Address &address) {
        auto sockaddr = address.to_sockaddr();
        if (bind(this->sock_fd, (sockaddr*) &sockaddr, sizeof(sockaddr)) != 0) {
                throw 1;
        }

        if (getsockname(this->sock_fd, sockaddr, sizeof(sockaddr)) != 0) {
                throw 2;
        }

        this->local_address = UDP_Address.from_sockaddr(sockaddr);
}

void set_timeout(std::uint8_t seconds, std::uint32_t microseconds) {
        timeval tv = {.tv_sec = seconds, .tv_usec = microseconds};
        if (setsockopt(rcv_sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            throw 5;
        }
}

void UDP_Socket::sendto(const UDP_Address &to, std::vector<std::uint8_t> data) {
        auto sockaddr = to.to_sockaddr();
        if (sendto(this->sock_fd, data.data(), data.size(), 0,
               (struct sockaddr *) &sockaddr, (socklen_t) sizeof(sockaddr)) == -1) {
               throw 3;
       }
}

void UDP_Socket::recvfrom(UDP_Address *from, std::vector<std::uint8_t> buffer) {
        // Use maximum availible capacity in the vector for buffering
        data.resize(data.capacity());

        struct sockaddr_in sockaddr;
        if (recvfrom(this->sock_fd, data.data(), data.capacity(), 0,
                 (struct sockaddr *) &sockaddr, sizeof(sockaddr))) == -1) {
                 throw 4;
         }

        if (from != nullptr) {
                from = new UDP_Address::from_sockaddr(sockaddr);
        }
}
