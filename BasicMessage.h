#include <cstdint>

class BasicMessage {
public:
        typedef std::uint16_t MagicType;
        typedef std::uint16_t LengthType;
        typedef std::uint32_t SequenceType;

        static const MagicType MAGIC = 0x0bee;

	struct Header {
        	MagicType magic:14, ack:1, eof:1;
        	LengthType length;
        	SequenceType sequence;
	};
	static_assert(sizeof(Header) == 8, "BasicMessage not eight bytes in size");
	static_assert(alignof(Header) == 4, "BasicMessage not four byte aligned");

private:
	inline Header& header_ref(void) {return (Header&)*data; }

public:
	std::uint8_t *data;
        BasicMessage(bool ack, bool eof, SequenceType sequence)
        	: BasicMessage(ack, eof, sequence, 0) {};
        BasicMessage(bool ack, bool eof, SequenceType sequence, size_t mss)
		: data((std::uint8_t*)malloc(sizeof(Header)+mss)) {
		this->header_ref() = {MAGIC, ack, eof, 0, htonl(sequence)};
	}
        BasicMessage(std::uint8_t *data) : data(data) {};
	~BasicMessage(void) { free(data); }

        inline MagicType get_magic(void)
                { return this->header_ref().magic; }
        inline void set_magic(MagicType magic)
                { this->header_ref().magic = magic; }

	inline bool is_ack(void)
                { return this->header_ref().ack; }
        inline void set_ack(bool ack)
                { this->header_ref().ack = ack; }

        inline bool is_eof(void)
                { return this->header_ref().eof; }
	inline void set_eof(bool eof)
		{ this->header_ref().eof = eof;}

        inline LengthType get_length(void)
                { return ntohs(this->header_ref().length); }
        inline void set_length(LengthType length)
                { this->header_ref().length = htons(length); }

        inline SequenceType get_sequence(void)
                { return ntohl(this->header_ref().sequence); }
        inline void set_sequence(SequenceType sequence)
                { this->header_ref().sequence = htonl(sequence); }

	inline bool is_valid(void)
                { return this->get_magic() == BasicMessage::MAGIC; }

	inline std::uint8_t *get_data(void) { return this->data + sizeof(Header); }
	//inline void assign_data(std::uint8_t *data, size_t length) { free(this->data); this->data=data; this->set_length(length); }
};
