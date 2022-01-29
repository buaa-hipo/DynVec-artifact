#ifndef TRANSFORM_DATA_HPP
#define TRANSFORM_DATA_HPP
#include <stdlib.h>
#include "configure.hpp"
#include <string>
#include <set>
#include <vector>
#include <map>
#include <ulimit.h>
#include<climits>
#include <unordered_map>
#include "type.hpp"
#include "tools_set.hpp"
#include "tools_set.hpp"
void transform_data(
    const std::map<std::string, Type > & name_type_map,
    const std::map<std::string,void*> & name_ptr_map,
    const std::set<std::string > & gather_set,
    const std::set<std::string> & scatter_set,
    const std::set<std::string> & reduction_set,
    const std::set<std::string> & load_set,
    const std::map<std::string,GatherInfo*> &gather_map,
    const std::map<std::string,ScatterInfo*> &scatter_map,
    const std::map<std::string,ReductionInfo*> &reduction_map,
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
    std::map<std::string,int*> & gather_name_new_ptr_map,
    std::map<std::string,int*> & reduction_name_new_ptr_map,
    std::map<std::string,int*> & scatter_name_new_ptr_map,
    std::map<std::string,void*> & name_new_ptr_map,
    int table_column_num,
    const int vector
        ) ;
#endif
