#include "node2state.hpp"
#include "type.hpp"
#include "statement.hpp"

#include "statement_print.hpp"
#include <stack>
#include <queue>
class Node2StateMent{
    private:
       std::map<const Node * , Varience *> _node2var_map;
       std::queue<Node*> _cal_queue;
    public:
    const std::string output_name_;
    const std::vector<std::string> &input_name_vec_;
    const std::set<std::string> &iterates_set_;
    const std::map<std::string, Type > & name_type_map_;
    const std::map<std::string,int*>  &gather_name_new_ptr_map_;
    const std::map<std::string,int*>  &reduction_name_new_ptr_map_;
    const std::map<std::string,int*>  &scatter_name_new_ptr_map_;
    const std::map<std::string,void*>  &name_new_ptr_map_;


    std::map<std::string,Varience*> &name_var_map_;
    std::map< std::string , Varience *> &name_varP_varVP_map_;
    std::map< std::string , Varience *> &name_varP_varPV_map_;
    std::map<std::string, Varience*>  &gather_name_new_var_map_;
    std::map<std::string, Varience*>  &reduction_name_new_var_map_;
    std::map<std::string, Varience*>  &scatter_name_new_var_map_;
    std::map<std::string, Varience*>  &name_new_var_map_;
    std::vector<StateMent *> &func_init_state_vec_;
    FuncStatement * &func_state_ptr_;
    const int vector_;
    Node2StateMent(
        const std::string & output_name,
        const std::vector<std::string> &input_var_vec,
        const std::set<std::string> &iterates_set,
        const std::map<std::string, Type > & name_type_map,
        const std::map<std::string,int*>  &gather_name_new_ptr_map,
        const std::map<std::string,int*>  &reduction_name_new_ptr_map,
        const std::map<std::string,int*>  &scatter_name_new_ptr_map,
        const std::map<std::string,void*>  &name_new_ptr_map,

        ///output
        std::map<std::string,Varience*> &name_var_map,
        std::map< std::string , Varience *> &name_varP_varVP_map,
        std::map< std::string , Varience *> &name_varP_varPV_map,
        std::map<std::string, Varience*>  &gather_name_new_var_map,
        std::map<std::string, Varience*>  &reduction_name_new_var_map,
        std::map<std::string, Varience*>  &scatter_name_new_var_map,
        std::map<std::string, Varience*>  &name_new_var_map,
        std::vector<StateMent *> &func_init_state_vec,
        FuncStatement * & func_state_ptr,
        const int vector
        ) :
        output_name_(output_name),
        input_name_vec_(input_var_vec),
        iterates_set_(iterates_set),
        name_type_map_(name_type_map),
        gather_name_new_ptr_map_(gather_name_new_ptr_map),
        reduction_name_new_ptr_map_(reduction_name_new_ptr_map),
        scatter_name_new_ptr_map_(scatter_name_new_ptr_map),
        name_new_ptr_map_(name_new_ptr_map),
        name_var_map_(name_var_map),
        name_varP_varVP_map_(name_varP_varVP_map),
        name_varP_varPV_map_(name_varP_varPV_map),
        gather_name_new_var_map_(gather_name_new_var_map),
        reduction_name_new_var_map_(reduction_name_new_var_map),
        scatter_name_new_var_map_(scatter_name_new_var_map),
        name_new_var_map_(name_new_var_map),
        func_init_state_vec_(func_init_state_vec),
        func_state_ptr_(func_state_ptr),
        vector_(vector)
    {
    }

    
    void generate_func() {
        std::vector<Type> arg_types_vec;
        std::vector<std::string> name_vec_tmp;

        name_vec_tmp.push_back(output_name_);
        for( const auto & it : input_name_vec_  ) {
            name_vec_tmp.push_back(it);
        }
        for( const auto & var_name : name_vec_tmp ) {
            auto name_type_map_it = name_type_map_.find( var_name );
            CHECK(name_type_map_it != name_type_map_.end()) << "can not find the type of "<< var_name;
            Type type = name_type_map_it->second;
            
            arg_types_vec.push_back( type ); 
        }
        
        func_state_ptr_ = new FuncStatement(__int, arg_types_vec,name_vec_tmp);

        const int args_num = arg_types_vec.size();

        for( int i = 0 ; i < args_num; i++ ) {
            Varience * arg_var_tmp = (*func_state_ptr_->get_args())[i];
            std::string node_name = name_vec_tmp[i];
            name_var_map_[ name_vec_tmp[i] ] = arg_var_tmp;
            
            Varience * name_varP_varVP_var= new Varience( type_scalar_ptr2vector_ptr(   arg_var_tmp->get_type()) , node_name);

            //LOG(INFO) << arg_var_tmp->get_type();
//            LOG(INFO) << type_scalar_ptr2ptr_vector(   arg_var_tmp->get_type());
            Varience * name_varP_varPV_var = new Varience( type_scalar_ptr2ptr_vector(   arg_var_tmp->get_type()), node_name );

            func_init_state_vec_.push_back( LetStat::make( name_varP_varVP_var ,BitCast::make( arg_var_tmp, name_varP_varVP_var->get_type() ) ));

            func_init_state_vec_.push_back( LetStat::make( name_varP_varPV_var ,BroadCast::make( arg_var_tmp ,vector_ )));

            name_varP_varVP_map_[ name_vec_tmp[i] ] = name_varP_varVP_var;

            name_varP_varPV_map_[ name_vec_tmp[i] ] = name_varP_varPV_var; 
        }
        
        for( const auto& it : gather_name_new_ptr_map_ ) {
            Type type_tmp = __int_ptr;
            Varience * var_tmp = new Varience( type_tmp,false );
            Const * ptr_const = new Const( (uint64_t) it.second );
            func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, type_tmp ) ));
            gather_name_new_var_map_[ it.first ] = var_tmp;
        } 
        for( const auto& it : reduction_name_new_ptr_map_ ) {

            Type type_tmp = __int_ptr;
            Varience * var_tmp = new Varience( type_tmp,false );
            Const * ptr_const = new Const( (uint64_t) it.second );
            func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, type_tmp ) ));
            reduction_name_new_var_map_[ it.first ] = var_tmp;
        } 
        for( const auto& it : scatter_name_new_ptr_map_ ) {

            Varience * var_tmp;
            if( reduction_name_new_ptr_map_.find( it.first ) != reduction_name_new_ptr_map_.end() ) {
                ////there is a gather
                auto gather_name_new_var_map_it = gather_name_new_var_map_.find( it.first );
                CHECK(gather_name_new_var_map_it != gather_name_new_var_map_.end()) << "can not find gather";
                var_tmp = gather_name_new_var_map_it->second;
            } else {
                Type type_tmp = __int_ptr;
                var_tmp = new Varience( type_tmp,false );
                Const * ptr_const = new Const( (uint64_t) it.second );
                func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, type_tmp ) ));
            }
            scatter_name_new_var_map_[ it.first ] = var_tmp;
        } 
        
        for( const auto & it :  name_new_ptr_map_) {
            const auto & var_type = name_type_map_.find( it.first );
            CHECK(var_type != name_type_map_.end()) << "can find type of " << it.first;   
            Varience * var_tmp = new Varience( var_type->second ,false);
            Const * ptr_const = new Const( (uint64_t) it.second );
            func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, var_type->second ) ));
            name_new_var_map_[ it.first ] = var_tmp;
        } 
    }

    Varience * find_var_from_node_tree( Node * node_ptr ) {
        node_ptr->dec_need_node();
        auto node_it = _node2var_map.find(node_ptr); 
        Varience * node_var;
        //LOG(INFO) << node_ptr->node_name_ << " " << node_ptr->get_need_node();
        if(node_it == _node2var_map.end()) {
            node_var = new Varience(  node_ptr->get_type(), node_ptr->node_name_ );
            _node2var_map[ node_ptr ] = node_var;

        } else {
            node_var = node_it->second; 
        }
        if( node_ptr->get_need_node() == 0 ) {
            _cal_queue.push( node_ptr  );
        }

        return node_var;
    }

    StateMent * generate_code_seed( Node * root_node_ptr ) {
       
       _cal_queue.push(root_node_ptr);
       std::vector<StateMent*> seed_state_vec;
       while( !_cal_queue.empty() ) {
            Node * top_node_ptr = _cal_queue.front();
            //LOG(INFO)  << top_node_ptr->get_node_type();
            _cal_queue.pop();
            GatherNode * gather_node = dynamic_cast<GatherNode*>( top_node_ptr );
            if( gather_node != NULL )  {
                auto gather_node_it = _node2var_map.find( gather_node ); 
                if( gather_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }

                Node * addr_ptr = gather_node->addr_ptr_;
                Node * index_ptr = gather_node->index_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * index_var = find_var_from_node_tree( index_ptr );
                seed_state_vec.push_back(LetStat::make( gather_node_it->second,Gather::make( addr_var,index_var) ));
                seed_state_vec.back()->set_index_name( gather_node->index_name_ );
                seed_state_vec.back()->set_addr_name( gather_node->addr_name_ );
            } else {
            ScatterNode * scatter_node = dynamic_cast< ScatterNode*>( top_node_ptr );

            if(scatter_node != NULL) {
                Node * addr_ptr = scatter_node->addr_ptr_;
                Node * index_ptr = scatter_node->index_ptr_;
                Node * data_ptr = scatter_node->data_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * index_var = find_var_from_node_tree( index_ptr );
                Varience * data_var = find_var_from_node_tree( data_ptr );
                seed_state_vec.push_back( Scatter::make( addr_var,index_var,data_var ) ); 

                seed_state_vec.back()->set_index_name( scatter_node->index_name_ );
                seed_state_vec.back()->set_addr_name( scatter_node->addr_name_ );
            } else {
            VarNode * var_node = dynamic_cast<VarNode*>( top_node_ptr );

            if(var_node != NULL) {
                const std::string & node_name = var_node->node_name_;
                auto it = name_var_map_.find( node_name ) ;
                if( it == name_var_map_.end() ) {
                    auto ind_it = iterates_set_.find( node_name );
                    if(ind_it == iterates_set_.end()) {
                        LOG(FATAL) << node_name << " Undefined";
                    }
                } else {
                    auto var_node_it = _node2var_map.find( var_node ); 
                    if( var_node_it == _node2var_map.end() ) {
                        LOG(FATAL) << "Can not find Varience";
                    }
                    it->second->set_node_name( node_name );
                    seed_state_vec.push_back( LetStat::make( var_node_it->second , it->second ) ); 
                }
            } else { 
             LoadNode * load_node = dynamic_cast< LoadNode*>( top_node_ptr);
            if(load_node != NULL) {
                auto load_node_it = _node2var_map.find( load_node ); 
                if( load_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * addr_ptr = load_node->addr_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );

                load_node_it->second->set_addr_name( addr_ptr->node_name_  );
                load_node_it->second->set_node_name( addr_ptr->node_name_  );

                seed_state_vec.push_back( LetStat::make( load_node_it->second , Load::make( addr_var ) ) ); 

            } else {
             AddNode * add_node = dynamic_cast< AddNode*>( top_node_ptr );
            if( add_node != NULL ) { 
                auto add_node_it = _node2var_map.find( add_node ); 
                if( add_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = add_node->left_node_;
                Node * right_node = add_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                StateMent * add_stat = Add::make(left_var,right_var);
                add_stat->set_index_name(add_node->index_name_);
                add_stat->set_node_name(add_node->node_name_);
                seed_state_vec.push_back( LetStat::make( add_node_it->second, add_stat ) ); 
                
                seed_state_vec.back()->set_node_name( add_node->node_name_ );

                seed_state_vec.back()->set_index_name( add_node->index_name_ );

            } else {
             DivNode * div_node = dynamic_cast< DivNode*>( top_node_ptr );
            if(div_node != NULL) {
                //LOG(INFO) <<"Div";
                 auto div_node_it = _node2var_map.find( div_node ); 
                if( div_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = div_node->left_node_;
                Node * right_node = div_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( div_node_it->second,Div::make( left_var,right_var ) ) ); 
                 
                seed_state_vec.back()->set_node_name( div_node->node_name_ );

          
            } else {
            
            MultNode * mult_node = dynamic_cast<MultNode*>( top_node_ptr );
            if(mult_node != NULL) {
                auto mult_node_it = _node2var_map.find( mult_node ); 
                if( mult_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = mult_node->left_node_;
                Node * right_node = mult_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( mult_node_it->second,Mul::make( left_var,right_var ) ) );           

                seed_state_vec.back()->set_node_name( mult_node->node_name_ );


            } else {
            StoreNode * store_node = dynamic_cast< StoreNode*>( top_node_ptr ); 
            if(store_node != NULL){
                Node * addr_ptr = store_node->addr_ptr_;
                Node * data_ptr = store_node->data_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * data_var = find_var_from_node_tree( data_ptr );
                seed_state_vec.push_back( Store::make( addr_var ,data_var ) );

                seed_state_vec.back()->set_node_name( store_node->node_name_ );

                seed_state_vec.back()->set_index_name( store_node->index_name_ );
                seed_state_vec.back()->set_addr_name( store_node->addr_name_ );

            } else {
            LOG(FATAL) << "Unsupported";
            }}}}}}}}
       }
        std::vector<StateMent*>  seed_state_vec_reverse;
        seed_state_vec_reverse.resize( seed_state_vec.size(), nullptr );
        std::vector<StateMent*>::reverse_iterator rit = seed_state_vec.rbegin();
        for ( int i = 0 ; rit!= seed_state_vec.rend(); ++rit,i++)
            (seed_state_vec_reverse)[i] = *rit;

        return CombinStatVec( seed_state_vec_reverse ); 
    }
};

void node_tree2state(
        const std::string & output_name,
        const std::vector<std::string> &input_var_vec,
        const std::set<std::string> &iterates_set,
        const std::map<std::string, Type > & name_type_map,
        const std::map<std::string,int*>  &gather_name_new_ptr_map,
        const std::map<std::string,int*>  &reduction_name_new_ptr_map,
        const std::map<std::string,int*>  &scatter_name_new_ptr_map,
        const std::map<std::string,void*>  &name_new_ptr_map,

        ///output
        std::map<std::string,Varience*> &name_var_map,
        std::map< std::string , Varience *> &name_varP_varVP_map,
        std::map< std::string , Varience *> &name_varP_varPV_map,
        std::map<std::string, Varience*>  &gather_name_new_var_map,
        std::map<std::string, Varience*>  &reduction_name_new_var_map,
        std::map<std::string, Varience*>  &scatter_name_new_var_map,
        std::map<std::string, Varience*>  &name_new_var_map,
        std::vector<StateMent *> &func_init_state_vec,
        StateMent * & calculate_state,
        FuncStatement * &func_state_ptr,
         Node * node_ptr,
         const int vector
        ) {
        Node2StateMent node2state = Node2StateMent(
                output_name,
                input_var_vec,
                iterates_set,
                name_type_map,
                gather_name_new_ptr_map,
                reduction_name_new_ptr_map,
                scatter_name_new_ptr_map,
                name_new_ptr_map,
                name_var_map,
                name_varP_varVP_map,
                name_varP_varPV_map,
                gather_name_new_var_map,
                reduction_name_new_var_map,
                scatter_name_new_var_map,
                name_new_var_map,
                func_init_state_vec,
                func_state_ptr,
                vector
                );
        node2state.generate_func();
        //LOG(INFO) << "function generated";
        calculate_state = node2state.generate_code_seed(node_ptr);
//        LOG(INFO) << calculate_state;
//        LOG(INFO) << "code seed generated";
}
