#include "state_pass.hpp"

#define SET_DISPATCH(CLASS_NAME)  \
            ftype_ptr->set_dispatch<CLASS_NAME>([this](StateMent * __basic_class_ptr)->StateMent*{\
                CLASS_NAME * __class_name = dynamic_cast<CLASS_NAME*>( __basic_class_ptr );\
                if(__class_name==NULL){printf("type StateMent" "-> "#CLASS_NAME"failt");exit(1);}\
                return this->pass_(__class_name);})\

StateMent* StateMentPass::pass_(StateMent * stat) {
    LOG(FATAL) << "\nthe statement %s does not support\n" << stat->get_class_name() ;
    return nullptr;
}
StateMent* StateMentPass::pass_(Block * stat) {
    std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    std::vector<StateMent* > * new_state_vec = new std::vector<StateMent*>() ;

    new_state_vec->resize( state_vec->size() , nullptr);
    bool is_change = false;
    for(unsigned int i = 0 ; i < state_vec->size() ; i++) {
        StateMent * new_state = pass((*state_vec)[i]);

        if( new_state != (*state_vec)[i] ) {
            is_change = true;
        }

        (*new_state_vec)[i] = new_state; 
    }
    if(is_change) {

        return Block::make( new_state_vec ); 
    } else {
        return stat;
    }
}
StateMent* StateMentPass::pass_(For * stat) {
//    StateMent * var_state = stat->get_var();
    StateMent * begin_state = stat->get_begin();
    StateMent * end_state = stat->get_end();
    StateMent * space_state = stat->get_space();
    StateMent * state_state = stat->get_stat();


//    StateMent * var_state_new = pass(var_state);
    StateMent * begin_state_new = pass(begin_state);
    StateMent * end_state_new = pass(end_state);
    StateMent * space_state_new = pass(space_state);

    StateMent * state_state_new = pass(state_state);
//    if( var_state == var_state_new &&
  if(      begin_state == begin_state_new &&
        end_state == end_state_new &&
        space_state == space_state_new &&
        state_state == state_state_new) {
        return stat;
    } else {
        return For::make(begin_state_new,space_state_new,end_state_new,state_state_new); 
    }
    
}
StateMent* StateMentPass::pass_(Varience * stat) {
    //os << stat->get_type_str() << " " << stat->get_name(); 
    return stat;
}
StateMent* StateMentPass::pass_(Const * stat) {
//    os << stat->get_type_str() << "(" << stat->get_data_str() <<")";
    return stat;
}
StateMent* StateMentPass::pass_(LetStat * stat) {
    
    Varience * res_var = stat->get_res();
    StateMent * expr_state = stat->get_expr();

    
    StateMent * expr_state_new = pass( expr_state);

    if(  expr_state == expr_state_new ){
        return stat;
    } else { 
        return LetStat::make( res_var,expr_state_new );
    }
}
StateMent* StateMentPass::pass_( IncAddr * stat ) {
    StateMent * addr_state = stat->get_addr();
    StateMent * inc_state = stat->get_inc();
    
    StateMent * addr_state_new = pass( addr_state );
    StateMent * inc_state_new = pass(inc_state);
    
    if( addr_state_new == addr_state && inc_state_new == inc_state ) {
        return stat;
    } else {
        return IncAddr::make( addr_state_new , inc_state_new );
    }

}
StateMent * StateMentPass::pass_(Nop * stat ) {
    return stat;
}
StateMent * StateMentPass::pass_(Scatter * stat) {
    StateMent * addr_state = stat->get_addr();
    
    StateMent * index_state = stat->get_index();

    StateMent * mask_state = stat->get_mask();
    
    StateMent * data_state = stat->get_data();
    ///new
    StateMent * addr_state_new = pass( addr_state );
    
    StateMent * index_state_new = pass(index_state);

    StateMent * mask_state_new = pass(mask_state);
    
    StateMent * data_state_new = pass( data_state);

    if( addr_state == addr_state_new &&
        index_state == index_state_new &&
        mask_state == mask_state_new &&
        data_state == data_state_new) {
        return stat;
    } else {
        return Scatter::make( addr_state_new, index_state_new,data_state_new, mask_state_new );
    }
}
StateMent * StateMentPass::pass_(Gather * stat) {

    StateMent * addr_state = stat->get_addr();
    
    StateMent * index_state = stat->get_index();

    StateMent * mask_state = stat->get_mask();
    
    ///new
    StateMent * addr_state_new = pass( addr_state );
    
    StateMent * index_state_new = pass(index_state);

    StateMent * mask_state_new = pass(mask_state);
    

    if( addr_state == addr_state_new &&
        index_state == index_state_new &&
        mask_state == mask_state_new ) {
        return stat;
    } else {
        return Gather::make( addr_state_new, index_state_new, mask_state_new );
    }

}
StateMent * StateMentPass::pass_(Load * stat) {

    StateMent * addr_state = stat->get_addr();
    bool is_alined = stat->get_is_aligned();

    StateMent * mask_state = stat->get_mask();
    //new
    StateMent * addr_state_new = pass(addr_state);

    StateMent * mask_state_new;
    
    if(mask_state != NULL)
        mask_state_new = pass(mask_state); 
    else
        mask_state_new = NULL;

    if( addr_state == addr_state_new &&
        mask_state == mask_state_new) {
        return stat;
    } else {
        if( mask_state_new == NULL ) {
            return Load::make( addr_state_new,  is_alined );
        } else { 
            return Load::make( addr_state_new, mask_state_new, is_alined );
        }
    }
}
StateMent * StateMentPass::pass_( Print * stat ) {
    StateMent * print_var = stat->get_var();
    StateMent * print_var_new = pass(print_var);
    if( print_var_new == print_var ) {
        return stat;
    } else {
        return Print::make(print_var_new);
    }
}
StateMent * StateMentPass::pass_( ExtractElement * stat ) {
    StateMent * from_state = stat->get_from();
    StateMent * index_state = stat->get_index();

    /////////new
    StateMent * from_state_new = pass( from_state );
    StateMent * index_state_new = pass( index_state );
    
    if( from_state == from_state_new &&
        index_state == index_state_new ) {
        return stat;
    } else {
        return ExtractElement::make(from_state_new,index_state_new);
    }
}
StateMent * StateMentPass::pass_( InsertElement * stat) {

    StateMent * to_state = stat->get_to();
    StateMent * from_state = stat->get_from();
    StateMent * index_state = stat->get_index();

    /////////new

    StateMent * to_state_new = pass( to_state );
    StateMent * from_state_new = pass( from_state );
    StateMent * index_state_new = pass( index_state );
    
    if( to_state == to_state_new &&
        from_state == from_state_new &&
        index_state == index_state_new ) {
        return stat;
    } else {
        return InsertElement::make(to_state_new ,from_state_new,index_state_new);
    }

}

StateMent* StateMentPass::pass_(Store * stat ) {

    StateMent * addr_state = stat->get_addr();
    bool is_alined = stat->get_is_aligned();
    StateMent * mask_state = stat->get_mask();

    StateMent * data_state = stat->get_data();
    //new

    StateMent * addr_state_new = pass(addr_state);
    StateMent * mask_state_new = pass(mask_state);
    
    StateMent * data_state_new = pass(data_state);
    if( data_state == data_state_new &&
        addr_state == addr_state_new &&
        mask_state == mask_state_new) {
        return stat;
    } else {
        if(mask_state == NULL) {
            return Store::make( addr_state_new, data_state_new , is_alined );
        } else {
        
            return Store::make( addr_state_new, data_state_new , mask_state_new, is_alined );
        }
    }

}
StateMent * StateMentPass::pass_(Select * stat ) {
    StateMent * v1_state = stat->get_v1();
    StateMent * v2_state = stat->get_v2();
    StateMent * index_state = stat->get_index();

    ////new
    StateMent * v1_state_new = pass(v1_state);
    StateMent * v2_state_new = pass(v2_state);

    StateMent * index_state_new = pass(index_state);

    if( v1_state == v1_state_new &&
        v2_state == v2_state_new &&
        index_state == index_state_new ) {
        return stat;
    } else {
            return Select::make( v1_state_new,v2_state_new,index_state_new );
    }
}

StateMent * StateMentPass::pass_(Shuffle * stat ) {
    StateMent * v1_state = stat->get_v1();
    StateMent * v2_state = stat->get_v2();
    StateMent * index_state = stat->get_index();

    ////new
    StateMent * v1_state_new = pass(v1_state);
    StateMent * v2_state_new = NULL;
    if( v2_state != NULL) {
        v2_state_new = pass(v2_state);
    }

    StateMent * index_state_new = pass(index_state);

    if( v1_state == v1_state_new &&
        v2_state == v2_state_new &&
        index_state == index_state_new ) {
        return stat;
    } else {
        if(v2_state == NULL) {
            return Shuffle::make(v1_state_new,index_state_new);
        } else { 
            return Shuffle::make( v1_state_new,v2_state_new,index_state_new );
        }
    }
}
StateMent * StateMentPass::pass_(ComplexReduce * stat ) {
    StateMent * v1_state = stat->get_v1();
    int mask = stat->get_mask();
    StateMent * index_state = stat->get_index();

    StateMent * shuffle_index_state = stat->get_shuffle_index_ptr();

    //new
    StateMent * v1_state_new = pass(v1_state);
    StateMent * index_state_new = pass(index_state);
    StateMent * shuffle_index_state_new = pass( shuffle_index_state );
    if( v1_state == v1_state_new &&
        shuffle_index_state == shuffle_index_state_new &&
        index_state == index_state_new ) {
        return stat;
    } else {
        return ComplexReduce::make( v1_state_new,index_state_new,shuffle_index_state_new ,mask);
    }

}
StateMent * StateMentPass::pass_(DetectConflict * stat ) {
    
    StateMent * index_state = stat->get_index();
    StateMent * index_state_new = pass(index_state);
    if( index_state == index_state_new  ) {
        return stat;
    } else {
        return DetectConflict::make(index_state_new);
    }
}


StateMent * StateMentPass::pass_(Reduce * stat ) {
    StateMent * v1_state = stat->get_v1();
    StateMent * v1_state_new = pass(v1_state);
    if( v1_state == v1_state_new  ) {
        return stat;
    } else {
        return Reduce::make(v1_state_new);
    }

}
StateMent * StateMentPass::pass_(BroadCast * stat) {
    StateMent * v1_state = stat->get_v1();
    StateMent * v1_state_new = pass(v1_state);
    if( v1_state == v1_state_new  ) {
        return stat;
    } else {
        return BroadCast::make(v1_state_new,stat->get_type().get_lanes());
    }

}

StateMent * StateMentPass::pass_(BitCast * stat ) {
    StateMent * v1_state = stat->get_v1();
    StateMent * v1_state_new = pass(v1_state);
    if( v1_state == v1_state_new  ) {
        return stat;
    } else {
        return BitCast::make(v1_state_new,stat->get_type());
    }
}
    StateMent * StateMentPass::pass_(ICmpEQ * stat ) {
        StateMent * v1_state = stat->get_v1();
        StateMent * v2_state = stat->get_v2();
        StateMent * v1_state_new = pass(v1_state);
        StateMent * v2_state_new = pass(v2_state);
        if( v1_state == v1_state_new && 
            v2_state == v2_state_new) { 
            return stat;
        } else {
            StateMent * ret =  ICmpEQ::make(v1_state_new,v2_state_new);
            return ret;
        }
    }

#define PASS_BINARY( CLASSNAME ,OP) \
    StateMent * StateMentPass::pass_(CLASSNAME * stat ) {\
        StateMent * v1_state = stat->get_v1();\
        StateMent * v2_state = stat->get_v2();\
        StateMent * v1_state_new = pass(v1_state);\
        StateMent * v2_state_new = pass(v2_state);\
        if( v1_state == v1_state_new && \
            v2_state == v2_state_new) { \
            return stat;\
        } else {\
            StateMent * ret =  CLASSNAME::make(v1_state_new,v2_state_new);\
            ret->set_addr_name( stat->addr_name_ );\
            ret->set_index_name( stat->index_name_ );\
            ret->set_node_name( stat->node_name_ );\
            return ret;\
        }\
    }

PASS_BINARY( Binary,"op" )
PASS_BINARY( Add, "+");

PASS_BINARY( Div, "/");
PASS_BINARY( Mul, "*");

PASS_BINARY( Minus, "-");

StateMent* StateMentPass::pass(StateMent * stat) {
    
    if(stat== NULL) return NULL;
    if(ftype_ptr == nullptr) {
        ftype_ptr = new FType();
        SET_DISPATCH( StateMent );
        SET_DISPATCH( Block );
        SET_DISPATCH( For );
        SET_DISPATCH(Varience);

        SET_DISPATCH( Const );

        SET_DISPATCH( LetStat );
        SET_DISPATCH(IncAddr);
        SET_DISPATCH(Nop);
        SET_DISPATCH(Scatter);
        SET_DISPATCH(Gather);
        SET_DISPATCH( Load );
        SET_DISPATCH(Store);
        SET_DISPATCH(Shuffle);

        SET_DISPATCH(Select);

        SET_DISPATCH(ICmpEQ);
        SET_DISPATCH( Reduce );
        SET_DISPATCH(BitCast);
        SET_DISPATCH(Binary);
        SET_DISPATCH(Add);
        SET_DISPATCH(Mul);

        SET_DISPATCH(Div);
        SET_DISPATCH(Minus);
        SET_DISPATCH(Print);
        SET_DISPATCH(BroadCast);

        SET_DISPATCH(ComplexReduce);
        SET_DISPATCH(DetectConflict  );

        SET_DISPATCH(ExtractElement );
        SET_DISPATCH(InsertElement );
    }
    return (*ftype_ptr)(stat);

}
#undef SET_DISPATCH
