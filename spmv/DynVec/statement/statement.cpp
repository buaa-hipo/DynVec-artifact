#include "statement.hpp"
    StateMent* CombinStatVec( const std::vector<StateMent*> &stat_vec ) {
        if( stat_vec.size() == 0 ) { 
		    return Nop::make(); 
	    }
        else if(stat_vec.size() == 1) {return stat_vec[0];}
        else {
            StateMent * block_stat = Block::make( stat_vec[0],stat_vec[1]);
            for(unsigned int i = 2 ; i < stat_vec.size(); i++ ) {
                block_stat = Block::make( block_stat, stat_vec[i] );
            }
            return block_stat;
        }
    }


std::map<std::string,int> classname_typeid_map = {
    {"statement",0},
    {"block",1},
    {"for",2},
    {"varience",3},
    {"const",4},
    {"let_stat",5},
    {"inc_addr",6},
    {"nop",7},
    {"scatter",8},
    {"init",9},
    {"gather",10},
    {"load",11},
    {"store",12},
    {"shuffle",13},
    {"reduce",14},
    {"bitcast",15},
    {"binary",16},
    {"Add",17},
    {"Mul",18},
    {"broadcast",19},
    {"print",20},

    {"Minus",21},

    {"Div",22},
    {"complex_reduce",23},
    {"detect_conflict",24},
    {"insert_element",25},
    {"extract_element",26},
    { "select", 27 },
    {"ICmpEQ",28}
    //static constexpr const char* class_name_ = "broadcast";
};
int GetTypeId(std::string class_name) {
    auto typeid_find = classname_typeid_map.find(class_name);
    if( typeid_find == classname_typeid_map.end()) {
        LOG(FATAL)<<"Can not find " << class_name << "\n";
        exit(1);
    } 
    return typeid_find->second;
}

