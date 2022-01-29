/*!
 *  Copyright (c) 2017 by Contributors
 * \file llvm_module.cc
 * \brief LLVM runtime module for TVM
 */
#ifndef LLVM_LIB__LLVM_MODULE_HPP
#define LLVM_LIB__LLVM_MODULE_HPP
#include <mutex>
#include "llvm_common.h"
//#include "codegen_llvm.h"
#include "llvm_log.h"
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>


class LLVMModule  {
 public:
  ~LLVMModule() {
    module_.reset();
    if (ee_ != nullptr) {
      ee_->runStaticConstructorsDestructors(true);
      delete ee_;
    }
  }
  LLVMModule(std::unique_ptr<llvm::Module> mod_ptr, std::unique_ptr<llvm::LLVMContext> ctx_ptr) {
        
        module_ = std::move(mod_ptr);
        ctx_ = std::move(ctx_ptr);
  }
 // std::unique_ptr<llvm::Module> module_;
 // std::shared_ptr<llvm::LLVMContext> ctx_;

  const char* type_key() const {
    return "llvm";
  }
  llvm::Function * CreateFunction() {

        llvm::Type * t_int_ =  llvm::Type::getInt32Ty(*ctx_);
        llvm::Function *f = llvm::Function::Create( \
        llvm::FunctionType::get( t_int_ , std::vector<llvm::Type*>(), false), \
        llvm::Function::ExternalLinkage, \
        "test", \
        module_.get() \
        );
        return f; 
  }
  uint64_t GetFunction(
      const std::string& name)  {
    
    if (ee_ == nullptr) LazyInitJIT();
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string& fname = name ;

//    uint64_t faddr = reinterpret_cast<BackendPackedCFunc>(GetFunctionAddr(fname));

    uint64_t faddr = GetFunctionAddr(fname);
    if (faddr == 0) { LOG(FATAL) << "Can not get function " << fname << "\n";
                            exit(1);
    };
    return faddr;
  }
/*  std::unique_ptr<llvm::Module> get_module() {
    return module_;
  }*/
  llvm::Module * get_module() {
    return module_.get();
  }
  std::string GetSource( const std::string & format ) {

        llvm::SmallString<256> str;
        llvm::raw_svector_ostream rso(str);

        llvm::legacy::PassManager pass;
        if( format == "asm" ) {
            CHECK(tm_->addPassesToEmitFile(
                pass, rso, nullptr, llvm::TargetMachine::CGFT_AssemblyFile) == 0)
              << "Cannot emit target CGFT_AssemblyFile";
            pass.run(*llvm::CloneModule(*mptr_));
            return rso.str().str();

        }
        return "";
  }


  llvm::TargetMachine * get_tm() {
    return tm_.get();
  }
  void Init(  std::string target) {
    InitializeLLVM();
    tm_ = GetLLVMTargetMachine(target);
    bool system_lib = (target.find("-system-lib") != std::string::npos);
    

    std::string verify_errors_storage;
    llvm::raw_string_ostream verify_errors(verify_errors_storage);
    
    CHECK(!llvm::verifyModule(*module_, &verify_errors))
        << "LLVM module verification failed with the following errors: \n"
        << verify_errors.str();
    
    module_->addModuleFlag(
        llvm::Module::Warning, "target",
        llvm::MDString::get(*ctx_, target));
    target_ = target;
    mptr_ = module_.get();
  }

 private:
  unsigned LookupLLVMIntrinsic(const std::string& name) {
    return llvm::Function::lookupIntrinsicID(name);
  }

  void LazyInitJIT() {
    CHECK(ee_ == nullptr) << "ee is not nullptr\n";
    std::lock_guard<std::mutex> lock(mutex_);
    llvm::EngineBuilder builder(std::move(module_));
    std::string triple, mcpu, mattr;
    llvm::TargetOptions opt;
    ParseLLVMTargetOptions(target_, &triple, &mcpu, &mattr, &opt);
    builder.setEngineKind(llvm::EngineKind::JIT);
    builder.setOptLevel(llvm::CodeGenOpt::Aggressive);
    if (mcpu.length() != 0) {
      builder.setMCPU(mcpu);
    }
    if (mattr.length() != 0) {
      std::vector<std::string> mattrs{mattr};
      builder.setMAttrs(mattrs);
    }
    builder.setTargetOptions(opt);
    auto tm = std::unique_ptr<llvm::TargetMachine>(builder.selectTarget());
    std::unique_ptr<llvm::TargetMachine> tm_sys = GetLLVMTargetMachine("llvm");
    if (tm_sys->getTargetTriple().getArch() != tm->getTargetTriple().getArch()) {
      LOG(FATAL) << "Cannot run module, architecture mismatch "
                 << " module=" << tm->getTargetTriple().str()
                 << " system=" << tm_sys->getTargetTriple().str();
    }
    llvm::DataLayout layout(tm->createDataLayout());
    CHECK(layout != mptr_->getDataLayout())
        << "Data layout mismatch between module("
        << mptr_->getDataLayout().getStringRepresentation() << ")"
        << " and ExecutionEngine ("
        << layout.getStringRepresentation() << ")\n";
    ee_ = builder.create(tm.release());
    CHECK(ee_ != nullptr)
        << "Failed to initialize git engine for " << mptr_->getTargetTriple() << "\n";
    ee_->runStaticConstructorsDestructors(false);
    // setup context address.
  }
  // Get global address from execution engine.
  uint64_t GetGlobalAddr(const std::string& name) {
    // first verifies if GV exists.
    if (mptr_->getGlobalVariable(name) != nullptr) {
      return ee_->getGlobalValueAddress(name);
    } else {
      return 0;
    }
  }
  uint64_t GetFunctionAddr(const std::string& name) {
    // first verifies if GV exists.
    if (mptr_->getFunction(name) != nullptr) {
      return ee_->getFunctionAddress(name);
    } else {
      return 0;
    }
  }
  // The target configuration string
  std::string target_;
  // Name of entry function.
  std::string entry_func_;
  // JIT lock
  std::mutex mutex_;
  // execution engine
  llvm::ExecutionEngine *ee_{nullptr};
  // The raw pointer to the module.
  llvm::Module* mptr_{nullptr};
  // The target machine
  std::unique_ptr<llvm::TargetMachine> tm_{nullptr};
  // The module, can be moved to ee if JIT is enabled.
  std::unique_ptr<llvm::Module> module_;
  // the context.
  std::unique_ptr<llvm::LLVMContext> ctx_;
};

#endif
