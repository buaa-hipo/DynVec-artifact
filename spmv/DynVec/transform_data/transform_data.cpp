#include "transform_data.hpp"
#include "util.h"
class TransformData {
    //data need to be transform
    const std::map<std::string, Type > & name_type_map_;
    const std::map<std::string,void*> & name_ptr_map_;
    const std::set<std::string > & gather_set_;
    const std::set<std::string> & scatter_set_;
    const std::set<std::string> & reduction_set_;

    const std::set<std::string> & load_set_;
    const std::map<std::string,GatherInfo*> &gather_map_;
    const std::map<std::string,ScatterInfo*> &scatter_map_;
    const std::map<std::string,ReductionInfo*> &reduction_map_;

    ////output
    std::map<std::string,int*> & gather_name_new_ptr_map_;
    std::map<std::string,int*> & reduction_name_new_ptr_map_;
    std::map<std::string,int*> & scatter_name_new_ptr_map_;

    std::map<std::string,void*> & name_new_ptr_map_;

    //index
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map_;
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map_;

    const int table_column_num_;

    const int vector_;
        ///
    #define RA_ARRAGE_ELEM( TYPE ) \
    int rearrange_elem(const int index, TYPE * data_ptr,int i, TYPE * new_data_ptr) { \
        int inner_i ;\
        for( inner_i = 0 ; inner_i < vector_ ; inner_i++ ) {\
            new_data_ptr[ i * vector_ + inner_i ] = data_ptr[index * vector_ + inner_i];\
        }\
        i++;\
        return i; \
    } \
    TYPE * malloc_new_data( TYPE * data_ptr ) { \
        TYPE * new_data_ptr = new TYPE[ table_column_num_ * vector_];\
        if(new_data_ptr == NULL) LOG(FATAL)  << "malloc failed";\
        return new_data_ptr;\
    }

    RA_ARRAGE_ELEM(float);
    
    RA_ARRAGE_ELEM(double);
    RA_ARRAGE_ELEM(int);
    int rearrange_elem(const int index, const ReductionInfo * data_ptr,int i,int * new_data_ptr) {
                const ReductionInfo & reduction_info_tmp = data_ptr[ index ];
                if( reduction_info_tmp.order_type_ == DisOrder ) {
                    const int mask = reduction_info_tmp.get_mask() & VEC_MASK_MAX;
                    for( int mask_i = 0 ; mask_i < mask ; mask_i++ ) {
                        for( int disorder_i = 0 ; disorder_i < (vector_ >> 2) ; disorder_i++) {
                            new_data_ptr[ i  ] = reduction_info_tmp.reduction_addr_[mask_i].int_data_vec_[disorder_i];
                            i++;
                        }
                    }
                } else if(reduction_info_tmp.order_type_ == IncContinue || reduction_info_tmp.order_type_ == OrderEquel){
                    
                } else  {
                    LOG(FATAL) << "Unsupported";
                }
            return i;
    }
    int * malloc_new_data( ReductionInfo * data_ptr ) {
        int need_data_num = 0;
        for( int i = 0 ; i < table_column_num_ ; i++ ) {
            if( data_ptr[i].order_type_ == DisOrder ) {
                const int mask = data_ptr[i].get_mask() & VEC_MASK_MAX;
                if( mask != vector_ )
                    need_data_num += sizeof( MASK ) * (vector_) * mask  / sizeof(int) ;            
            } else if(data_ptr[i].order_type_ == IncContinue || data_ptr[i].order_type_ == OrderEquel)  {
            
            } else {
                LOG(FATAL) << "Unsupported";
            } 
            
        }
        int * new_data_ptr = NULL;
        if( need_data_num > 0 ) 
            new_data_ptr = ( int* )malloc(sizeof( int ) *  need_data_num );
        return new_data_ptr;
    }
    int * malloc_new_data( GatherInfo * data_ptr ) {
        int need_data_num = 0;
        for( int i = 0 ; i < table_column_num_ ; i++ ) {
            if( data_ptr[i].order_type_ == DisOrder ) {
                const int mask = data_ptr[i].get_mask() & VEC_MASK_MAX;
                need_data_num += mask;
                if( mask != vector_ && vector_ > VECTOR4 )
                    need_data_num += sizeof( MASK )  * vector_ / sizeof(int) ;            
            } else if( data_ptr[i].order_type_ == IncContinue ){
                need_data_num ++; 
            } else if( data_ptr[i].order_type_ == OrderEquel ){
                need_data_num++; 
            } else {
                LOG(FATAL) << "Unsupported" << i<<data_ptr[i].order_type_;
            }
        } 
        int * new_data_ptr = NULL;
        if( need_data_num > 0 ) 
            new_data_ptr = ( int* )malloc(sizeof( int ) *  need_data_num );
        return new_data_ptr;
    }
    int rearrange_elem(const int index, GatherInfo * data_ptr,int i,int * new_data_ptr) {
                const GatherInfo & gather_info_tmp = data_ptr[ index ];
                if( gather_info_tmp.order_type_ == DisOrder ) {
                    const int mask = gather_info_tmp.get_mask() & VEC_MASK_MAX;
                    if( vector_ > VECTOR4 ) {
                    for( int mask_i = 0 ; mask_i < mask ; mask_i++,i++) { 
                        new_data_ptr[ i  ] = gather_info_tmp.data_index_[mask_i];
                    }
                    if( mask != vector_  ) {
                        for( int disorder_i = 0 ; disorder_i < (vector_ >> 2) ; disorder_i++) {
                            new_data_ptr[ i  ] = gather_info_tmp.disorder_addr_.int_data_vec_[disorder_i];
                            i++;
                        }
                    }
                    } else {
                        if( mask == VECTOR4 ) {
                            for( int mask_i = 0 ; mask_i < mask ; mask_i++,i++) { 
                                new_data_ptr[ i  ] = gather_info_tmp.data_index_[mask_i];
                            }
                        } else {
                            new_data_ptr[i] = gather_info_tmp.data_index_[0];
                            i++;
                        } 
                    }

                } else if( gather_info_tmp.order_type_ == IncContinue || gather_info_tmp.order_type_ == OrderEquel ) {
                    new_data_ptr[i] = gather_info_tmp.data_index_[0];
                    i++;
                } else {
                    LOG(FATAL) << "Unsupported";
                }
            return i;
    }

    int * malloc_new_data( ScatterInfo * data_ptr ) {

        int need_data_num = 0;
        for( const auto & same_feature_it : same_feature_map_ ) {
            const std::vector<int> & vec_tmp = same_feature_it.second;
            for( auto index : vec_tmp ) {
                if( data_ptr[index].order_type_ == DisOrder ) {
                    const int mask = data_ptr[index].get_mask() & VEC_MASK_MAX;
                    need_data_num += mask;
                    if( mask != vector_ )
                        need_data_num += sizeof( MASK ) * vector_ / sizeof(int) ;            
                } else if( data_ptr[index].order_type_ == IncContinue ){
                    need_data_num ++; 
                } else if( data_ptr[index].order_type_ == OrderEquel ){
                    need_data_num++; 
                } else {
                    LOG(FATAL) << "Unsupported" << data_ptr[index].order_type_;
                }        
            }
        }
        for( const auto & same_feature_range_it : same_feature_range_map_ ) {
            const std::vector<std::pair<int,int>> & vec_tmp = same_feature_range_it.second;
            for( auto index_pair : vec_tmp ) {
                int index = index_pair.first;
                if( data_ptr[index].order_type_ == DisOrder ) {
                    const int mask = data_ptr[index].get_mask() & VEC_MASK_MAX;
                    need_data_num += mask;
                    if( mask != vector_ )
                        need_data_num += sizeof( MASK ) * vector_ / sizeof(int) ;            
                } else if( data_ptr[index].order_type_ == IncContinue ){
                    need_data_num ++; 
                } else if( data_ptr[index].order_type_ == OrderEquel ){
                    need_data_num++; 
                } else {
                    LOG(FATAL) << "Unsupported" << data_ptr[index].order_type_;
                }                 
            }
        }
        int * new_data_ptr = NULL;
        if( need_data_num > 0 ) 
            new_data_ptr = ( int* )malloc(sizeof( int ) *  need_data_num );
        return new_data_ptr;
    }

public:
    ///Construct function
    TransformData(
    const std::map<std::string, Type > & name_type_map,
    const std::map<std::string,void*> & name_ptr_map,
    const std::set<std::string > & gather_set,
    const std::set<std::string> & scatter_set,
    const std::set<std::string> & reduction_set,
    const std::set<std::string> & load_set,
    const std::map<std::string,GatherInfo*> &gather_map,
    const std::map<std::string,ScatterInfo*> &scatter_map,
    const std::map<std::string,ReductionInfo*> &reduction_map,
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
    ///output
    std::map<std::string,int*> & gather_name_new_ptr_map,
    std::map<std::string,int*> & reduction_name_new_ptr_map,
    std::map<std::string,int*> & scatter_name_new_ptr_map,
    std::map<std::string,void*> & name_new_ptr_map,
    const int table_column_num,
    const int vector
            ) : 
        name_type_map_(name_type_map),
        name_ptr_map_(name_ptr_map),
        gather_set_( gather_set ),
        scatter_set_(scatter_set),
        reduction_set_(reduction_set),
        load_set_(load_set),
        gather_map_(gather_map),
        scatter_map_(scatter_map),
        reduction_map_(reduction_map),
        gather_name_new_ptr_map_(gather_name_new_ptr_map),
        reduction_name_new_ptr_map_(reduction_name_new_ptr_map),
        scatter_name_new_ptr_map_(scatter_name_new_ptr_map),
        name_new_ptr_map_(name_new_ptr_map),
        same_feature_map_(same_feature_map),
        same_feature_range_map_(same_feature_range_map),
        table_column_num_(table_column_num),
        vector_(vector)
    {
    }
    int * rearrange_scatter( ScatterInfo * data_ptr ) {
       int * new_data_ptr = malloc_new_data(data_ptr);
       if(new_data_ptr != NULL) { 
       int i = 0;
        for( const auto & same_feature_it : same_feature_map_ ) {
            const std::vector<int> & vec_tmp = same_feature_it.second;
            for( auto index : vec_tmp ) {
                 
                i = rearrange_elem( index, data_ptr, i, new_data_ptr) ;
            }
        }
        for( const auto & same_feature_range_it : same_feature_range_map_ ) {
            const std::vector<std::pair<int,int>> & vec_tmp = same_feature_range_it.second;
            for( auto index_pair : vec_tmp ) {
                int begin = index_pair.first;
                i = rearrange_elem( begin, data_ptr, i, new_data_ptr) ;
            }
        }
        }
       return new_data_ptr;
    }

    template<typename R,typename InfoT>
    R * rearrange( InfoT * data_ptr ) {
       R * new_data_ptr = malloc_new_data(data_ptr);
       if(new_data_ptr != NULL) { 
       int i = 0;
        for( const auto & same_feature_it : same_feature_map_ ) {
            const std::vector<int> & vec_tmp = same_feature_it.second;
            for( auto index : vec_tmp ) {
                i = rearrange_elem( index, data_ptr, i, new_data_ptr) ;
            }
        }
        for( const auto & same_feature_range_it : same_feature_range_map_ ) {
            const std::vector<std::pair<int,int>> & vec_tmp = same_feature_range_it.second;
            for( auto index_pair : vec_tmp ) {
                int begin = index_pair.first;
                int end = index_pair.second;
                for( int index_i = begin ; index_i < end; index_i++ ) {

                    i = rearrange_elem( index_i, data_ptr, i, new_data_ptr) ;
                }
            }
        }
        }
       return new_data_ptr;
    }
    
    //
    void rearrange_all() {
        for( const auto & data_name : load_set_) {
            const auto & name_type_it = name_type_map_.find(data_name);
            CHECK(name_type_it != name_type_map_.end());
            const auto & name_ptr_map_it = name_ptr_map_.find( data_name );
            if( name_ptr_map_it == name_ptr_map_.end() ) {
                LOG(FATAL) << "can not find the type of "<< data_name;
            }
            const Type var_type = name_type_it->second;
            const void * addr = name_ptr_map_it->second;

            void * data_new;
            if(var_type == __float_ptr) {
                float * data_new_float = rearrange<float,float>( (float*)addr );
                data_new = (void*)data_new_float;
            } else if(var_type == __double_ptr){
                
                double * data_new_double = rearrange<double,double>( (double*)addr );
                data_new = (void*)data_new_double;
            } else if(var_type == __int_ptr) { 
                int * data_new_int = rearrange<int,int>( (int*)addr );

                data_new = (void*)data_new_int;
            } else {
                LOG(FATAL) << "Unsupported";
            }
            name_new_ptr_map_[ data_name ] = data_new;
        }
        //LOG(INFO) ;
            for( auto scatter_index : scatter_set_ ) {
                auto scatter_info_it = scatter_map_.find( scatter_index );
                CHECK(scatter_info_it != scatter_map_.end()) << "Can not find "<<scatter_index;
                int * rearrange_scatter_data = rearrange_scatter( scatter_info_it->second );
                scatter_name_new_ptr_map_[ scatter_index ] = rearrange_scatter_data;
                /////////////reduction
                auto reduction_info_it = reduction_map_.find( scatter_index );
                if(reduction_info_it != reduction_map_.end()) {
                    int * rearrange_reduction_data = rearrange<int,ReductionInfo>( reduction_info_it->second );
                    reduction_name_new_ptr_map_[scatter_index] = rearrange_reduction_data;
                    gather_name_new_ptr_map_[scatter_index] = rearrange_scatter_data;
                }
            }
            //LOG(INFO);
            for( auto gather_index : gather_set_ ) {

                auto gather_info_it = gather_map_.find( gather_index );
                CHECK(gather_info_it != gather_map_.end()) << "Can not find "<<gather_index;
                int * rearrange_gather_data = rearrange<int,GatherInfo>( gather_info_it->second );
    
                gather_name_new_ptr_map_[gather_index] = rearrange_gather_data;
            }
    }
};
void transform_data(
    const std::map<std::string, Type > & name_type_map,
    const std::map<std::string,void*> & name_ptr_map,
    const std::set<std::string > & gather_set,
    const std::set<std::string> & scatter_set,
    const std::set<std::string> & reduction_set,
    const std::set<std::string> & load_set,
    const std::map<std::string,GatherInfo*> &gather_map,
    const std::map<std::string,ScatterInfo*> &scatter_map,
    const std::map<std::string,ReductionInfo*> &reduction_map,
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
    std::map<std::string,int*> & gather_name_new_ptr_map,
    std::map<std::string,int*> & reduction_name_new_ptr_map,
    std::map<std::string,int*> & scatter_name_new_ptr_map,
    std::map<std::string,void*> & name_new_ptr_map,
    int table_column_num,
    const int vector
        ) {
    TransformData transform_data( 
            name_type_map,
            name_ptr_map,
            gather_set,
            scatter_set,
            reduction_set,
            load_set,
            gather_map,
            scatter_map,
            reduction_map,
            same_feature_map,
            same_feature_range_map,
            gather_name_new_ptr_map,
            reduction_name_new_ptr_map,
            scatter_name_new_ptr_map,
            name_new_ptr_map,
            table_column_num,
            vector
            );
    //LOG(INFO) << vector;
    transform_data.rearrange_all();
}

