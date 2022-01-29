#ifndef BIT2ADDR_H
#define BIT2ADDR_H
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "configure.hpp"
typedef  struct TransAddr {
    int num;
    char * addr;
}TransAddr ,  *TransAddrPtr ;
typedef struct CompressAddr{
    MASK mask_;
    int compress_vec[VECTOR_MAX];  
}CompressAddr;

class Bit2Addr{
    
    public:
    int vector_;
    TransAddr trans_addr_;
    explicit Bit2Addr( int vector ) : vector_(vector) {}
    int get_max_len( const int mask ) ;
    friend std::ostream & operator << ( std::ostream & stream , const Bit2Addr& para ) ;
    TransAddr generate( int mask ) ;

    CompressAddr generate_compress( int mask ) ;
};
#endif
