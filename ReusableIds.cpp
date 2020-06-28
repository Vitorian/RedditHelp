#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>

template< typename T >
class ReusableIds {
public:

    ReusableIds( T size ) { init( size ); }

    T alloc() {
        T id = first;
        first = ids[first];
        return id;
    }

    void free( T id ) {
        ids[id] = first;
        first = id;
    }

private:
    T first;
    std::vector< T > ids;

    void init( T size ) {
        ids.resize(size);
        for ( T j=0; j<size; ++j ) {
            ids[j] = j+1;
        }
        first = 0;
    }

};

int main()
{
    const uint32_t SIZE = 32;
    ReusableIds<uint32_t> gen(SIZE);
    for ( uint32_t j=0; j<10; ++j ) {
        uint32_t id[SIZE];
        for ( uint32_t k=0; k<SIZE; ++k ) {
            id[k] = gen.alloc();
            std::cout << id[k] << ",";
        }
        std::cout << std::endl;

        std::random_shuffle( &id[0], &id[SIZE] );

        for ( uint32_t k=0; k<SIZE; ++k ) {
            gen.free(id[k]);
        }
    }
}
