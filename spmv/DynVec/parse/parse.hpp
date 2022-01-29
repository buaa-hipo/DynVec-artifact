#ifndef PARSE_HPP
#define PARSE_HPP
#include "type.hpp"
#include "log.h"
#include "node.hpp"
#include <string>
typedef enum{
    Input = 0, 
    Output = 1,
    Float = 2,
    Double = 3,
    Int = 4,
    FloatPtr = 5,
    DoublePtr = 6,
    IntPtr = 7,
    Var = 8, 
    Lambda = 9,
    LeftBracket = 10, 
    RightBracket = 11,
    Colon = 12, 
    Mult = 13, 
    Add = 14,
    MultEquel = 15,
    Equel = 16,
    AddEquel = 17,
    Div = 18, 
    End = 19,
    Comma = 20 } TokenType;
typedef struct Token {
    TokenType token_type_;
    std::string token_name_;
} Token; 
//typedef enum{ Mult, Add, Div , Gather, Scatter,Load } OpType;
//typedef enum { var_float,var_double,var_int,var_float_ptr,var_double_ptr,var_int_ptr } VarType;
/*
VarType GetBasicType( VarType var_type) {
     switch( var_type  ) {
        case FloatPtr:
            return var_float;
        case DoublePtr:
            return var_double;
        case IntPtr:
            return var_int;
        default:
            LOG(FATAL) << "Transfer fault";
    }

}
*/
/*Type TokenType2Type( TokenType token_type ) {

    switch( token_type  ) {
        case FloatPtr:
            return __float_ptr;
        case DoublePtr:
            return __double_ptr;
        case IntPtr:
            return __int_ptr;
        case Float:
            return __float;
        case Double:
            return __double;
        case Int:
            return __int;
        default:
            LOG(FATAL) << "Transfer fault";
    }
}
*/

int parse_expression( 
        const std::string &expr_str,
        Node * &root_node_ptr,
        std::set<std::string> &gather_set,
        std::set<std::string> &scatter_set,
        std::set<std::string> &reduction_set,
        std::set<std::string> &load_set,
        std::map<std::string, Type >  &name_type_map,
        std::vector<std::string> & input_var_vec,
        std::set<std::string> & iterates_set,
        std::string & output_name
        ) ;

#endif
