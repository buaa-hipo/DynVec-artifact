#include "bit2addr.h"   
#include "log.h"
    int Bit2Addr::get_max_len( const int mask ) {
        int len = 1;
        for( int i = 0 ; i < vector_ ; ) {
            int len_tmp = 1;
            int j;
            for(  j = i + 1 ; j < vector_; j++ ) {
                if( ((1 << j) & mask) != 0 ) {
                    len = len_tmp > len? len_tmp : len;
                    break;
                } else {
                    len_tmp++;
                }
            }
            if( j == vector_ ) { 
                len = len_tmp > len? len_tmp : len;
            }

            i = j;
        }
        int res = 0;
        for( len--; len != 0 ; res++, len >>= 1 );
        return res;
    }
    std::ostream & operator << ( std::ostream & stream , const Bit2Addr& para ) {
        TransAddr trans_addr = para.trans_addr_;
        stream << trans_addr.num << std::endl ;
        char * addr = trans_addr.addr;
        for( int i = 0 ; i < trans_addr.num; i++ ) {
            for( int j = 0 ; j < para.vector_; j++ )
                stream << std::dec << (int)addr[i * para.vector_ + j] << " ";
            stream<<std::endl;
        }
        return stream;
    }

    TransAddr Bit2Addr::generate( int mask ) {
        int max_len = get_max_len( mask ); 
    
        trans_addr_.num = max_len;
        trans_addr_.addr = ( char* ) malloc( sizeof(char) * max_len * vector_ );
        char * addr_tmp = trans_addr_.addr;
        int num = 0 ;
        int each_num[ vector_ ];
        each_num[ num  ] = 0;
        for( int i = 1 ; i < vector_ ; i++ ) {
            if( ((1 << i) & mask) == 0 ) {
                each_num[ num  ] ++;
            } else {
                num++;
                each_num[num] = 0;
            }
        }
        num++;
        for( int i = 0 ; i < max_len ; i++ , addr_tmp += vector_ ) {
            int begin = 0;
            for( int j = 0 ; j < vector_ ; j++ ) {
                addr_tmp[ j ] = vector_;
            }
            int space = (1<<(i+1));
            int add_space = ( 1<<i );
             for( int j = 0 ; j < num; j++ )  {
                for( int k = 0 ; k < each_num[ j ]; k += space ) {
                    addr_tmp[ k + begin ] = (k + add_space + begin) > begin + each_num[j] ? vector_ : k + add_space + begin;
                
                }
                
                begin += each_num[j] + 1;
             } 
        }
        return trans_addr_;
    }

    CompressAddr Bit2Addr::generate_compress( int circle_mask ) {
            int ii = 0;

            CompressAddr compress_addr;

            MASK compress_mem_mask = 0;
            for( int i = 0; i < vector_ ; i++ ) {
                if(( (1<<i) & circle_mask ) != 0) {
                    compress_addr.compress_vec[ii] = i;
                    compress_mem_mask |= (1<<ii);
                    ii++;
                }
            }
            for( ; ii < vector_ ; ii++ ) {
                compress_addr.compress_vec[ii] = vector_;
            }
            compress_addr.mask_ = compress_mem_mask;
            return compress_addr;
    }
/*#define VECTOR 8
int main(int argc , char ** argv) {
    Bit2Addr generate_trans_code( VECTOR );
    int num = atoi(argv[1]);
    std::cout<<std::hex<<num<<std::endl;
    generate_trans_code.generate( num );
    
    std::cout << generate_trans_code;
}*/
