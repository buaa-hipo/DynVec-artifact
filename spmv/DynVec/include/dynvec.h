#ifndef _DYNVEC_H_
#define _DYNVEC_H_
#include <string>
#include <stdint.h>
#include <map>
uint64_t compiler( const std::string & expr_str,  std::map<std::string,void*> & name2ptr_map  ,int table_column_num) ;
#endif