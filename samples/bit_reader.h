#ifndef __BIT_READER_H__
#define __BIT_READER_H__

#include <stdint.h>

#define LC_MP4_WORD_BITS  32
#define LC_MP4_WORD_BYTES 4
#define LC_MP4_BIT_MASK(_n) ((1<<(_n))-1)

class LC_MP4_BitReader
{
public:
    typedef unsigned int BitsWord;

    LC_MP4_BitReader(const uint8_t* data, uint32_t data_size);
    ~LC_MP4_BitReader();

    // methods
    int        Reset();
    int        ReadBit();
    uint32_t   ReadBits(uint32_t bit_count);
    int        PeekBit();
    uint32_t   PeekBits(uint32_t bit_count);
    int        SkipBytes(uint32_t byte_count);
    void       SkipBit();
    void       SkipBits(uint32_t bit_count);

    uint32_t GetBitsRead();

private:
    // methods
    BitsWord ReadCache() const;

    // members
    const uint8_t*  m_buffer;
    uint32_t     m_size;
    uint32_t     m_position;
    uint32_t     m_cache;
    unsigned int m_bits_cached;
};

#endif

