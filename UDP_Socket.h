#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include <cstdint>
#include <sstream>

#include <arpa/inet.h>

class UDP_Address {
public:
        std::string ip;
        std::uint16_t port;

        inline static UDP_Address from_sockaddr(const sockaddr_in & sockaddr) {
                return UDP_Address(inet_ntop(sockaddr.sin_addr.s_addr), sockaddr.sin_port);
        }

        inline sockaddr_in to_sockaddr(void) const {
                return {.sin_family = AF_INET,
                        .sin_port = htons(this->port),
                        .sin_addr = {.s_addr = inet_addr( this->ip.c_str() )}};
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

        void sendto(const local_address & to);
        void recvfrom(const local_address & from);
};

#endif // UDP_SOCKET_H_
