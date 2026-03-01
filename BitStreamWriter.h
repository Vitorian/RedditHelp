#ifndef BITSTREAMWRITER_H
#define BITSTREAMWRITER_H

#include <cstdint>

template< class Stream >
class BitStreamWriter
{
public:
    // Keep an internal 64-bit shift register and spill whole bytes to stream.
    BitStreamWriter( Stream& of ) : _of(of), _bits(0), _count(0) {}
    ~BitStreamWriter() { flush(); }

    void pushbit( uint32_t bit ) {
        // Append next bit at the current low-end position.
        _bits |= (uint64_t(bit) & 1ULL) << _count;
        if ( ++_count==BITS_PER_WORD ) commit();
    }

    void flush() {
        // Pad with zero bits so we can emit an integral number of bytes.
        uint32_t leftbits = _count % 8;
        if ( leftbits>0 ) {
            _count += 8-leftbits;
        }
        commit();
    }

    void commit() {
        // Emit all currently complete bytes from the low end of the register.
        if ( _count==0 ) return;
        uint32_t bytes = _count / 8;
        if ( bytes>0 ) {
            _of.write( (const char*)&_bits, bytes );
            _count -= bytes * 8;
            if ( bytes>=sizeof(_bits) ) {
                _bits = 0;
            } else {
                _bits = _bits >> (bytes*8);
            }
        }
    }
private:
    static const uint32_t BITS_PER_WORD = sizeof(uint64_t) * 8;
    Stream& _of;
    uint64_t _bits;
    uint32_t _count;
};

#endif // BITSTREAMWRITER_H
