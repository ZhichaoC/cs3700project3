#include <cstdint>
#include <functional>
#include <iostream>

template<typename MessageClass>
class BasicSender {
        static const std::uint16_t DATA_SIZE = 1460;
        std::uint32_t sequence;
public:
        std::function<void(void)> timeout_handler, corrupt_ack_handler;
        std::function<void(std::string)> ack_handler;

        void transmit_stream(std::ostream &stream);
};
