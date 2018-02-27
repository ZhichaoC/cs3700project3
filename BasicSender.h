#include <cstdint>
#include <functional>

template<typename MessageClass>
class BasicSender {
        static const std::uint16_t DATA_SIZE = 1460;
        std::uint32_t sequence;

        std::Function<void> timeout_handler, corrupt_ack_handler;
        std::Function<void(std::string)> ack_handler;
};
