#ifndef LLVM_CODEGEN_HPP
#define LLVM_CODEGEN_HPP
#include "statement.hpp"
#include <map>
#include "llvm_lib/llvm_common.h"
#include "llvm_lib/llvm_log.h"
#include "llvm_lib/llvm_print.hpp"
#include "log.h"
#include <string>
#include <memory>
#include "ir_func.hpp"
#define SET_DISPATCH(CLASS_NAME)  \
    ftype_ptr->set_dispatch<CLASS_NAME>([this](StateMent * __basic_class_ptr)->Value*{\
                CLASS_NAME * __class_name = dynamic_cast<CLASS_NAME*>( __basic_class_ptr );\
                if(__class_name==NULL){printf("type StateMent " "-> "#CLASS_NAME"failt");exit(1);}\
                return this->CodeGen_(__class_name);})\
 
class LLVMCodeGen {
    //const int lanes_ = VECTOR;
    const int vector_;
    const int alinements_ = 64;
    using Value = llvm::Value;
    using Function = llvm::Function;
    using string = std::string;
    using AllocaInst = llvm::AllocaInst; 
    using BasicBlock = llvm::BasicBlock;


    using FType = ir_func<Value*(StateMent*)>; 
    std::map<Varience * , Value *> var_val_map;

    std::map<Varience * , Value *> arg_val_map_;
    std::set<Varience *> var_mutable_set_;
    std::unique_ptr<llvm::Module> mod_ptr_;
    std::unique_ptr<llvm::IRBuilder<>>  build_ptr_;
    std::unique_ptr<llvm::LLVMContext> ctx_ptr_;

    std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;

    Function * conflict_512_;

    Function * permvar_float_256_;
    Function * permvar_double_512_;
    Function * permvar_float_512_;
    Function * permvar_int_512_;

//    Function * permvar_double_256_;
    llvm::Type*  t_int_;

    llvm::Type*  t_int4_;
    llvm::Type*  t_int8_;

    llvm::Type*  t_int16_;
    llvm::Type*  t_int64_ ;
    llvm::Type*  t_bool_ ;
    llvm::Type*  t_double_;

    llvm::Type*  t_float_;

    llvm::Type*  t_int_p_ ;

    llvm::Type*  t_int_p_p_ ;
    llvm::Type*  t_int64_p_;

    llvm::Type*  t_int64_p_p_;
    llvm::Type*  t_int8_ptr_ ;
    llvm::Type*  t_bool_p_;
    llvm::Type*  t_double_p_;

    llvm::Type*  t_float_ptr_;

    llvm::Type* t_bool_vec_;
    llvm::Type* t_int_vec_;

    llvm::Type* t_int_dvec_;
    llvm::Type* t_int64_vec_;
    llvm::Type*  t_double_vec_;

    llvm::Type* t_int8_vec_;
    llvm::Type*  t_float_vec_;

    llvm::Type*  t_float_ptr_vec_;

    llvm::Type*  t_float_vec_ptr_;

    llvm::Type* t_int8_vec_ptr_;
    llvm::Type* t_void_;
    llvm::Type* t_int_vec_p_;

    llvm::Type* t_int64_vec_p_;
    llvm::Type* t_int_dvec_p_;
    llvm::Type*  t_double_vec_p_;
    llvm::Type* t_int_ptr_vec_ ;
    llvm::Type*  t_double_ptr_vec_;

    llvm::Constant *Zero_ ;
    llvm::Constant* FZeroVec_;

    llvm::Constant *FZero_ ;

    llvm::Constant* DZeroVec_;

    llvm::Constant * SixTeenVec_;
    llvm::Constant *DZero_ ;
    llvm::Constant *One_ ;

//    llvm::Constant * CONST_INDEX_NUM_[VECTOR];
    llvm::Constant *True_;

    llvm::Constant * true_vec_value_ ;
    llvm::Constant * Null_;
    llvm::Constant * ZeroVec_;
    llvm::Constant * FFFF_;
    llvm::Constant * SixTeen_;

    std::map< Varience*,Value*> var_val_map_;
    llvm::Type * Type2LLVMType(const Type & type) ;

    llvm::Value * LLVMBroadCast( llvm::Value * value, const int lanes) ;
    public:
    std::unique_ptr<llvm::Module> get_mod() {
        if(mod_ptr_)
            return std::move(mod_ptr_);
        else 
            LOG(FATAL) << "mod ptr is null";
        return NULL;
    }
    std::unique_ptr<llvm::LLVMContext> get_ctx() {
        if( ctx_ptr_ ) 
            return std::move(ctx_ptr_);
        else 
            LOG(FATAL) << "ctx ptr is null";
        return NULL;
    }

    template<typename T> 
    void init_vec( llvm::Value * &vec, const void * data_arg, const int lanes,llvm::Type* llvm_type ) ;

    LLVMCodeGen(const int) ;

    Value* CodeGen_(StateMent * stat ) ;

    Value * CodeGen_(Block * stat  ) ;

    Value * CodeGen_(Print * stat) ;

    Value * CodeGen_( For * stat ) ;

    Value* CodeGen_( Const* stat) ;

    Value * CodeGen_( LetStat * stat ) ;

    Value * CodeGen_( IncAddr * stat ) ;

    Value * CodeGen_(Varience * stat) ;

    Value * CodeGen_(Nop * stat) ;

    Value * CodeGen_(Scatter * stat) ;


    Value * CodeGen_(Gather * stat) ;

    Value * CodeGen_(Load * stat) ;

    Value * CodeGen_(Store * stat) ;

    Value * CodeGen_(ICmpEQ * stat) ;
    Value * CodeGen_(Select * stat) ;
    Value * CodeGen_(Shuffle * stat) ;

    Value * CodeGen_(Reduce * stat) ;

    Value * CodeGen_(BroadCast* stat) ;

    Value * CodeGen_(BitCast * stat) ;

    Value * CodeGen_(Binary * stat) ;

    Value * CodeGen_(Add * stat) ;

     Value * CodeGen_(Minus * stat) ;

    Value * CodeGen_(Div * stat) ;

    Value * CodeGen_(Mul * stat) ;

//    Value * CodeGen_( ComplexReduce * stat) ;

    Value * CodeGen_( DetectConflict * stat) ;

    Value * CodeGen_(InsertElement * stat) ;
    Value * CodeGen_(ExtractElement * stat) ;
    Value* CodeGen( StateMent * stat ) ;

    void AddFunction( FuncStatement * func_state ) ;
    void PrintModule() {
        LLVMLOG(INFO) << *mod_ptr_;
    }
};
#endif
