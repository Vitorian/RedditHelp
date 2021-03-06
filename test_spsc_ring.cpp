/* clang++ test_spsc_ring.cpp -o test_spsc_ring -std=c++11 -l pthread
   ./test_spsc_ring
 */

#include <memory>
#include <thread>
#include <cstring>
#include <cassert>
#include <chrono>
#include <atomic>

template< typename T, typename IndexT = uint32_t, typename AllocT = std::allocator<T> >
class SimpleRing {
public:
    static constexpr uint32_t SLOTSIZE = sizeof(T);
    SimpleRing( uint32_t sz ) : size(sz) {
        data = allocator.allocate( size );
        read_idx = 0;
        write_idx = 0;
    }
    ~SimpleRing() {
        allocator.deallocate( data, size );
    }
    bool push( const T& obj ) {
        uint32_t next_idx = write_idx+1<size ? write_idx+1 : 0;
        if (  next_idx != read_idx ) {
            data[write_idx] = obj;
            write_idx = next_idx;
            return true;
        }
        return false;
    }
    bool pop( T& obj ) {
        uint32_t next_idx = read_idx+1<size ? read_idx+1 : 0;
        if ( read_idx != write_idx ) {
            obj = data[read_idx];
            read_idx = next_idx;
            return true;
        }
        return false;
    }
private:
    SimpleRing();
    SimpleRing( const SimpleRing& );
    std::atomic<IndexT> read_idx;
    std::atomic<IndexT> write_idx;
    const uint32_t size;
    T* data;
    AllocT allocator;
};

template< typename T, typename IndexT = uint32_t, typename AllocT = std::allocator<T> >
class SnellmanRing {
public:
    static constexpr uint32_t SLOTSIZE = sizeof(T);
    SnellmanRing( uint32_t sz ) : size(sz) {
        data = allocator.allocate( sz );
        read_idx = 0;
        write_idx = 0;
    }
    ~SnellmanRing() {
        allocator.deallocate( data, size );
    }
    bool push( const T& obj ) {
        uint32_t numel = write_idx - read_idx;
        if ( numel < size  ) {
            data[write_idx%size] = obj;
            write_idx++;
            return true;
        }
        return false;
    }
    bool pop( T& obj ) {
        uint32_t numel = write_idx - read_idx;
        if ( numel>0 ) {
            obj = data[read_idx%size];
            read_idx++;
            return true;
        }
        return false;
    }
private:
    SnellmanRing();
    SnellmanRing( const SnellmanRing& );
    std::atomic<IndexT> read_idx;
    std::atomic<IndexT> write_idx;
    const uint32_t size;
    T* data;
    AllocT allocator;
};


template< typename T, typename IndexT = uint32_t, typename AllocT = std::allocator<T> >
class VitorianRing {
public:
    static constexpr uint32_t SLOTSIZE = sizeof(T);
    VitorianRing( uint32_t sz ) : size(sz) {
        data = allocator.allocate( sz );
        read_idx = 0;
        write_idx = 2*size;
    }
    ~VitorianRing() {
        allocator.deallocate( data, size );
    }

    bool push( const T& obj ) {
        uint32_t numel = (write_idx-read_idx) % (2*size);
        //fprintf( stderr, "Push read:%d write:%d diff:%d\n", read_idx, write_idx, numel );
        if ( numel < size ) {
            data[write_idx%size] = obj;
            write_idx = ( write_idx+1 < 4*size ) ? write_idx + 1 : 2*size;
            return true;
        }
        return false;
    }
    bool pop( T& obj ) {
        uint32_t numel = (write_idx-read_idx) % (2*size);
        //fprintf( stderr, "Pop  read:%d write:%d diff:%d\n", read_idx, write_idx, numel );
        if ( numel > 0 ) {
            obj = data[read_idx%size];
            read_idx = ( read_idx+1 < 2*size ) ? read_idx+1 : 0;
            return true;
        }
        return false;
    }
private:
    VitorianRing();
    VitorianRing( const VitorianRing& );
    std::atomic<IndexT> read_idx;
    std::atomic<IndexT> write_idx;
    const uint32_t size;
    T* data;
    AllocT allocator;
};

template< class RingT >
void producer( RingT& rng, int count, int seed ) {
    for ( int j=0; j<count; ++j ) {
        while ( !rng.push( j + seed ) );
            //std::this_thread::sleep_for( std::chrono::milliseconds(1) );
        //fprintf( stderr, " >> pushed %d\n", j+val );
    }
}

template< class RingT >
void consumer( RingT& rng, int count, int seed ) {
    for ( int j=0; j<count; ++j ) {
        int res;
        while ( !rng.pop( res ) );
            //std::this_thread::sleep_for( std::chrono::milliseconds(1) );
        //fprintf( stderr, " >> popped %d\n", res );
        assert( res==seed+j );
    }
}

template< class RingT >
void test()
{
    printf( "Testing...\n" );
    try {
        RingT rng( 8 );

        std::thread th1( producer<RingT>, std::ref(rng), 10000, 99 );
        std::thread th2( consumer<RingT>, std::ref(rng), 10000, 99 );

        th1.join();
        th2.join();
    }
    catch( std::exception& ex ) {
        printf( "%s\n", ex.what() );
    }
}

int main( int argc, char* argv[] ) {
    test<SimpleRing<int>>();
    test<SnellmanRing<int>>();
    test<VitorianRing<int>>();
}
