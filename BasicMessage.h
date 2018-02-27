#include <cstdint>

class BasicMessage {
        typedef std::uint16_t MagicType;
        typedef std::uint16_t LengthType;
        typedef std::uint32_t SequenceType;

        static const MagicType MAGIC = 0x0bee;

        MagicType magic:14, ack:1, eof:1;
        LengthType length;
        SequenceType sequence;
        char* data;

public:
        BasicMessage(const bool &ack, const bool &eof, const SequenceType &sequence)
                : magic(MAGIC), ack(ack), eof(eof), sequence(sequence), length(0), data(nullptr) {};

        inline bool is_ack(void)
                { return this->ack; }
        inline bool is_eof(void)
                { return this->eof; }
        inline SequenceType get_sequence(void)
                { return this->sequence; }
        inline void set_sequence(const SequenceType & seq)
                { this->sequence = seq; }
        inline bool is_valid(void)
                { return this->magic == BasicMessage::MAGIC; }

        static BasicMessage load(char* message, size_t len_message, char* data);
};

static_assert(sizeof(BasicMessage) == 16, "BasicMessage not eight bytes in size");
static_assert(alignof(BasicMessage) == 8, "BasicMessage not four byte aligned");
