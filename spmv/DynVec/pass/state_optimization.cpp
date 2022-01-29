#include "bit2addr.h"
#include "state_optimization.hpp"
#include "statement_print.hpp"
#include "state_pass.hpp"

#include "state_redirect_var.hpp"
class OptimizationPass : public StateMentPass{
    private:
    protected:
        std::map<std::string, Varience * > gather_scatter_has_load_info_index_;

        std::map<std::string, Varience * > gather_scatter_has_load_info_shuffle_;
        Const * fzero_vec_const_;
        Const * dzero_vec_const_;
        Const * vecnum_int8_v_const_ ;
    public:
    const std::map<std::string,GatherInfo*> &gather_map_;

    const std::map<std::string,ScatterInfo*> &scatter_map_;
    const std::map<std::string,ReductionInfo*> &reduction_map_;
    const std::map<std::string,Varience*> &name_var_map_;
    const std::map< std::string , Varience *> &name_varP_varVP_map_;
    const std::map< std::string , Varience *> &name_varP_varPV_map_;
    const std::map<std::string, Varience*>  &gather_name_new_var_map_;
    const std::map<std::string, Varience*>  &reduction_name_new_var_map_;
    const std::map<std::string, Varience*>  &scatter_name_new_var_map_;
    const std::map<std::string, Varience*>  &name_new_var_map_;

    const int index_;
    const int circle_num_;
    const int vector_;
    OptimizationPass(
        const std::map<std::string,GatherInfo*> &gather_map,
        const std::map<std::string,ScatterInfo*> &scatter_map,
        const std::map<std::string,ReductionInfo*> &reduction_map,
        const std::map<std::string,Varience*> &name_var_map,
        const std::map< std::string , Varience *> &name_varP_varVP_map,
        const std::map< std::string , Varience *> &name_varP_varPV_map,
        const std::map<std::string, Varience*>  &gather_name_new_var_map,
        const std::map<std::string, Varience*>  &reduction_name_new_var_map,
        const std::map<std::string, Varience*>  &scatter_name_new_var_map,
        const std::map<std::string, Varience*>  &name_new_var_map,
        const int index,
        const int circle_num,
        const int vector
    ): 
        gather_map_( gather_map ),
        scatter_map_(scatter_map),
        reduction_map_(reduction_map),
        name_var_map_(name_var_map),
        name_varP_varVP_map_(name_varP_varVP_map),
        name_varP_varPV_map_(name_varP_varPV_map),
        gather_name_new_var_map_(gather_name_new_var_map),
        reduction_name_new_var_map_(reduction_name_new_var_map),
        scatter_name_new_var_map_(scatter_name_new_var_map),
        name_new_var_map_(name_new_var_map),
        index_(index),
        circle_num_(circle_num),
        vector_(vector)
    {
        double dzero_vec[vector_];
        for( int i = 0 ; i < vector_ ; i++ ) {
            dzero_vec[i] = 0;
	    }

        dzero_vec_const_ = new Const( dzero_vec , vector_ );
        float fzero_vec[vector_];
        for( int i = 0 ; i < vector_ ; i++ ) {
            fzero_vec[i] = 0;
	    }

        fzero_vec_const_ = new Const( fzero_vec , vector_ );
        int8_t vecnum_vec[VECTOR16];
        for( int i = 0 ; i < vector_ ; i++ ) 
            vecnum_vec[i] = vector_;
        vecnum_int8_v_const_ = new Const(vecnum_vec, vector_);
    }

virtual    StateMent* pass_(Block * stat ) ;
virtual    StateMent* pass_(Gather * stat   ) ;
virtual    StateMent* pass_(Add * stat   );
virtual    StateMent* pass_(Scatter * stat  ) ;

virtual    StateMent* pass_(Load * stat  ) ;

virtual    StateMent* pass_(LetStat * stat  ) ;
};


StateMent * OptimizationPass::pass_(Block * stat) {
    std::vector<StateMent*> state_vec_new;

    std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    for(unsigned int i = 0 ; i < state_vec->size(); i++  ) {
        StateMent * state = (*state_vec)[i];
        LetStat * let  = dynamic_cast<LetStat*>(state);
        if( let != NULL ) {
            Varience * res_var = let->get_res(); 
            StateMent * state_expr_pass = pass( let->get_expr() );
            if( state_expr_pass != nop_ ) {
            Block * block_tmp = dynamic_cast<Block*>(state_expr_pass);
            if(block_tmp != NULL) {
                     
                std::vector<StateMent* > * state_vec_tmp = block_tmp->get_stat_vec();
                for( unsigned int j = 0 ; j < state_vec_tmp->size(); j++ ) {
                    StateMent * state_inner = (*state_vec_tmp)[j]  ;
                    if( dynamic_cast<LetStat*>(state_inner) != NULL || dynamic_cast<Store*>(state_inner) != NULL || dynamic_cast<Scatter*>(state_inner) != NULL || dynamic_cast<Print*>(state_inner) != NULL ) {
                         
                        state_vec_new.push_back( state_inner );
                    } else {
                        res_var->set_type( state_inner->get_type()  );
                        state_vec_new.push_back( LetStat::make( res_var,state_inner ) );    
                    }
                }
            } else {

                res_var->set_type( state_expr_pass->get_type()  );
                state_vec_new.push_back( LetStat::make( res_var,state_expr_pass ) );    
            }
            }
        } else {
            StateMent * state_pass = pass( state  );
            state_vec_new.push_back( state_pass ); 
        }
    }
//    StateMent * block_state = StateMentPass::pass_(stat);
    return For::make( new Const(0), new Const(1), new Const( circle_num_ ),  CombinStatVec( state_vec_new ) ); 
}
StateMent * OptimizationPass::pass_(Gather * stat) {
    const std::string & index_name = stat->index_name_;
    auto gather_map_it = gather_map_.find(index_name);
    ////LOG(INFO) << index_name ;
    if( gather_map_it != gather_map_.end() ) {
        const std::string & index_name = stat->index_name_;

        GatherInfo gather_info = (gather_map_it->second)[index_];
        //LOG(INFO) << gather_info;
        Varience * load_index;

        std::string addr_name = stat->addr_name_;
        std::vector<StateMent *> gather_state_vec;
        const auto & gather_name_new_var_map_it = gather_name_new_var_map_.find(index_name);
        CHECK(gather_name_new_var_map_it!=gather_name_new_var_map_.end()) << "unfind ";
        Varience * gather_info_var = const_cast<Varience*>(gather_name_new_var_map_it->second);

        if( gather_info.order_type_ == DisOrder ) {
            int mask = gather_info.get_mask() & VEC_MASK_MAX;
            //LOG(INFO) << mask;
            if( mask == 1 || ( vector_ == VECTOR4 && mask != vector_ ) ) {
                Varience * permulation_addr;
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {

                    load_index = new Varience( __int );
                
                    gather_state_vec.push_back( LetStat::make( load_index , Load::make( gather_info_var ) ));
                    gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(1) ) )); 
//                    gather_state_vec.push_back( Print::make( load_index ) );
                    //warning 
                    //
                    permulation_addr = new Varience( __dynvec_int8_v );

                    if(vector_ == VECTOR8) {

                        gather_state_vec.push_back( LetStat::make( permulation_addr , Load::make(BitCast::make( gather_info_var, __dynvec_int8_v_ptr )) ));
                        gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(2) ) ));
                    } else if(vector_ == VECTOR16){

                        gather_state_vec.push_back( LetStat::make( permulation_addr , Load::make(BitCast::make( gather_info_var, __dynvec_int8_v_ptr )) ));
                        gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(4) ) ));
                    } else if (vector_ == VECTOR4){
                           
                       // gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(1) ) ));
                    } else {
                        LOG(FATAL) << "Unsupported";
                    }
                    gather_scatter_has_load_info_index_[ index_name ] = load_index;
                    gather_scatter_has_load_info_shuffle_[index_name] = permulation_addr;
                } else {
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                    permulation_addr = gather_scatter_has_load_info_shuffle_[index_name];
                     
                }
    
                auto name_var_map_it = name_var_map_.find(addr_name);
                CHECK(name_var_map_it != name_var_map_.end()) << "can not find " << addr_name;
                
                Varience * load_data = new Varience( stat->get_type() );
                gather_state_vec.push_back( LetStat::make( load_data , Load::make( BitCast::make( IncAddr::make( name_var_map_it->second,load_index  ) , type_scalar_ptr2vector_ptr( name_var_map_it->second->get_type() )  ) )));

//                gather_state_vec.push_back( Print::make( load_data ) );
//                gather_state_vec.push_back( Print::make( load_data ) );
//                gather_state_vec.push_back( Print::make( permulation_addr ) );
                if( vector_ > VECTOR4 ) {
                    gather_state_vec.push_back(Shuffle::make( load_data , permulation_addr) );
                } else {
                    if(vector_ == VECTOR4) {
                        if(stat->get_type().get_data_type() == DOUBLE) {
                            int shuffle_vec[VECTOR4];
                            shuffle_vec[0] = mask & 0xf;
                            shuffle_vec[1] = (mask>>4) & 0xf;

                            shuffle_vec[2] = (mask>>8) & 0xf;
                            shuffle_vec[3] = (mask>>12) & 0xf;
                            gather_state_vec.push_back(Shuffle::make( load_data , dzero_vec_const_ ,new Const(shuffle_vec ,VECTOR4) ));
                        } else {
                            LOG(FATAL) << "Unsupported";
                        }
                    } else {
                        LOG(FATAL) << "Unsupported";
                    } 
                }

                return CombinStatVec( gather_state_vec );

            } else if (mask==vector_) { 
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {
                    load_index = new Varience( __int_v );


                    gather_state_vec.push_back( LetStat::make( load_index , Load::make( BitCast::make( gather_info_var, type_scalar_ptr2vector_ptr(__int_ptr)) )));
                    gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(vector_) ) )); 
                     gather_scatter_has_load_info_index_[ index_name ] = load_index;
                } else {
                     
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                }
                const auto & name_varP_varPV_map_it = name_varP_varPV_map_.find(addr_name);
                CHECK( name_varP_varPV_map_it != name_varP_varPV_map_.end() ) ;

//                gather_state_vec.push_back( Print::make( load_index )  );
                gather_state_vec.push_back( Gather::make( const_cast<Varience*>(name_varP_varPV_map_it->second),load_index ));
                return CombinStatVec( gather_state_vec );
            } else {

                LOG(FATAL) << "Unsupported";
                return nop_;
            }
        } else if ( gather_info.order_type_ == IncContinue)  {
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {

                    load_index = new Varience( __int );
                    gather_state_vec.push_back( LetStat::make( load_index , Load::make( gather_info_var)));
                    gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(1) ) )); 
                     gather_scatter_has_load_info_index_[ index_name ] = load_index;
                } else {
                     
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                }
                //LOG(INFO);
                int mask = gather_info.get_mask() & VEC_MASK_MAX;
                Bit2Addr bit2addr( vector_);
                CompressAddr compress_addr = bit2addr.generate_compress( mask );
                Const * compress_mem_mask_const = new Const( compress_addr.mask_);
                const auto & name_var_map_it = name_var_map_.find(addr_name);
                if(scatter_map_.find( index_name ) == scatter_map_.end() ) {

                    StateMent * load_data_state = Load::make( BitCast::make( IncAddr::make( name_var_map_it->second, load_index),type_scalar_ptr2vector_ptr(name_var_map_it->second->get_type())), compress_mem_mask_const ) ;
                    int shuffle_index_vec[vector_];
                    int shuffle_index_vec_index = -1;
                    for( int shuffle_index_vec_i = 0 ; shuffle_index_vec_i < vector_ ; shuffle_index_vec_i++  ) {
                        if( (mask & (1<<shuffle_index_vec_i)) != 0 ) {
                            shuffle_index_vec_index++;
                        }
                        CHECK(shuffle_index_vec_index>=0);
                        shuffle_index_vec[ shuffle_index_vec_i ] = shuffle_index_vec_index; 
                    }
                    //LOG(INFO) << mask;
                    if( (mask & VEC_MASK_MAX) == ( 0xffffffff >> (32-vector_) )  ) {
                     //   LOG(INFO) << load_data_state->get_type();
                        gather_state_vec.push_back( load_data_state );
                    } else {
                        if(stat->get_type().get_data_type() == DOUBLE) {
                            gather_state_vec.push_back( Shuffle::make( load_data_state, dzero_vec_const_, new Const( shuffle_index_vec, vector_ ) ) );
                        } else {
                            if( stat->get_type().get_data_type() == FLOAT ) {
                                gather_state_vec.push_back( Shuffle::make( load_data_state, fzero_vec_const_, new Const( shuffle_index_vec, vector_ ) ) );
                            }
                        }
                    }
                } else {
                    gather_state_vec.push_back(  Load::make( BitCast::make( IncAddr::make( name_var_map_it->second, load_index),type_scalar_ptr2vector_ptr(name_var_map_it->second->get_type())), compress_mem_mask_const ) );
                }
//                gather_state_vec.push_back(  Load::make( BitCast::make( IncAddr::make( name_var_map_it->second, load_index),type_scalar_ptr2vector_ptr(name_var_map_it->second->get_type())), compress_mem_mask_const ) );
                return CombinStatVec(gather_state_vec);

        } else if( gather_info.order_type_ == OrderEquel ){
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {

                    load_index = new Varience( __int );
                    gather_state_vec.push_back( LetStat::make( load_index , Load::make( gather_info_var)));
                    gather_state_vec.push_back( LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(1) ) )); 
                     gather_scatter_has_load_info_index_[ index_name ] = load_index;
                } else {
                     
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                }
               //gather_state_vec.push_back(Print::make( load_index ));
                const auto & name_var_map_it = name_var_map_.find(addr_name);
                StateMent * load_state = Load::make(IncAddr::make( name_var_map_it->second, load_index));
                if(scatter_map_.find( index_name ) == scatter_map_.end() ) {
                    gather_state_vec.push_back(  BroadCast::make( load_state ,vector_));
                } else {
                    gather_state_vec.push_back( load_state ) ;
                }
                return CombinStatVec(gather_state_vec);
        } else {
            LOG(FATAL) << "does not supported";
            return nop_;
        }
    } else {
        //doesn't have gather information, not optimize it
        return StateMentPass::pass_(stat); 
    }
}
StateMent * OptimizationPass::pass_(Add * stat ) { 
        std::string  index_name = stat->index_name_;
        const auto& reduction_map_it = reduction_map_.find( index_name);
         
                //LOG(INFO);
        if( reduction_map_it != reduction_map_.end() ) {
            StateMent * v1_state_new = pass( stat->get_v1());
            StateMent * v2_state_new = pass( stat->get_v2());
            

            std::vector<StateMent *> reduce_state_vec;
//            reduce_state_vec.push_back( Print::make( v2_state_new ) );
 //           Varience * shuffle_res = new Varience(v2_state_new->get_type()); 
//            StateMent * add_new_state = LetStat::make( shuffle_res, v2_state_new);
            ReductionInfo reduction_info = (reduction_map_it->second)[index_];
            
            const auto & reduction_name_new_var_map_it = reduction_name_new_var_map_.find( index_name );
            Varience * reduction_info_var = const_cast<Varience*>(reduction_name_new_var_map_it->second);
            if(reduction_info.order_type_ == IncContinue) {
                int mask = reduction_info.get_mask()&VEC_MASK_MAX;
                Bit2Addr bit2addr(vector_);
                TransAddr trans_addr = bit2addr.generate( mask );
                int reduce_num = trans_addr.num;
                int * reduce_addr_int = (int*)malloc(sizeof(int)* vector_ * reduce_num );
                const char * reduce_addr = trans_addr.addr;
                for( int i = 0 ; i < reduce_num ; i++ ) {
                    for( int j = 0 ; j < vector_ ; j++ ) {
                        reduce_addr_int[ i* vector_ + j ] = (int)reduce_addr[ i * vector_ + j ];
                    }
                }
                std::vector<Const*> shuffle_index_const_vec;
                for( int i = 0 ; i < reduce_num ; i++ )  {
                    shuffle_index_const_vec.push_back( new Const( reduce_addr_int+ i*vector_,vector_ ) );
                }
                Type v2_state_new_type = v2_state_new->get_type(); 
                Varience *shuffle_res = new Varience( v2_state_new_type,false );
                reduce_state_vec.push_back( LetStat::make( shuffle_res ,v2_state_new ) );
                DataType v2_state_new_type_basic_data_type = v2_state_new_type.get_data_type();
                for( int reduce_i = 0 ; reduce_i < reduce_num ; reduce_i++ ) {
                    Varience * shuffle_simd = new Varience( v2_state_new_type );
                    if(v2_state_new_type.get_data_type() == DOUBLE) { 
                        reduce_state_vec.push_back(LetStat::make( shuffle_simd, Shuffle::make( shuffle_res , dzero_vec_const_, shuffle_index_const_vec[reduce_i] )));
                    } else if(v2_state_new_type.get_data_type() == FLOAT) {
                    
                        reduce_state_vec.push_back(LetStat::make( shuffle_simd, Shuffle::make( shuffle_res , fzero_vec_const_, shuffle_index_const_vec[reduce_i] )));
                    } else {
                        LOG(FATAL) << "Unsupported";
                    }
                    reduce_state_vec.push_back(LetStat::make( shuffle_res, Add::make( shuffle_simd, shuffle_res ) ));

                }
                CompressAddr compress_addr = bit2addr.generate_compress( mask );

                Const * compress_const = new Const( compress_addr.compress_vec, vector_ );
                
                if(v2_state_new_type_basic_data_type == DOUBLE) { 
                    reduce_state_vec.push_back(LetStat::make( shuffle_res, Shuffle::make( shuffle_res , dzero_vec_const_, compress_const ) ));
                } else if(v2_state_new_type_basic_data_type == FLOAT) {
                
                    reduce_state_vec.push_back(LetStat::make( shuffle_res, Shuffle::make( shuffle_res , fzero_vec_const_, compress_const )));
                } else {
                
                        LOG(FATAL) << "Unsupported";
                }

                 reduce_state_vec.push_back(Add::make( v1_state_new, shuffle_res)) ;
                 return CombinStatVec(reduce_state_vec);
            } else if(reduction_info.order_type_ == OrderEquel) {
                    
                 StateMent * ret = Add::make(v1_state_new,Reduce::make( v2_state_new)) ;
                 return ret;
                 
            } else if( reduction_info.order_type_ == DisOrder ){
                const int reduce_num = reduction_info.get_mask() & VEC_MASK_MAX ;
                Varience *shuffle_res = new Varience( v2_state_new->get_type(),false );
                reduce_state_vec.push_back( LetStat::make( shuffle_res ,v2_state_new ) );

                for( int reduce_i = 0 ; reduce_i < reduce_num ; reduce_i++ ) {
                    Varience * permulation_addr;
                    permulation_addr = new Varience( __dynvec_int8_v );
                    reduce_state_vec.push_back( LetStat::make( permulation_addr , Load::make(BitCast::make( reduction_info_var, __dynvec_int8_v_ptr )) ));

                    if(vector_ == VECTOR8) {
                        reduce_state_vec.push_back( LetStat::make( reduction_info_var , IncAddr::make( reduction_info_var, new Const(2) ) ));
                    } else if (vector_ == VECTOR16){
                                                
                        reduce_state_vec.push_back( LetStat::make( reduction_info_var , IncAddr::make( reduction_info_var, new Const(4) ) ));

                    } else if(vector_ == VECTOR4){

                        reduce_state_vec.push_back( LetStat::make( reduction_info_var , IncAddr::make( reduction_info_var, new Const(1) ) ));
                    } else {
                        LOG(FATAL) << "Unsupported";
                    }

                    Varience *shuffle_simd = new Varience( v2_state_new->get_type());
                    reduce_state_vec.push_back(LetStat::make( shuffle_simd, Shuffle::make( shuffle_res ,  permulation_addr)));                  
                    Varience * mask_var = new Varience( __bool_v );
                  //  LOG(INFO);
                    reduce_state_vec.push_back( LetStat::make(mask_var, ICmpEQ::make( permulation_addr, vecnum_int8_v_const_ ) ) );

                    Varience * select_var = new Varience( v2_state_new->get_type());
                    reduce_state_vec.push_back( LetStat::make(select_var, Select::make(  fzero_vec_const_,shuffle_simd, mask_var ) ) );
                    reduce_state_vec.push_back(LetStat::make( shuffle_res, Add::make( select_var, shuffle_res ) ));
                }

                reduce_state_vec.push_back(Add::make( v1_state_new, shuffle_res)) ;


                return CombinStatVec(reduce_state_vec);
            } else {
                LOG(FATAL) << "Unsupported";
                return nop_;
            }
        } else {
            return StateMentPass::pass_(stat); 
        }
}
StateMent * OptimizationPass::pass_(LetStat * stat) {
    Varience * res_var = stat->get_res();
    StateMent * expr_state = stat->get_expr();
    StateMent * expr_state_new = pass(expr_state);
    if( expr_state_new != nop_ ) {
    Block * expr_block = dynamic_cast<Block*>(expr_state_new);
    if( expr_state_new == expr_state) {

        return stat;
    } else if ( expr_block != NULL ){
        
        std::vector<StateMent* > * state_vec = expr_block->get_stat_vec() ;
        std::vector<StateMent *> * new_state_vec = new std::vector<StateMent*>();
        new_state_vec->resize( state_vec->size() , nullptr);
        for(unsigned int i = 0 ; i < state_vec->size() ; i++) {
            StateMent * state_inner = (*state_vec)[i];
            if( dynamic_cast<LetStat*>(state_inner) != NULL || dynamic_cast<Store*>(state_inner) != NULL || dynamic_cast<Scatter*>(state_inner) != NULL || dynamic_cast<Print*>(state_inner) != NULL ) {
                (*new_state_vec)[i] = (*state_vec)[i]; 
            } else {
                res_var->set_type( state_inner->get_type()  );
//                LOG(INFO) << res_var;
//                LOG(INFO) << state_inner; 
                (*new_state_vec)[i] = LetStat::make(res_var,state_inner);
            }
        }
        return Block::make(new_state_vec);
    } else {
        res_var->set_type( expr_state_new->get_type()  );
        return LetStat::make(res_var,expr_state_new);
    }
    } else{
        return nop_;
    }
}

StateMent * OptimizationPass::pass_(Load * stat) {
    const auto & it  = name_new_var_map_.find(stat->addr_name_); 

    if( it != name_new_var_map_.end()) {
        return Block::make( 
            Load::make( BitCast::make( it->second,type_scalar_ptr2vector_ptr(it->second->get_type() ))), 
            LetStat::make( it->second,IncAddr::make(it->second,new Const(vector_)))
        ); 
    } else {
//        StateMent * ret = StateMentPass::pass_(stat); 
        return nop_;
    }
}
StateMent * OptimizationPass::pass_(Scatter * stat) {
    StateMent * new_stat = StateMentPass::pass_(stat); 
    const std::string & index_name = stat->index_name_;
    const auto & scatter_map_it = scatter_map_.find(index_name);

    if( scatter_map_it != scatter_map_.end() ) {
        Scatter * scatter_new_stat = dynamic_cast<Scatter*>( new_stat );     
//        StateMent * addr_state = scatter_new_stat->get_addr();
        StateMent * data_state = scatter_new_stat->get_data();
        const ScatterInfo scatter_info = (scatter_map_it->second)[index_];
        Varience * load_index;
        std::vector<StateMent*> scatter_state_vec;
        std::string addr_name = stat->addr_name_;

        std::string index_name = stat->index_name_;
        const auto &gather_name_new_var_map_it = gather_name_new_var_map_.find(index_name);
        Varience * scatter_info_var = const_cast<Varience*>( gather_name_new_var_map_it->second );
        
        if(scatter_info.order_type_ == IncContinue)  {
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {
                    load_index = new Varience( __int );
                    scatter_state_vec.push_back( LetStat::make( load_index , Load::make( scatter_info_var)));
                    scatter_state_vec.push_back( LetStat::make( scatter_info_var , IncAddr::make( scatter_info_var, new Const(1) ) )); 
                    gather_scatter_has_load_info_index_[ index_name ] = load_index;
                } else {  
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                }

                int mask = scatter_info.get_mask() & VEC_MASK_MAX;
                Bit2Addr bit2addr( vector_);
                CompressAddr compress_addr = bit2addr.generate_compress( mask );

                Const * compress_mem_mask_const = new Const( compress_addr.mask_);
                const auto & addr_name_var_map_it = name_var_map_.find(addr_name);

                CHECK( addr_name_var_map_it != name_var_map_.end() );
                scatter_state_vec.push_back(  Store::make( BitCast::make( IncAddr::make( const_cast<Varience*>(addr_name_var_map_it->second ), load_index),type_scalar_ptr2vector_ptr( addr_name_var_map_it->second->get_type() )) , data_state, compress_mem_mask_const )) ;
                return CombinStatVec(scatter_state_vec);
        } else if (scatter_info.order_type_ == OrderEquel ) {
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {
                    load_index = new Varience( __int );
                    scatter_state_vec.push_back( LetStat::make( load_index , Load::make( scatter_info_var)));
                    scatter_state_vec.push_back( LetStat::make( scatter_info_var , IncAddr::make( scatter_info_var, new Const(1) ) )); 
                    gather_scatter_has_load_info_index_[ index_name ] = load_index;
                } else {  
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                }
                const auto & addr_name_var_map_it = name_var_map_.find(addr_name);
                CHECK( addr_name_var_map_it != name_var_map_.end() );
                scatter_state_vec.push_back(  Store::make( IncAddr::make( const_cast<Varience*>(addr_name_var_map_it->second ), load_index) , data_state)) ;
                return CombinStatVec( scatter_state_vec );
        } else if( scatter_info.order_type_ == DisOrder ) {
            int mask = scatter_info.get_mask() & VEC_MASK_MAX;
            if (mask==vector_) { 
                if( gather_scatter_has_load_info_index_.find(index_name) == gather_scatter_has_load_info_index_.end() ) {
                    load_index = new Varience( __int_v );

                    scatter_state_vec.push_back( LetStat::make( load_index , Load::make( BitCast::make( scatter_info_var, type_scalar_ptr2vector_ptr(__int_ptr)) )));
                    scatter_state_vec.push_back( LetStat::make( scatter_info_var , IncAddr::make( scatter_info_var, new Const(vector_) ) )); 
                    gather_scatter_has_load_info_index_[ index_name ] = load_index;
                } else {
                     
                    load_index = gather_scatter_has_load_info_index_[ index_name ];
                }
                const auto & name_varP_varPV_map_it = name_varP_varPV_map_.find(addr_name);
                CHECK( name_varP_varPV_map_it != name_varP_varPV_map_.end() ) ;

//                gather_state_vec.push_back( Print::make( load_index )  );
                scatter_state_vec.push_back( Scatter::make( const_cast<Varience*>(name_varP_varPV_map_it->second),load_index,data_state ));
                return CombinStatVec( scatter_state_vec );
            } else {
                LOG(FATAL) << "Unsupported";
                return nop_;
            }
        } else {
            LOG(FATAL)  << "Unsupported";
            return nop_;
        }
    } else {
        return new_stat;
    }    
}

class OptimizationInnerReducePass : public OptimizationPass {
    public:
    Varience * range_num_var_;
    const std::string & output_name_;
    OptimizationInnerReducePass(
        const std::map<std::string,GatherInfo*> &gather_map,
        const std::map<std::string,ScatterInfo*> &scatter_map,
        const std::map<std::string,ReductionInfo*> &reduction_map,
        const std::map<std::string,Varience*> &name_var_map,
        const std::map< std::string , Varience *> &name_varP_varVP_map,
        const std::map< std::string , Varience *> &name_varP_varPV_map,
        const std::map<std::string, Varience*>  &gather_name_new_var_map,
        const std::map<std::string, Varience*>  &reduction_name_new_var_map,
        const std::map<std::string, Varience*>  &scatter_name_new_var_map,
        const std::map<std::string, Varience*>  &name_new_var_map,
        const int index,
        const int circle_num,
        Varience * range_num_var,
        const std::string & output_name,
        const int vector
    ):OptimizationPass(
        gather_map,
        scatter_map,
        reduction_map,
        name_var_map,
        name_varP_varVP_map,
        name_varP_varPV_map,
        gather_name_new_var_map,
        reduction_name_new_var_map,
        scatter_name_new_var_map,
        name_new_var_map,
        index,
        circle_num,
        vector
        ), range_num_var_(range_num_var),output_name_(output_name){
    }
    virtual    StateMent* pass_(Block * stat ) ;
};

StateMent * OptimizationInnerReducePass::pass_(Block * stat) {
    /////assume that there is only one scatter/store operation
    std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    std::vector<StateMent* > inner_state_vec ;
;
    std::vector<StateMent* > outer_store_state_vec  ;

    std::vector<StateMent* > inner_reducetion_state_vec ;
    std::vector<StateMent* > outer_reducetion_state_vec ;

    std::vector<StateMent* > outer_init_state_vec;

    Varience * reduce_var ;
    for( auto state : *state_vec ) {
        if( state->node_name_ == output_name_ ) {
            if(dynamic_cast<Scatter*>(state) && state->node_name_ == output_name_ )
                outer_store_state_vec.push_back(state);
            else {
                LetStat * let_stat = dynamic_cast<LetStat*>( state );
                if( let_stat != NULL && let_stat->node_name_ == output_name_) {
                    
                    StateMent * expr_state = let_stat->get_expr();
                    Add * add_state = dynamic_cast<Add*>( expr_state );
                    if(add_state != NULL ) {
                        reduce_var = new Varience( add_state->get_type(),false );
                        if(reduce_var->get_type() == __double_v) {
                            outer_init_state_vec.push_back(LetStat::make(reduce_var,dzero_vec_const_ )) ;
                        } else if(reduce_var->get_type() == __float_v){
                        
                            outer_init_state_vec.push_back(LetStat::make(reduce_var,fzero_vec_const_ )) ;
                        } else {
                            LOG(FATAL) << "Unsupported";
                        }
                        
                        
                        inner_reducetion_state_vec.push_back( LetStat::make(reduce_var,Add::make( reduce_var, add_state->get_v2()))) ;
                        StateMent * add_outer = Add::make(add_state->get_v1(),reduce_var);
                        add_outer->set_index_name( add_state->index_name_ );
                        outer_reducetion_state_vec.push_back( LetStat::make( let_stat->get_res(), add_outer )) ;
                        outer_reducetion_state_vec.back()->set_node_name( output_name_ );
                    } else {
                    
                        Gather * gather_state = dynamic_cast<Gather*>( expr_state );
                        if(gather_state!=NULL ) {
                        
                            outer_init_state_vec.push_back(state) ;
                        
                        } else {
                    
                            LOG(FATAL) << "Unsupported";
                        }
                    }
                } else {
                    LOG(FATAL) << "Unsupported";
                }
            } 
                
        } else {
            inner_state_vec.push_back(state);
        }
    }
    std::vector<StateMent*> inner_state_vec_new ;
    
    for( auto state : inner_state_vec ) {
        StateMent * opt_state_tmp = OptimizationPass::pass( state );
        if( opt_state_tmp != nop_ )
            inner_state_vec_new.push_back(opt_state_tmp);
    }
    for( auto state : inner_reducetion_state_vec ) {
        inner_state_vec_new.push_back( state );
    }
    
    Varience * inner_for_num = new Varience(__int);
    
    StateMent * inner_for = For::make( new Const(0), new Const(1), inner_for_num,  CombinStatVec(inner_state_vec_new  ) );
    //LOG(INFO)  << inner_for;
    std::vector<StateMent*> outer_vec;
    outer_vec.push_back( LetStat::make( inner_for_num, Load::make( range_num_var_ ) ) );

    outer_vec.push_back( LetStat::make( range_num_var_, IncAddr::make( range_num_var_,new Const(1) ) ) );

//    outer_vec.push_back( Print::make(inner_for_num) );
    for (auto state : outer_init_state_vec) {

        outer_vec.push_back( OptimizationPass::pass(state ));
    }
    outer_vec.push_back(inner_for);
    for (auto state : outer_reducetion_state_vec) {
        outer_vec.push_back( OptimizationPass::pass(state ));
    }
    for (auto state : outer_store_state_vec) {
        outer_vec.push_back( OptimizationPass::pass(state ));
    }



    return For::make( new Const(0), new Const(1), new Const( circle_num_ ),  CombinStatVec( outer_vec ) ); 
}
StateMent * optimization_state( 
        const std::map<std::string,GatherInfo*> &gather_map,
        const std::map<std::string,ScatterInfo*> &scatter_map,
        const std::map<std::string,ReductionInfo*> &reduction_map,
        const std::map<std::string,Varience*> &name_var_map,
        const std::map< std::string , Varience *> &name_varP_varVP_map,
        const std::map< std::string , Varience *> &name_varP_varPV_map,
        const std::map<std::string, Varience*>  &gather_name_new_var_map,
        const std::map<std::string, Varience*>  &reduction_name_new_var_map,
        const std::map<std::string, Varience*>  &scatter_name_new_var_map,
        const std::map<std::string, Varience*>  &name_new_var_map,
        const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
        const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&  same_feature_range_map,
        const std::string & output_name,
        StateMent * code_seed,
        const int vector
        ) {
    //LOG(INFO) << code_seed;
       std::vector<StateMent*> state_vec;
       for( const auto & it : same_feature_map ) {
            
            const std::vector<int> & index_vec = it.second;
            const int index_vec_num = index_vec.size();
            OptimizationPass opt_pass = OptimizationPass(
                gather_map,
                scatter_map,
                reduction_map,
                name_var_map,
                name_varP_varVP_map,
                name_varP_varPV_map,
                gather_name_new_var_map,
                reduction_name_new_var_map,
                scatter_name_new_var_map,
                name_new_var_map,
                index_vec[0],
                index_vec_num,
                vector);
            StateMent * opt_state = opt_pass.pass(code_seed);
            StateMent * redirect_opt_state = redirect_var_state(opt_state);
            state_vec.push_back( redirect_opt_state);
       }
       

       //////////////////////////
       std::vector<int> same_feature_range_map_vec;
       
       for( const auto & it : same_feature_range_map ) {

            const std::vector<std::pair<int,int>> & index_vec = it.second;
            for( const auto & index_it : index_vec ) {
               int range_size = index_it.second - index_it.first;
               same_feature_range_map_vec.push_back( range_size );
            }
       }
       int * same_feature_range_map_ptr = (int*)malloc(sizeof(int)*same_feature_range_map_vec.size());
       int i = 0;
       for( auto it : same_feature_range_map_vec ) {
            same_feature_range_map_ptr[i] = it; 
            i++;
       }
       if( same_feature_range_map_vec.size() > 0 ) {
       Const * range_num_const = new Const( (uint64_t) same_feature_range_map_ptr );
       Varience * range_num_var = new Varience(__int_ptr,false);
       state_vec.push_back( LetStat::make(range_num_var, BitCast::make(range_num_const,__int_ptr)));  
       /////////////////////////

       for( const auto & it : same_feature_range_map ) {
            const std::vector<std::pair<int,int>> & index_vec = it.second;
            const int index_vec_num = index_vec.size();
            //LOG(INFO) << index_vec_num;
            OptimizationInnerReducePass opt_reduce_pass = OptimizationInnerReducePass(
                gather_map,
                scatter_map,
                reduction_map,
                name_var_map,
                name_varP_varVP_map,
                name_varP_varPV_map,
                gather_name_new_var_map,
                reduction_name_new_var_map,
                scatter_name_new_var_map,
                name_new_var_map,
                index_vec[0].first,
                index_vec_num,
                range_num_var,
                output_name,
                vector
                );
            StateMent * opt_state = opt_reduce_pass.pass(code_seed);
            StateMent * redirect_opt_state = redirect_var_state(opt_state);

            state_vec.push_back(redirect_opt_state);
       }
       
       }
       StateMent * final_stat = CombinStatVec(state_vec);
       return final_stat;
}

