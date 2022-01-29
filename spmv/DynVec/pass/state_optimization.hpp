#ifndef STATE_OPTIMIZATION_HPP
#define STATE_OPTIMIZATION_HPP
#include "statement.hpp"
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

StateMent * optimization_state( 
        const std::map<std::string,GatherInfo*> &gather_map,
        const std::map<std::string,ScatterInfo*> &scatter_map,
        const std::map<std::string,ReductionInfo*> &reduction_map,
        const std::map<std::string,Varience*> &name_var_map,
        const std::map< std::string , Varience *> &name_varP_varVP_map,
        const std::map< std::string , Varience *> &name_varP_varPV_map,
        const std::map<std::string, Varience*>  &gather_name_new_var_map,
        const std::map<std::string, Varience*>  &reduction_name_new_var_map,
        const std::map<std::string, Varience*>  &scatter_name_new_var_map,
        const std::map<std::string, Varience*>  &name_new_var_map,
        const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
        const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&  same_feature_range_map,
        const std::string & output_name,
        StateMent * code_seed,
        const int vector
        ); 
#endif
