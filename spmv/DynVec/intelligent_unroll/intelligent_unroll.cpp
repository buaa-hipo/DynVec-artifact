#include "intelligent_unroll.hpp"
#include "statement_print.hpp"

#include "Timers.hpp"
class IntelligentUnroll{

    ///produced by parse expression
    std::string output_name_;
    std::set<std::string> gather_set_;
    std::set<std::string> scatter_set_;
    std::set<std::string> reduction_set_;

    std::set<std::string> load_set_;
    std::map<std::string, Type >  name_type_map_;

    std::vector<std::string> input_name_vec_;
    std::set<std::string> iterates_set_;
    
    Node * root_node_ptr_;
    int vector_;
    /////produced by generate information    
    std::map<std::string,GatherInfo*> gather_map_;
    std::map<std::string,ScatterInfo*> scatter_map_;
    std::map<std::string,ReductionInfo*> reduction_map_;
    std::unordered_map<size_t,std::vector<int>> same_feature_map_;
    std::unordered_map<size_t,std::vector<std::pair<int,int>>>same_feature_range_map_;

    /////produced by transform_data
    std::map<std::string,int*>  gather_name_new_ptr_map_;
    std::map<std::string,int*>  reduction_name_new_ptr_map_;
    std::map<std::string,int*>  scatter_name_new_ptr_map_;
    std::map<std::string,void*>  name_new_ptr_map_;
    ///produced by code seed generate
    std::map<std::string,Varience*> name_var_map_;
    std::map< std::string , Varience *> name_varP_varVP_map_;
    std::map< std::string , Varience *> name_varP_varPV_map_;
    std::map<std::string, Varience*>  gather_name_new_var_map_;
    std::map<std::string, Varience*>  reduction_name_new_var_map_;
    std::map<std::string, Varience*>  scatter_name_new_var_map_;
    std::map<std::string, Varience*>  name_new_var_map_;
    std::vector<StateMent *> func_init_state_vec_;
    StateMent * calculate_state_; 

    FuncStatement * func_state_ptr_=NULL;
    ////produced by passes
    //
    
    //llvm module
    LLVMModule * llvm_module_ptr_; 
    public:
    IntelligentUnroll(  ) {
    }
    //////////////////
    uint64_t compiler( const std::string & expr_str,  std::map<std::string,void*> & name2ptr_map  ,int table_column_num) {

        ////////////parse the expression, get a node tree
        vector_ = parse_expression( 
                          expr_str,
                          root_node_ptr_,
                          gather_set_,
                          scatter_set_,
                          reduction_set_,
                          load_set_,
                          name_type_map_,
                          input_name_vec_,
                          iterates_set_,
                          output_name_
                          );
        Type::init_type( vector_ );
//        LOG(INFO) << "parse expression finished";
        ///////////generate information
        generate_information( 
                scatter_set_,
                reduction_set_, 
                gather_set_,
                name2ptr_map,
                table_column_num,
                gather_map_,
                scatter_map_,
                reduction_map_,
                same_feature_map_,
                same_feature_range_map_ ,
                vector_);

//        LOG(INFO) << "generate information finished";
        ///////////transform data
        transform_data( name_type_map_,
                        name2ptr_map,
                        gather_set_,
                        scatter_set_,
                        reduction_set_,
                        load_set_,
                        gather_map_,
                        scatter_map_,
                        reduction_map_,
                        same_feature_map_,
                        same_feature_range_map_,
                        gather_name_new_ptr_map_,
                        reduction_name_new_ptr_map_,
                        scatter_name_new_ptr_map_,
                        name_new_ptr_map_,
                        table_column_num,
                        vector_
        ); 

        //LOG(INFO) << "transform data finished";
        ////////////////node tree to code seed
        node_tree2state(
                output_name_,
                input_name_vec_,
                iterates_set_,
                name_type_map_,
                gather_name_new_ptr_map_,
                reduction_name_new_ptr_map_,
                scatter_name_new_ptr_map_,
                name_new_ptr_map_,
                name_var_map_,
                name_varP_varVP_map_,
                name_varP_varPV_map_,
                gather_name_new_var_map_,
                reduction_name_new_var_map_,
                scatter_name_new_var_map_,
                name_new_var_map_,
                func_init_state_vec_,
                calculate_state_,
                func_state_ptr_,
                root_node_ptr_,
                vector_
                );
        //LOG(INFO) << root_node_ptr_;
//        LOG(INFO) << "node tree 2 statement finished";
        ///////////// formulation and optimization
        calculate_state_ = formulation_state(
                name_varP_varVP_map_,
                name_varP_varPV_map_,
                calculate_state_ );

//        LOG(INFO) << "formulation statement finished";
        calculate_state_ = optimization_state(
                gather_map_,
                scatter_map_,
                reduction_map_,
                name_var_map_,
                name_varP_varVP_map_,
                name_varP_varPV_map_,
                gather_name_new_var_map_,
                reduction_name_new_var_map_,
                scatter_name_new_var_map_,
                name_new_var_map_,
                same_feature_map_,
                same_feature_range_map_,
                output_name_,
                calculate_state_,
                vector_
        );

//        LOG(INFO) << "optimize statement finished";
        /////////////combin statement with func_statement
        StateMent * init_func = CombinStatVec(func_init_state_vec_);
        StateMent * func_state = Block::make( init_func,calculate_state_ );
//        LOG(INFO) << func_state;
        func_state_ptr_->set_state( func_state );
         
//        LOG(INFO) << "merge state finished";
        /////////// 
        LLVMCodeGen codegen = LLVMCodeGen( vector_ );
        codegen.AddFunction( func_state_ptr_ );

//        codegen.PrintModule();
//        LOG(INFO) << "add function finished";
        llvm_module_ptr_ = new LLVMModule( codegen.get_mod(),codegen.get_ctx() );
        #ifdef __AVX512CD__
            std::string arch_str = "llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx";
        #else
            #ifdef __AVX2__
            std::string arch_str = "llvm -mcpu=x86-64  -mattr=+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx,+fast-gather";
            #else
            LOG(FATAL) << "Unsupported architetures";

            #endif
        #endif

        llvm_module_ptr_->Init( arch_str );
      
        Timer::startTimer("llvmcompile");
        uint64_t func = llvm_module_ptr_->GetFunction("function");

        Timer::endTimer("llvmcompile");

//        LOG(INFO) << "generated module finished";
        return func;
    }
}; 

uint64_t compiler( const std::string & expr_str,  std::map<std::string,void*> & name2ptr_map  ,int table_column_num) {
    IntelligentUnroll intel_unroll;
    return intel_unroll.compiler( expr_str,name2ptr_map,table_column_num );
}
