#ifndef LLVM_LOG
#define LLVM_LOG
#include "llvm_lib/llvm_common.h"
#include "log.h"
    class LLVMLogMessage :public LogMessage{
        public:
            LLVMLogMessage(const char*file,int line):LogMessage(file,line) {
            }
    llvm::raw_ostream &stream() { return llvm::errs();}
 
    };
    class LLVMLogMessageFatal:public LogMessageFatal {
       LLVMLogMessageFatal(const char*file,int line) : LogMessageFatal(file,line){
      }

    llvm::raw_ostream &stream() { return llvm::errs();}
    };
#define LLVMLOG( TYPE ) LLVMLOG_##TYPE
#define LLVMLOG_FATAL LLVMLogMessageFatal(__FILE__,__LINE__).stream()
#define LLVMLOG_INFO LLVMLogMessage(__FILE__,__LINE__).stream()

#endif
