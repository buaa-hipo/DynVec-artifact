#include <string>
#include <set>
#include <vector>
#include <map>
#include <unordered_map>
#include "node.hpp"
#include "parse.hpp"
////
//example : lambda i : y[ row[i] ] += x[column[i]] * data[i]
//
    std::ostream& operator<< ( std::ostream & stream, const Token& token  )  {
        std::string token_type_str[] = {
            "Input",
            "Output",
            "Float",
            "Double",
            "Int",
            "FloatPtr",
            "DoublePtr",
            "IntPtr",
            "Var",
            "Lambda",
            "LeftBracket",
            "RightBracket",
            "Colon",
            "Mult",
            "Add",
            "MultEquel",
            "Equel",
            "AddEquel",
            "Div",
            "End",
            "Comma"
        }; 

        stream << token_type_str[token.token_type_];
        stream << " " << token.token_name_;
        return stream;
    }


class ParseClass{
    private:
    unsigned int max_bits_;
    std::vector<std::string> scatter_impermulate_var_set_;
    Type output_var_type_;
    Token current_token_;
    //output
    Node * &root_node_ptr_;
    std::set<std::string> &gather_set_;
    std::set<std::string> &scatter_set_;
    std::set<std::string> &reduction_set_;
    std::set<std::string> &load_set_;
    std::map<std::string, Type> &name_type_map_;
    
    std::vector<std::string> &input_name_vec_;

    std::set<std::string> &iterates_set_;

    std::string & output_name_;
    public:
    ParseClass(
        Node * &root_node_ptr,
        std::set<std::string> &gather_set,
        std::set<std::string> &scatter_set,
        std::set<std::string> &reduction_set,
        std::set<std::string> &load_set,
        std::map<std::string, Type> &name_type_map,
        std::vector<std::string> & input_name_vec,
        std::set<std::string> & iterates_set,
        std::string & output_name
        ):
        max_bits_(0),
        root_node_ptr_(root_node_ptr),
        gather_set_(gather_set),
        scatter_set_(scatter_set),
        reduction_set_(reduction_set),
        load_set_(load_set),
        name_type_map_(name_type_map),
        input_name_vec_(input_name_vec),
        iterates_set_(iterates_set),
        output_name_(output_name)
    {

    }
    private:

    const std::vector<std::string> & get_input_name_vec() {
        return input_name_vec_;
    }
    Type get_var_type( const std::string & var_name ) {
        auto  it = name_type_map_.find( var_name );
        if( it == name_type_map_.end() ) {
            LOG(FATAL) << "Can not find " << var_name;
        }
        return it->second;
    }

Token get_next_token(const std::string &expr) {
    static unsigned int index = 0;
    Token token;
    for( ; index < expr.size() && (expr[index] == ' ' || expr[index]=='\n');  index++ ) ;

    token.token_type_ = End;

    if( index >= expr.size() ) {
        return  token;
    } 
    std::string token_name = "";
     
    if( isalpha(expr[index]) ||
          expr[index] == '_') {
        do {
            token_name += expr[index];
            index++;
        } while( isalnum( expr[index] ) ||
                 expr[index] == '_');
        if( token_name == "lambda" ) {
            token.token_type_ = Lambda;
        } else if(token_name == "input") {
            token.token_type_ = Input;
        } else if(token_name == "output") {
            token.token_type_ = Output;
        } else if(token_name == "float") {
            token.token_type_ = Float;
        } else if(token_name == "double") {
            
            token.token_type_ = Double;
        } else if(token_name == "int") {
            
            token.token_type_ = Int;
        } else {
            token.token_type_ = Var;
            token.token_name_ = token_name;
        }
    } else if( expr[index] == ':' ) {
        index++;
        token.token_type_ = Colon;
    } else if( expr[index] == '+' ) {
        index++;
        if(expr[index] == '=') {
            index++;
            token.token_type_ = AddEquel;
        } else {
            token.token_type_ = Add;
        }
    } else if( expr[index] == '*' ) {
         index++;
        if(expr[index] == '=') {
            index++;
            token.token_type_ = MultEquel;
        } else {
            token.token_type_ = Mult;
        }
    } else if(expr[index]=='/') {
        index++;
        token.token_type_ = Div;
    } else if( expr[index] == '[') {
        index++;
        token.token_type_ = LeftBracket;
    } else if( expr[index] == ']' ){
        index++;

        token.token_type_ = RightBracket;
    } else if( expr[index] == ',') {
        index++;

        token.token_type_ = Comma;
    }
    return token; 
}
void ParseOutput(const std::string & expr ) {
        Type var_type;
        current_token_ = get_next_token( expr);
        if(current_token_.token_type_ != Colon) {
            LOG(FATAL) << "Input misses a colon"; 
        }

        current_token_ = get_next_token( expr);
        
        TokenType current_token_type = current_token_.token_type_;
        
        current_token_ = get_next_token( expr);
        bool is_pointer = false;
        if( current_token_.token_type_ == Mult ) {
            is_pointer = true; 
        }

        if(current_token_type == Float) {

            max_bits_ = max_bits_ > sizeof(float) * ByteSize ? max_bits_ : sizeof(float) * ByteSize;
            if( is_pointer ) 
                var_type = __float_ptr;
            else
                var_type = __float;
        } else if(current_token_type == Double) {

            max_bits_ = max_bits_ > sizeof(double) * ByteSize ? max_bits_ : sizeof(double) * ByteSize;
            if( is_pointer )
                var_type = __double_ptr;
            else
                var_type = __double;
        } else if(current_token_type == Int) {

            max_bits_ = max_bits_ > sizeof(int) * ByteSize ? max_bits_ : sizeof(int) * ByteSize;
            if( is_pointer )   
            var_type = __int_ptr;
            else

            var_type = __int;
        } else {
        
            LOG(FATAL) << "input miss a type"; 
        }

        current_token_ = get_next_token( expr);
        if(current_token_.token_type_ == Var) {
            output_name_ = current_token_.token_name_;
            output_var_type_ = var_type;
            name_type_map_[ output_name_ ] = var_type;
        } else {
            LOG(FATAL) << "input miss a varience"; 
        }

        current_token_ = get_next_token( expr);
}
void ParseLambda(const std::string & expr ) {

    do{ 

        current_token_ = get_next_token( expr);
        if(current_token_.token_type_ == Var) {
            iterates_set_.insert( current_token_.token_name_); 
        } else {
            LOG(FATAL) << "input miss a varience"; 
        }

        current_token_ = get_next_token( expr);
    }while( current_token_.token_type_ == Comma ); 
}

void ParseInput(const std::string & expr ) {

    current_token_ = get_next_token( expr);
    if(current_token_.token_type_ != Colon) {
        LOG(FATAL) << "Input misses a colon" << current_token_; 
    }
    do{ 
        current_token_ = get_next_token( expr);
        TokenType current_token_type = current_token_.token_type_;
        current_token_ = get_next_token( expr);
        bool is_pointer = false;
        if( current_token_.token_type_ == Mult ) {
            is_pointer = true;
            current_token_ = get_next_token( expr);
        }
        Type var_type;
        if(current_token_type == Float) {
            max_bits_ = max_bits_ > sizeof(float) * ByteSize ? max_bits_ : sizeof(float) * ByteSize;
            if(is_pointer)
                var_type = __float_ptr;
            else
                var_type = __float;
        } else if(current_token_type == Double) {

            max_bits_ = max_bits_ > sizeof(double) * ByteSize ? max_bits_ : sizeof(double) * ByteSize;
            if(is_pointer)
                var_type = __double_ptr;
            else
                var_type = __double;
        } else if(current_token_type == Int) {

            max_bits_ = max_bits_ > sizeof(int) * ByteSize ? max_bits_ : sizeof(int) * ByteSize;
            if(is_pointer)
                var_type = __int_ptr;
            else
                var_type = __int;
        } else {
        
            LOG(FATAL) << "input miss a type"; 
        }

        if(current_token_.token_type_ == Var) {
            name_type_map_[ current_token_.token_name_ ] = var_type;
            input_name_vec_.push_back(current_token_.token_name_);
        } else {
            LOG(FATAL) << "input miss a varience"; 
        }

        current_token_ = get_next_token( expr);
    }while( current_token_.token_type_ == Comma ); 
}

Node* ParseInputExpr(const std::string & expr) {
    
    current_token_ = get_next_token( expr);

    Node * left_node_ptr = NULL;
    if(current_token_.token_type_ == Var) {
        std::string left_node_name = current_token_.token_name_;
        auto var_type_it = name_type_map_.find( left_node_name );
        Type var_type;
        if(var_type_it == name_type_map_.end()) {
            auto var_type_set_it = iterates_set_.find( left_node_name );
            if(var_type_set_it == iterates_set_.end()) {
                LOG(FATAL) << left_node_name<<" not defined";
            } else {
                var_type = __int;
            }
        } else { 
            var_type = var_type_it->second;
        }
        left_node_ptr = new VarNode(var_type, left_node_name  );
        
        current_token_ = get_next_token( expr);
        if( current_token_.token_type_ == LeftBracket ) { 
            Node * left_node_index_ptr = ParseInputExpr( expr ); 
            if( current_token_.token_type_ != RightBracket) {
                LOG(FATAL) << "the input expression misses a right bracket";
            } else {
                current_token_ = get_next_token( expr);
            }
            if( dynamic_cast<LoadNode*>( left_node_index_ptr) != NULL ) {
                left_node_ptr = new GatherNode( left_node_ptr,  left_node_index_ptr );
                left_node_ptr->addr_name_ = left_node_name;
                left_node_ptr->index_name_ = left_node_index_ptr->addr_name_;

                left_node_ptr->node_name_ = left_node_ptr->index_name_;
                gather_set_.insert( left_node_ptr->index_name_ );
                load_set_.erase( left_node_ptr->index_name_ );
            } else if( dynamic_cast<VarNode*>(left_node_index_ptr) != NULL && iterates_set_.find(left_node_index_ptr->node_name_) != iterates_set_.end() ){

                left_node_ptr = new LoadNode( left_node_ptr );
                 
                left_node_ptr->index_name_ = left_node_index_ptr->node_name_;
                left_node_ptr->addr_name_ = left_node_name;

                left_node_ptr->node_name_ = left_node_ptr->index_name_;
                load_set_.insert(left_node_name);

            } else {
                LOG(FATAL) << "unsupported";
            }
        } 
    } else {
        LOG(FATAL) << "the input expression misses a varience";
    }
    TokenType current_token_type = current_token_.token_type_;
    Node * right_node_ptr = NULL;
    if(current_token_type == Add ||
       current_token_type == Mult ||
       current_token_type == Div) {
       right_node_ptr = ParseInputExpr( expr );
    }
    Node * res_node_ptr;
    if(current_token_type == Add ) {
        res_node_ptr = new AddNode( left_node_ptr,right_node_ptr );
    } else if( current_token_type == Mult ){ 
        res_node_ptr = new MultNode( left_node_ptr,right_node_ptr );
    } else if( current_token_type == Div) {
        //LOG(INFO) << "Div";
        res_node_ptr = new DivNode( left_node_ptr,right_node_ptr );
    } else {
        res_node_ptr = left_node_ptr; 
    }
    CHECK(res_node_ptr!=NULL) << "Wrong";
    return res_node_ptr;
}
void ParseExpr(const std::string & expr) {

    current_token_ = get_next_token( expr);
    Node * addr_ptr = NULL;
    if( output_name_ != current_token_.token_name_ ) {
        LOG(FATAL) << "The defination of output is different with the output in the express";
    } else {
        addr_ptr = new VarNode( output_var_type_,output_name_ );
    }
    bool has_bracket = false; 
    current_token_ = get_next_token( expr);
    Node * index_node_ptr = NULL;
    if( current_token_.token_type_ ==  LeftBracket) {
        has_bracket = true;
        index_node_ptr = ParseInputExpr(expr);

        if(current_token_.token_type_ != RightBracket) {
            LOG(FATAL) << "Output miss a right bracked";
        } else {
              
            current_token_ = get_next_token( expr);
        }
    }
    TokenType equal_type = current_token_.token_type_;
    if( equal_type != AddEquel &&
        equal_type != Equel &&
        equal_type != MultEquel) {
        LOG(FATAL) << "equal type is fatal"; 
    }

    Node * data_ptr = ParseInputExpr( expr );
    if(has_bracket) {
    if( dynamic_cast<LoadNode*>(index_node_ptr) != NULL ) {
        if( equal_type == AddEquel ) {
            Node * gather_node_ptr = new GatherNode( addr_ptr, index_node_ptr);
            gather_node_ptr->index_name_ = index_node_ptr->addr_name_;
            gather_node_ptr->node_name_ = gather_node_ptr->index_name_;
            gather_node_ptr->addr_name_ = output_name_;
            data_ptr = new AddNode( gather_node_ptr, data_ptr );
            data_ptr->node_name_ = gather_node_ptr->addr_name_;

            data_ptr->index_name_ = gather_node_ptr->index_name_;
            
            reduction_set_.insert( gather_node_ptr->index_name_ );

        } else if(equal_type == MultEquel) {
            LOG(FATAL)  << "Unsupported";
        }
        //LOG(INFO) << addr_ptr->node_name_ << " ";

        //LOG(INFO) << index_node_ptr->node_name_ << " ";
        //LOG(INFO) << data_ptr->node_name_ << " ";

        root_node_ptr_ = new ScatterNode( addr_ptr, index_node_ptr, data_ptr );

        root_node_ptr_->index_name_ = index_node_ptr->addr_name_;

        root_node_ptr_->addr_name_ = output_name_;

        scatter_set_.insert( root_node_ptr_->index_name_ );
        load_set_.erase( root_node_ptr_->index_name_ );
    } else if(dynamic_cast<VarNode*>(index_node_ptr)!= NULL) {
        if( equal_type == AddEquel ) {
            LoadNode * load_node_ptr = new LoadNode( addr_ptr);
            load_node_ptr->index_name_ = index_node_ptr->node_name_;
            load_node_ptr->addr_name_ = output_name_;
            data_ptr = new AddNode( load_node_ptr, data_ptr );
            data_ptr->node_name_ = load_node_ptr->index_name_;

        } else if(equal_type == MultEquel) {
            LOG(FATAL)  << "Unsupported";
        } 

        root_node_ptr_ = new StoreNode( addr_ptr, data_ptr );
        root_node_ptr_->index_name_ = index_node_ptr->addr_name_;

        root_node_ptr_->addr_name_ = output_name_;

    } else {
        LOG(FATAL) << "Unsupported"; 
    }
    } else {
        LOG(FATAL) << "unsupported";
    }
}
public:
int Parse(const std::string & expr ) {

    current_token_ = get_next_token( expr);
    bool end_circle = false;
    do{
        switch(current_token_.token_type_) {
            case Output:
                ParseOutput( expr );
                break;
            case Input:
                ParseInput( expr );
                break;
            case Lambda:
                ParseLambda(expr);
                break;
            case Colon:
                ParseExpr(expr);
            case End:
                end_circle = true; 
                break;
            default:
                LOG(FATAL) << "Type Wrong" << current_token_;
                break;
        }
    }  while(!end_circle);  
    return max_bits_;
}
};
int parse_expression( 
        const std::string &expr_str,
        Node * &root_node_ptr,
        std::set<std::string> &gather_set,
        std::set<std::string> &scatter_set,
        std::set<std::string> &reduction_set,
        std::set<std::string> &load_set,
        std::map<std::string, Type >  &name_type_map,
        std::vector<std::string> & input_name_vec,
        std::set<std::string> & iterates_set,
        std::string & output_name
        ) {

        ParseClass parse_class = ParseClass(  
                root_node_ptr, 
                gather_set, 
                scatter_set,
                reduction_set,
                load_set,
                name_type_map,
                input_name_vec,
                iterates_set,
                output_name
                );
        const int max_bits_ = parse_class.Parse( expr_str );
        #ifdef __AVX512CD__
            const int vector_bits = 512;

            return vector_bits / max_bits_;
        #else 
            #ifdef __AVX2__
            const int vector_bits = 256;

            return vector_bits / max_bits_;
            #else
            LOG(FATAL) << "Unsupported architetures";
            #endif
        #endif
}


