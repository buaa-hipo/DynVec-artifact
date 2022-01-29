#include "tools_set.hpp"
#include "hash_feature_table.hpp"
#include "log.h"
std::ostream & operator<<( std::ostream &stream , const DisorderAddr & in ) {
    
    for( int i = 0 ; i < VECTOR_MAX ; i++ )
        stream << (int)in.data_vec[i] << " ";
    stream << std::endl;
    return stream;
}

std::ostream & operator<<( std::ostream &stream , const GatherInfo &scatter_info ) {
    stream << scatter_info.order_type_;
    if(scatter_info.order_type_ == DisOrder) {
        stream << scatter_info.disorder_addr_;
        stream <<( scatter_info.get_mask() )<<"\n";
        for( int i = 0 ; i < (VECTOR_MAX) ; i++ ) {
            stream << scatter_info.data_index_[i];
        }
    } else {
        stream <<" " << scatter_info.data_index_[0] << "\n";
    }
    return stream;
}


    
ScatterInfo generate_scatter_disorder_info_elem(const int * elem_addr ,const int vector_) {
    ScatterInfo scatter_info;
    int level;
    
            for( int v_i = 0 ; v_i < vector_ ; v_i++ ) 
                scatter_info.data_index_[v_i] = elem_addr[v_i];
            level = vector_;
    scatter_info.set_mask( level | (DisOrder << VECTOR_MAX));
    return scatter_info;
}

GatherInfo generate_gather_disorder_info_elem(const int * elem_addr,const int vector_ ) {
    GatherInfo scatter_info;
        DisorderAddr disorder_addr;
        std::vector<int> column_begin_vec;
        int column_block[vector_];
        for( int v_i = 0 ; v_i < vector_; v_i++ ) {
            column_block[v_i] = elem_addr[v_i];
        }
        int num = 0;
        while(true) {
            int min = INT_MAX ;
            for( int v_i = 0 ; v_i < vector_ ; v_i++) {
                min = column_block[v_i] < min ? column_block[v_i] : min;
            }
            if( min == INT_MAX) {
                break;
            } else {
                column_begin_vec.push_back( min );
            }
            num++;

            for( int v_i = 0 ; v_i < vector_ ; v_i++ ) {
                if( column_block[v_i] < min + vector_ ) {
                    disorder_addr.data_vec[ v_i ] = column_block[v_i] - min;
                    column_block[v_i] = INT_MAX ;
                }
            }
        }
        int level;
#define LOAD_TO_GATHER_LEVEL 1
        if( num <= LOAD_TO_GATHER_LEVEL ) {

            scatter_info.data_index_[0] = column_begin_vec[0];
            level = num;
            scatter_info.disorder_addr_ = disorder_addr;
        } else {
            for( int v_i = 0 ; v_i < vector_ ; v_i++ ) 
                scatter_info.data_index_[v_i] = elem_addr[v_i];
            level = vector_;
        }
    if( vector_ > VECTOR4 )
        scatter_info.set_mask( level | (DisOrder<<VECTOR_MAX));
    else {

        int mask = 0;
        if(level == 1) {
            CHECK(disorder_addr.data_vec[0] < VECTOR4);
            CHECK(disorder_addr.data_vec[1] < VECTOR4);

            CHECK(disorder_addr.data_vec[2] < VECTOR4);
            CHECK(disorder_addr.data_vec[3] < VECTOR4);

            mask |= (disorder_addr.data_vec[ 0 ] & 0xf) ;

            mask |= ((disorder_addr.data_vec[ 1 ] << 4) & 0xf0);

            mask |= ((disorder_addr.data_vec[ 2 ] << 8) & 0xf00);

            mask |= ((disorder_addr.data_vec[ 3 ] << 12) & 0xf000);
        } else {
            mask = vector_; 
        }
        scatter_info.set_mask( mask | DisOrder << VECTOR_MAX );
    }

    return scatter_info;
}
 OrderType get_order_type(const int * index_ptr,const int vector_) {
    
    int inc_num = 0;
    int dec_num = 0;
    int equal_num = 0;
    int inc_continue_num = 0;
    int dec_continue_num = 0;

    for( int i = 1 ; i < vector_ ; i++ ) {
        
        int before_index = index_ptr[i-1];
        int after_index = index_ptr[i];
        if( before_index < after_index) {
            inc_num++;
            inc_continue_num = before_index + 1 == after_index ? inc_continue_num + 1 : inc_continue_num;
        } else if( before_index > after_index ) {
            dec_num++;

            dec_continue_num = before_index - 1 == after_index ? dec_continue_num + 1 : dec_continue_num;
        } else {
            equal_num++;
        }
    }
    OrderType ot;
    if( equal_num == vector_ - 1) {
        ot =  OrderEquel;
    } else if( vector_ > VECTOR8 ) {
        ot = DisOrder;
    } else if( inc_continue_num + equal_num == vector_ - 1 ) {
        ot = IncContinue;
    } else if(dec_continue_num + equal_num == vector_ - 1) {
//        ot = DecContinue;
        ot = DisOrder;
    } else if( inc_num + equal_num == vector_ - 1) {
//        ot = Inc;
         ot = DisOrder;
    } else if( dec_num + equal_num == vector_ - 1) {
//        ot =  Dec;
         ot = DisOrder;
    } else {
        ot = DisOrder;
    }
    return ot;
}
int get_mask(const int * index_ptr,OrderType order_type,const int vector_) {

    int mask = 0x1;
    if( order_type != DisOrder ) {
        int cur_index = index_ptr[0];
        for( int i = 1 ; i < vector_ ; i++ ) {
            if(cur_index != index_ptr[i]) {
                mask |= (1<<i);
                cur_index = index_ptr[i];
            }
        }
        mask |= (order_type << (VECTOR_MAX) );
    } else {
        LOG(FATAL) << "Unsupported";
    }
    return mask;
}

ScatterInfo generate_scatter_info_elem(const int * index_ptr,const int vector_) {
    ScatterInfo scatter_info;

    OrderType order_type = get_order_type( index_ptr ,vector_);
    if(order_type == OrderEquel) {
        scatter_info.set_mask( 0x1 | (OrderEquel << VECTOR_MAX));
        scatter_info.order_type_ = order_type;
        scatter_info.data_index_[0] = index_ptr[0];
    } else if( order_type == IncContinue ) {
        scatter_info.set_mask( get_mask( index_ptr, order_type ,vector_));
        scatter_info.order_type_ = order_type;
        scatter_info.data_index_[0] = index_ptr[0];
    } else if( order_type == DisOrder ) {
        scatter_info = generate_scatter_disorder_info_elem( index_ptr,vector_ );
        scatter_info.order_type_ = order_type;
    } else {
        LOG(FATAL) << "Unsupported" << order_type; 
    }
    return scatter_info;
}
GatherInfo generate_gather_info_elem(const int * index_ptr,const int vector_) {
    GatherInfo scatter_info;

    OrderType order_type = get_order_type( index_ptr,vector_ );
    if(order_type == OrderEquel) {
        scatter_info.set_mask( 0x1 | (OrderEquel << VECTOR_MAX));
        scatter_info.order_type_ = order_type;
        scatter_info.data_index_[0] = index_ptr[0];
    } else if( order_type == IncContinue ) {
        scatter_info.set_mask( get_mask( index_ptr, order_type,vector_ ));
        scatter_info.order_type_ = order_type;
        scatter_info.data_index_[0] = index_ptr[0];
    } else if( order_type == DisOrder ) {
        scatter_info = generate_gather_disorder_info_elem( index_ptr,vector_ );
        scatter_info.order_type_ = order_type;
    } else {
        LOG(FATAL) << "Unsupported" << order_type; 
    }
    return scatter_info;
}
ReductionInfo generate_reduction_disorder_info_elem( const int * index_ptr,const int vector_ ) {
    ReductionInfo reduction_info;
    if(vector_ > VECTOR4) {
    int max_len = 1;
    bool has_searched[vector_];
    for( int i = 0 ; i < vector_ ; i++ ) {
        has_searched[i] = false;
    }
    for( int i = 0 ; i < vector_; i++ ) {
        int value = index_ptr[i];
        int tmp_max_len = 1;
        if( !has_searched[i]  ) {
        for( int j = i + 1 ; j < vector_ ; j++ ) {
            if(index_ptr[j] == value) {
                tmp_max_len++;
                has_searched[j] = true;
            }
        }
        max_len = tmp_max_len > max_len? tmp_max_len:max_len;
        }
    }
    int reduce_num[] = { 0 , 1 , 2 , 2,3,3,3,3, 4,4,4,4,4,4,4,4 };
    int num = reduce_num[ max_len - 1 ];

    int has_reduce = 0x0;
    for( int i = 0 ; i < num ; i++ ) {
        int8_t * shuffle_index_ptr_tmp = reduction_info.reduction_addr_[i].data_vec; 
        
        for( int i = vector_ - 1 ; i >= 0 ; i-- ) {
            int value = index_ptr[i];
            bool has_find = false;

            if( (has_reduce & (0x1<<i)) == 0 ) {
            for( int j = i - 1 ; j >= 0 ; j-- ) {
                if(value == index_ptr[j]&&(( has_reduce &(0x1<<j))==0) ) {
                    has_find = true;
                    has_reduce |= (0x1<<j);
                    shuffle_index_ptr_tmp[i] = j;
                    break;
                }
            }
            }
            if(!has_find) {
               shuffle_index_ptr_tmp[i] = vector_; 
            }
        }
    }

    reduction_info.set_mask(num | (DisOrder << VECTOR_MAX));
    } else {
        LOG(FATAL) << "Unsupported"; 
    }

    return reduction_info;
}

ReductionInfo generate_reduction_info_elem(const int * index_ptr,const ScatterInfo &scatter_info,const int vector_ ) {
    ReductionInfo reduction_info;
    if(scatter_info.order_type_ == IncContinue || scatter_info.order_type_== OrderEquel ) {
         reduction_info.order_type_ = scatter_info.order_type_; 
         reduction_info.set_mask(scatter_info.get_mask());
    } else if(scatter_info.order_type_ == DisOrder) {
         reduction_info = generate_reduction_disorder_info_elem( index_ptr,vector_ );
         reduction_info.order_type_ = scatter_info.order_type_;
    } else {
        printf("ORDER: %d (must be one of [%d, %d, %d])\n", scatter_info.order_type_, IncContinue, OrderEquel, DisOrder); fflush(stdout);
        LOG(FATAL) << "Unsupported Now" << scatter_info ;
    }
    return reduction_info;
}

void generate_info( ScatterInfo *info_ptr ,const int *addr,int vec_num,const int vector_) {
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = generate_scatter_info_elem( addr + i * vector_ ,vector_);
        }
}
void generate_info( ReductionInfo *info_ptr ,const int *addr, const int vec_num , ScatterInfo * scatter_info_ptr ,const int vector_) {
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = generate_reduction_info_elem( addr + i * vector_, scatter_info_ptr[i],vector_ );
        }
}

void generate_info( GatherInfo * info_ptr ,const int *addr,int vec_num,const int vector_) {
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = generate_gather_info_elem( addr + i * vector_ , vector_);
        }

}
size_t *  get_feature_hash( const std::map<std::string,GatherInfo*> &gather_map,
                            const std::map<std::string,ScatterInfo*> & scatter_map,
                            const std::map<std::string,ReductionInfo*>&reduction_map,
                            const int table_column_num ) {
//    size_t * key_ptr = (size_t*)malloc(sizeof(size_t) * table_column_num);
    size_t * key_ptr = new size_t[ table_column_num ]; 
    for( int i = 0 ; i < table_column_num ; i++ ) {
        size_t key = 0;
        key_ptr[i] = key;
    }
    for( auto it : gather_map ) {
         GatherInfo * info_ptr = it.second;

         for( int i = 0 ; i < table_column_num ; i++ ) 
            key_ptr[i] = HashFeatureTable::HashCombine( key_ptr[i] , info_ptr[i].get_mask() );
    }
    for( auto it : scatter_map ) {
         ScatterInfo * info_ptr = it.second;

         for( int i = 0 ; i < table_column_num ; i++ ) 
            key_ptr[i] = HashFeatureTable::HashCombine( key_ptr[i] , info_ptr[i].get_mask() );
    }
    for( auto it : reduction_map ) {
         ReductionInfo * info_ptr = it.second;

         for( int i = 0 ; i < table_column_num ; i++ ) 
            key_ptr[i] = HashFeatureTable::HashCombine( key_ptr[i] , info_ptr[i].get_mask() );
    }

    return key_ptr;
} 


void combin_same_feature_together( std::unordered_map<size_t, std::vector<int>> & same_feature_map , size_t * hash_info_ptr , const int table_column_num ) {
    
    for( int i = 0 ; i < table_column_num ; i++ ) {
/*         if( same_feature_map.find( hash_info_ptr[i] ) == same_feature_map.end() ) {
               
             same_feature_map[hash_info_ptr[i]] = std::vector<int>();
         }
         */
         same_feature_map[ hash_info_ptr[ i ] ].push_back( i );
    }
}
void combin_same_write_pattern_together_elem( std::vector<std::pair<int,int>> &same_write_range , std::vector<int> & index_vec,const ScatterInfo * info_ptr ) {
    const unsigned int index_vec_size = index_vec.size();
    std::vector<int> new_index_vec; 
    if(index_vec.size() == 0 ) {
        return;
    }
    const ScatterInfo & scatter_info = info_ptr[index_vec[0]];
    if( scatter_info.order_type_ == OrderEquel ) {
        std::vector<int> range_vec;
        for( unsigned int i = 0 ; i < index_vec_size ; ) {
            int len = 1;
            int cur_index = index_vec[i];
            int cur_write_local = info_ptr[cur_index].data_index_[0];
            unsigned int j = i + 1;
            int cur_index_tmp;
            for(  ; j < index_vec_size ; j++ ) {
                cur_index_tmp = index_vec[j];
                int cur_write_local_tmp = info_ptr[cur_index_tmp].data_index_[0];
                if( cur_write_local_tmp == cur_write_local && cur_index_tmp == cur_index + 1 ){ 
                    len++;
                    cur_index = cur_index_tmp;
                }
                else
                    break;
            }
            i = j;
            range_vec.push_back( len );
        }
        int begin_index = 0;
        for( auto range : range_vec ) {
            if(range == 1) {
                new_index_vec.push_back(index_vec[ begin_index ]);

            } else { 
                same_write_range.push_back( std::pair<int,int>(index_vec[begin_index], index_vec[begin_index+range-1] + 1  ) );
            }
            begin_index += range;
        }

        if( new_index_vec.size() != index_vec_size) {
              for(unsigned int i = 0 ; i < new_index_vec.size(); i++ )
                  index_vec[i] = new_index_vec[i];
              index_vec.erase( index_vec.begin() + new_index_vec.size() , index_vec.end() );
//            index_vec.swap( new_index_vec );
        }
    }
}

void combin_same_write_pattern_together( std::unordered_map<size_t,std::vector<int>> & same_feature_map, std::unordered_map<size_t,std::vector<std::pair<int,int>>> & same_write_pattern_map, const ScatterInfo * info ) {
    std::vector<size_t> erase_vec;
    for(  auto & it : same_feature_map ) {
          const size_t & feature_hash = it.first;
          combin_same_write_pattern_together_elem( same_write_pattern_map[feature_hash], it.second  , info);

          if( same_write_pattern_map[feature_hash].size() == 0 ) {
                same_write_pattern_map.erase(feature_hash);  
          }
          if( it.second.size() == 0 ) {
                erase_vec.push_back(it.first);
          }
    }
}




    //////////////////
    void generate_information( 
        const std::set<std::string> & scatter_set,
        const std::set<std::string> & reduction_set,
        const std::set<std::string> & gather_set,
        const std::map<std::string, void*> & name2ptr_map,
        const int table_column_num,
        std::map<std::string,GatherInfo*> &gather_map,
        std::map<std::string,ScatterInfo*> &scatter_map,
        std::map<std::string,ReductionInfo*> &reduction_map,
        std::unordered_map<size_t,std::vector<int>> &same_feature_map,
        std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
        const int vector
        ) {
        //////Init feature table
        std::string output_name;
        for( const auto & scatter_index : scatter_set ) {
            output_name = scatter_index;
            ScatterInfo * scatter_info_tmp = new ScatterInfo[table_column_num] ;

            auto name2ptr_map_it = name2ptr_map.find(scatter_index);

            generate_info( scatter_info_tmp , (int*)name2ptr_map_it->second , table_column_num,vector );
            scatter_map[ scatter_index ] = scatter_info_tmp;
            ////////////reduction
            if( reduction_set.find(scatter_index) != reduction_set.end()  ) {

                ReductionInfo * reduction_info_tmp = new ReductionInfo[ table_column_num ];

                generate_info( reduction_info_tmp, (int*)name2ptr_map_it->second, table_column_num ,scatter_info_tmp ,vector);

                reduction_map[scatter_index] = reduction_info_tmp; 
                gather_map[ scatter_index ] = scatter_info_tmp; 
            }
        }
        for( const auto & gather_index : gather_set ) {

            GatherInfo * gather_info_tmp = new GatherInfo[table_column_num];
            auto name2ptr_map_it = name2ptr_map.find(gather_index);
            generate_info( gather_info_tmp , (int*)name2ptr_map_it->second , table_column_num,vector );

            gather_map[ gather_index ] = gather_info_tmp;
        }
        /////////////////
        size_t * hash_table = get_feature_hash(  gather_map,scatter_map,reduction_map,table_column_num );
        /////////////////combin same feature
        combin_same_feature_together( same_feature_map, hash_table,table_column_num);
        ////combine same write location pattern
        ///assume that the number of scatter or store operation is only one
        /////

        const int scatter_num = scatter_set.size();
        CHECK(scatter_num == 1 || scatter_num == 0) << "scatter_num should be 1 or 0";
        if(scatter_num == 1) {

            ScatterInfo * scatter_info_tmp = scatter_map[ output_name ];
            CHECK(scatter_info_tmp!=NULL);
            combin_same_write_pattern_together( same_feature_map, same_feature_range_map, scatter_info_tmp);
        }

    }


    std::ostream &  operator << (std::ostream & stream , const OrderType &order_type) {
        std::string order_type_str [] = {
            "DisOrder",
            "Inc",
            "Dec",
            "IncContinue",
            "DecContinue",
            "OrderEquel"
        }; 
        stream << order_type_str[ order_type ];
        return stream;
    }
