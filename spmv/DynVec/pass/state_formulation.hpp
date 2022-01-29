#ifndef STATE_FORMULATION_HPP
#define STATE_FORMULATION_HPP

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

StateMent * formulation_state( 
        const std::map<std::string, Varience *> &name_varP_varVP_map,
        const std::map<std::string, Varience *> &name_varP_varPV_map,
        StateMent * state ) ;
#endif
