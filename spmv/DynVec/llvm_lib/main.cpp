#include "llvm_common.h"
#include "llvm_module.h"
using namespace llvm;
using namespace llvm::sys;
std::shared_ptr<LLVMContext> TheContext;
Module* TheModule;
//std::unique_ptr<Module> TheModule;
static std::map<std::string, AllocaInst *> NamedValues;
int main() {

    std::unique_ptr<LLVMModule> llvm_module = llvm::make_unique<LLVMModule>( );

    llvm_module->Init("llvm -mcpu=x86-64  -mattr=avx2");
    TheContext = llvm_module->get_ctx();
//    TheModule = llvm_module->get_module();

    llvm::Type * t_int_ =  llvm::Type::getInt32Ty(*TheContext);
    llvm::Constant* One_ = llvm::ConstantInt::get( t_int_ , 1);
    IRBuilder<> Builder = IRBuilder<>(*TheContext);
    Function * f = llvm_module->CreateFunction();
    //    Function *f = Function::Create( \
        FunctionType::get(llvm::Type::getVoidTy(*TheContext), std::vector<llvm::Type*>(), false), \
        Function::ExternalLinkage, \
        "test", \
        TheModule.get() \
        );
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", f);
    Builder.SetInsertPoint(BB);
    Builder.CreateRet(One_);
     
    if( verifyFunction(*f, &errs()) )
        return 1;

    BackendPackedCFunc func = llvm_module->GetFunction("test");
    printf("\n%d\n",func());

    func = llvm_module->GetFunction("test");

    printf("\n%d\n",func());
    return 0;
}
