/*!
 *  Copyright (c) 2017 by Contributors
 * \file llvm_common.h
 * \brief Common utilities for llvm initialization.
 */
#ifndef TVM_CODEGEN_LLVM_LLVM_COMMON_H_
#define TVM_CODEGEN_LLVM_LLVM_COMMON_H_

#include <llvm/ExecutionEngine/MCJIT.h>

#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/SourceMgr.h>

#include <llvm/IR/Value.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Verifier.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>

#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/CodeGen/TargetLoweringObjectFileImpl.h>

#include <llvm/Linker/Linker.h>
#include "log.h"
#include <utility>
#include <string>
extern "C" {
    //typedef int (*BackendPackedCFunc)(double*,double*); 

    #define DATATYPE double
//    typedef void(*BackendPackedCFunc)(DATATYPE *,DATATYPE*,DATATYPE*,int*,int*);
}
/*!
 * \brief Initialize LLVM on this process,
 *  can be called multiple times.
 */
void InitializeLLVM();

/*!
 * \brief Parse target options
 * \param target_str Target string, in format "llvm -target=xxx -mcpu=xxx"
 * \param triple Target triple
 * \param mcpu cpu info
 * \param options the options
 * \param mattr The attributes
 */
void ParseLLVMTargetOptions(const std::string& target_str,
                            std::string* triple,
                            std::string* mcpu,
                            std::string* mattr,
                            llvm::TargetOptions* options);

/*!
 * \brief Get target machine from target_str string.
 * \param target_str Target string, in format "llvm -target=xxx -mcpu=xxx"
 * \param allow_null Whether allow null to be returned.
 * \return target machine
 */
std::unique_ptr<llvm::TargetMachine>
GetLLVMTargetMachine(const std::string& target_str, bool allow_null = false);

#endif  // TVM_CODEGEN_LLVM_LLVM_COMMON_H_
