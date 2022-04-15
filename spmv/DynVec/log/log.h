#ifndef LOG_H
#define LOG_H
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
class LogMessage {
    protected:
    std::stringstream log_stream_;
    inline std::string Demangle(char const *msg_str) {
        using std::string;
        string msg(msg_str);
        size_t symbol_start = string::npos;
        size_t symbol_end = string::npos;
        if ( ((symbol_start = msg.find("_Z")) != string::npos)
            && (symbol_end = msg.find_first_of(" +", symbol_start)) ) {
            string left_of_symbol(msg, 0, symbol_start);
            string symbol(msg, symbol_start, symbol_end - symbol_start);
            string right_of_symbol(msg, symbol_end);
            int status = 0;
            size_t length = string::npos;
            std::unique_ptr<char, decltype(&std::free)> demangled_symbol =
                {abi::__cxa_demangle(symbol.c_str(), 0, &length, &status), &std::free};
        if (demangled_symbol && status == 0 && length > 0) {
            string symbol_str(demangled_symbol.get());
            std::ostringstream os;
            os << left_of_symbol << symbol_str << right_of_symbol;
            return os.str();
        }
        }
        return string(msg_str);
    }

    void stacktrace() {
        const int size = 100;
        void * array[100];
        int stack_num = backtrace(array, size);
        char ** stacktrace = backtrace_symbols(array, stack_num);
        for (int i = 0; i < stack_num; ++i) {
            log_stream_ << Demangle(stacktrace[i]) << "\n";
        }
        free(stacktrace);
    }
    public:
    LogMessage( const char * file,int line ) {
        log_stream_<< file << ":" << line << ":";
        std::cout << log_stream_.str() ;
        fflush(stdout);
        log_stream_.str(std::string());
    }
    std::ostream &stream() { return std::cout; }
    ~LogMessage() {
        std::cout<<std::endl;
    }
};
class LogMessageFatal :public LogMessage{
    public:
    LogMessageFatal(const char*file,int line):LogMessage(file,line) {
        log_stream_<<"\n";
        stacktrace();
        std::cout << log_stream_.str() ;

        fflush(stdout);
    }
    ~LogMessageFatal() {
        std::cout<<std::endl;
        exit(1);  
    } 
};
#define LOG(TYPE) LOG_##TYPE
#define LOG_FATAL LogMessageFatal(__FILE__,__LINE__).stream()
#define LOG_INFO LogMessage(__FILE__,__LINE__).stream()
//#define LOG() llvm::errs()<<__FILE__<<" " << __LINE__ << " "
#define CHECK( COND ) \
        if(!(COND)) \
            LOG_FATAL
#endif
