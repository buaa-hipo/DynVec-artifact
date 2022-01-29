#ifndef NODE2STATE_HPP
#define NODE2STATE_HPP
#include "node.hpp"
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
#include "statement.hpp"
void node_tree2state(
        const std::string & output_name,
        const std::vector<std::string> &input_var_vec,
        const std::set<std::string> &iterates_set,
        const std::map<std::string, Type > & name_type_map,
        const std::map<std::string,int*>  &gather_name_new_ptr_map,
        const std::map<std::string,int*>  &reduction_name_new_ptr_map,
        const std::map<std::string,int*>  &scatter_name_new_ptr_map,
        const std::map<std::string,void*>  &name_new_ptr_map,
        std::map<std::string,Varience*> &name_var_map,
        std::map< std::string , Varience *> &name_varP_varVP_map,
        std::map< std::string , Varience *> &name_varP_varPV_map,
        std::map<std::string, Varience*>  &gather_name_new_var_map,
        std::map<std::string, Varience*>  &reduction_name_new_var_map,
        std::map<std::string, Varience*>  &scatter_name_new_var_map,
        std::map<std::string, Varience*>  &name_new_var_map,
        std::vector<StateMent *> &func_init_state_vec,
        StateMent * & calculate_state,
        FuncStatement * &func_state_ptr,
         Node * node_ptr,
         const int vector
        ) ;
#endif
