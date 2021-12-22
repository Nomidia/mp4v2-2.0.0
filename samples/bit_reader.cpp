#include "bit_reader.h"

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::LC_MP4_BitReader
+---------------------------------------------------------------------*/
LC_MP4_BitReader::LC_MP4_BitReader(const uint8_t* data, uint32_t data_size) :
    m_buffer(data),
    m_size(data_size),
    m_position(0),
    m_cache(0),
    m_bits_cached(0)
{
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::~LC_MP4_BitReader
+---------------------------------------------------------------------*/
LC_MP4_BitReader::~LC_MP4_BitReader()
{
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::Reset
+---------------------------------------------------------------------*/
int LC_MP4_BitReader::Reset()
{
    m_position   = 0;
    m_cache      = 0;
    m_bits_cached = 0;

    return 0;
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::GetBitsRead
+---------------------------------------------------------------------*/
uint32_t LC_MP4_BitReader::GetBitsRead()
{
    return 8*m_position - m_bits_cached;
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::ReadCache
+---------------------------------------------------------------------*/
LC_MP4_BitReader::BitsWord
LC_MP4_BitReader::ReadCache() const
{
    const uint8_t* out_ptr = m_buffer + m_position;
    return (((LC_MP4_BitReader::BitsWord) out_ptr[0]) << 24) |
           (((LC_MP4_BitReader::BitsWord) out_ptr[1]) << 16) |
           (((LC_MP4_BitReader::BitsWord) out_ptr[2]) <<  8) |
           (((LC_MP4_BitReader::BitsWord) out_ptr[3])      );
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::ReadBits
+---------------------------------------------------------------------*/
uint32_t LC_MP4_BitReader::ReadBits(uint32_t n)
{
    if (n == 0) return 0;
    LC_MP4_BitReader::BitsWord result;
    if (m_bits_cached >= n) {
        /* we have enough bits in the cache to satisfy the request */
        m_bits_cached -= n;
        result = (m_cache >> m_bits_cached) & LC_MP4_BIT_MASK(n);
    } else {
        /* not enough bits in the cache */
        LC_MP4_BitReader::BitsWord word = ReadCache();
        m_position += LC_MP4_WORD_BYTES;

        /* combine the new word and the cache, and update the state */
        LC_MP4_BitReader::BitsWord cache = m_cache & LC_MP4_BIT_MASK(m_bits_cached);
        n -= m_bits_cached;
        m_bits_cached = LC_MP4_WORD_BITS - n;
        result = m_bits_cached ? (word >> m_bits_cached) | (cache << n) : word;
        m_cache = word;
    }

    return result;
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::ReadBit
+---------------------------------------------------------------------*/
int LC_MP4_BitReader::ReadBit()
{
    LC_MP4_BitReader::BitsWord result;
    if (m_bits_cached == 0) {
        /* the cache is empty */

        /* read the next word into the cache */
        m_cache = ReadCache();
        m_position += LC_MP4_WORD_BYTES;
        m_bits_cached = LC_MP4_WORD_BITS - 1;

        /* return the first bit */
        result = m_cache >> (LC_MP4_WORD_BITS - 1);
    } else {
        /* get the bit from the cache */
        result = (m_cache >> (--m_bits_cached)) & 1;
    }
    return result;
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::PeekBits
+---------------------------------------------------------------------*/
uint32_t LC_MP4_BitReader::PeekBits(uint32_t n)
{
   /* we have enough bits in the cache to satisfy the request */
   if (m_bits_cached >= n) {
      return (m_cache >> (m_bits_cached - n)) & LC_MP4_BIT_MASK(n);
   } else {
      /* not enough bits in the cache, read the next word */
      LC_MP4_BitReader::BitsWord word = ReadCache();

      /* combine the new word and the cache, and update the state */
      LC_MP4_BitReader::BitsWord   cache = m_cache & LC_MP4_BIT_MASK(m_bits_cached);
      n -= m_bits_cached;
      return (word >> (LC_MP4_WORD_BITS - n)) | (cache << n);
   }
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::PeekBit
+---------------------------------------------------------------------*/
int LC_MP4_BitReader::PeekBit()
{
   /* the cache is empty */
   if (m_bits_cached == 0) {
      /* read the next word into the cache */
      LC_MP4_BitReader::BitsWord cache = ReadCache();

      /* return the first bit */
      return cache >> (LC_MP4_WORD_BITS - 1);
   } else {
      /* get the bit from the cache */
      return (m_cache >> (m_bits_cached-1)) & 1;
   }
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::SkipBits
+---------------------------------------------------------------------*/
void LC_MP4_BitReader::SkipBits(uint32_t n)
{
   if (n <= m_bits_cached) {
      m_bits_cached -= n;
   } else {
      n -= m_bits_cached;
      while (n >= LC_MP4_WORD_BITS) {
         m_position += LC_MP4_WORD_BYTES;
         n -= LC_MP4_WORD_BITS;
      }
      if (n) {
         m_cache = ReadCache();
         m_bits_cached = LC_MP4_WORD_BITS-n;
         m_position += LC_MP4_WORD_BYTES;
      } else {
         m_bits_cached = 0;
         m_cache = 0;
      }
   }
}

/*----------------------------------------------------------------------
|   LC_MP4_BitReader::SkipBit
+---------------------------------------------------------------------*/
void LC_MP4_BitReader::SkipBit()
{
   if (m_bits_cached == 0) {
      m_cache = ReadCache();
      m_position += LC_MP4_WORD_BYTES;
      m_bits_cached = LC_MP4_WORD_BITS - 1;
   } else {
      --m_bits_cached;
   }
}

