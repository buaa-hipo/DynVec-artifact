#ifndef STATEMENT_HPP
#define STATEMENT_HPP
#include "type.hpp"
#include "log.h"
#include <vector>
#include <sstream>
#include <map>

#include <set>
extern std::map<std::string,int>  classname_typeid_map;
int GetTypeId( std::string class_name );

class StateMent {
    protected:
    Type type_;
    public:
    std::string node_name_;
    std::string addr_name_;
    std::string index_name_;
    static constexpr const char* class_name_ = "statement";
    virtual std::string get_class_name() {
        return class_name_;
    }

    StateMent( )  {
    }
    virtual Type& get_type() {
        return type_;
    }
    virtual void set_node_name( std::string var_name ) {
        node_name_ = var_name;
    }
    virtual void set_addr_name( std::string var_name ) {
        addr_name_ = var_name;
    }
    virtual void set_index_name( std::string var_name ) {
        index_name_ = var_name;
    }

    virtual void set_type( Type type ) {
        type_ = type;
    }
};
class Varience :public StateMent{
    std::string name_;
    bool is_const_;
    void * ptr_;
    std::set<std::string> name_set_;
    std::string get_unique_name() {
        static int name_index = 0;
        std::stringstream ss ;
        ss << "__" << name_index;
        name_index++;
        auto name_set_find_ = name_set_.find( ss.str() );
        while(name_set_find_ != name_set_.end() ) {
            ss.str(std::string());
            ss << "__" << name_index;
            name_index++;
            name_set_find_ = name_set_.find( ss.str() );
        }
        name_set_.insert( ss.str() );
        return ss.str();
    }
    public:
    
    static constexpr const char * class_name_ = "varience";
    Varience() {
        LOG(FATAL) << "please use other constructe functions";
    }
    Varience(const Type &type, std::string & name, bool is_const = true ) {
        type_ = type;
        node_name_ = name;
        name_ = get_unique_name() + "_" + name;
        is_const_ = is_const;
    }

    Varience(const Type &type, bool is_const = true ) {
        type_ = type;
        name_ = get_unique_name();
        is_const_ = is_const;
    }
    Varience(double * data) {
        type_ = Type( &__double , NOT_VEC);
        ptr_ = reinterpret_cast<void*>(data);
        name_ = get_unique_name();
        is_const_ = true;
    }
    Varience(int * data) {
        type_ = Type( &__int,NOT_VEC);
        ptr_ = reinterpret_cast<void*>(data);
        name_ = get_unique_name();
        is_const_ = true;
    }
    virtual Type& get_type() {
        return type_;
    } 
    std::string get_type_str() {
        return type_.str();
    }
    std::string get_name()const {
        return name_;
    }
    bool get_is_const() {
        return is_const_;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    friend std::ostream& operator<< (std::ostream & stream, const Varience& var) {
        stream << var.get_name();
        return stream;
    }
};
class Nop : public StateMent{

    protected:
        Nop(){
        }
    public:

    static constexpr const char* class_name_ = "nop";
    static StateMent * make() {
            StateMent * stat_ptr = new Nop();
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }

};

class FuncStatement : public StateMent {
    Varience * ret_;
    StateMent * state_;
    std::string func_name_;

    std::vector<Varience * > *args_;
    public:
    static constexpr const char* class_name_ = "func";
    virtual std::string get_class_name() {
        return class_name_;
    }
    FuncStatement( const Type &ret_type, const std::vector<Type> &args_type, const std::vector<std::string> & args_name ) {
        ret_ = new Varience(ret_type);
        args_ = new std::vector<Varience*>();
        func_name_ = "function" ;
        const int args_len = args_type.size();
        args_->resize( args_len,nullptr );
        for(int i = 0 ; i < args_len ; i++) {
            (*args_)[i] = new Varience(args_type[i]); 
        }
    }

    FuncStatement( const Type &ret_type, const std::vector<Type> &args_type) {
        ret_ = new Varience(ret_type);
        args_ = new std::vector<Varience*>();
        func_name_ = "function" ;
        const int args_len = args_type.size();
        args_->resize( args_len,nullptr );
        for(int i = 0 ; i < args_len ; i++) {
            (*args_)[i] = new Varience(args_type[i]); 
        }
    }
    const std::vector<Varience*> * get_args() {
        return args_;
    }
    Varience * get_ret() {
        return ret_;
    }
    void set_state(StateMent * state) {
        state_ = state;
        return;
    }
    StateMent * get_state() {
        return state_;
    }
    std::string & get_func_name() {
        return func_name_;
    }
};

class Expr :public StateMent {
    public :
    static constexpr const char* class_name_ = "expr";
    virtual std::string get_class_name() {
        return class_name_;
    }

    Expr( )  {
    }
    virtual Type& get_type() {
        return type_;
    } 
};
class Block : public StateMent{
    std::vector<StateMent*> * state_vec_ptr;
//    StateMent* stat1_;
//    StateMent* stat2_;
    protected:
        Block( std::vector<StateMent*> * state_vec_ptr_pata ):state_vec_ptr( state_vec_ptr_pata ) {

        }

        Block(StateMent*stat1 , StateMent * stat2  ) {
            state_vec_ptr = new std::vector<StateMent*>();
            state_vec_ptr->resize( 2,nullptr);

            (*state_vec_ptr)[0] = stat1;
            (*state_vec_ptr)[1] = stat2;
        }

    public:

    static constexpr const char* class_name_ = "block";

    static  StateMent * make( std::vector<StateMent *> * stat ) {
        return new Block(stat);
    }
    static  StateMent * make( StateMent * stat1,StateMent * stat2 ) {
        Block * stat1_block = dynamic_cast<Block*>(stat1 ); 
        Block * stat2_block = dynamic_cast<Block*>(stat2);

        StateMent * state_ptr;
        if( stat1_block != nullptr && stat2_block != nullptr ) {
            stat1_block->get_stat_vec()->push_back(stat2);
            state_ptr = stat1_block;

        } else if(stat1_block != nullptr ) {
            stat1_block->get_stat_vec()->push_back(stat2);
            state_ptr = stat1_block;
        } else if(stat2_block != nullptr) {
      
            state_ptr = new Block( stat1, stat2 );
        } else {
        
            state_ptr = new Block( stat1, stat2 );
        }
//            StateMent * stat_ptr = new Block(stat1,stat2);
        return state_ptr;
    }

    std::vector<StateMent *> * get_stat_vec() {
        return state_vec_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }


};
class Print : public StateMent{
    
    StateMent * var_;
    protected:
    Print( StateMent * var ) : var_(var) {
        type_ = var->get_type();
    }
    public:
    static constexpr const char* class_name_ = "print";

    static StateMent * make( StateMent * var )  {
        StateMent * stat_ptr = new Print(var);
        return stat_ptr;
    }
    StateMent * get_var() {
        return var_;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }

};
class For : public StateMent {
    Varience * var_;
    StateMent * begin_;
    StateMent * space_;
    StateMent * end_;
    StateMent * stat_;

    protected:
    For(StateMent * begin, StateMent * space, StateMent*end, StateMent * stat):begin_(begin), space_(space),end_(end),stat_(stat){
        var_ = new Varience( __int );
    }
    For(StateMent * begin, StateMent * space, StateMent*end):begin_(begin), space_(space),end_(end){
        var_ = new Varience( __int );
    }
    public:

    static constexpr const char* class_name_ = "for";
    static StateMent * make( StateMent * begin,StateMent * space,StateMent * end ){
         
        StateMent * stat_ptr = new For( begin,space,end);
        return stat_ptr;
    }

    static StateMent * make( StateMent * begin,StateMent * space,StateMent * end, StateMent * stat ){
         
        StateMent * stat_ptr = new For( begin,space,end,stat );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }

    void SetState( StateMent * stat ) {
        stat_ = stat;
    }
    StateMent * get_begin() {
        return begin_;
    }
    StateMent * get_space() {
        return space_;
    } 
    StateMent * get_end() {
        return end_;
    } 
    StateMent * get_stat() {
        return stat_;
    }
    Varience * get_var() {
        return var_;
    }

};
class Const : public Expr{
    void * data_;
    public:

    static constexpr const char * class_name_ = "const";
    template<typename T>
    Const( T data) {
        if( typeid(T) == typeid(int) ) {
            type_ = __int;
        } else if( typeid(T) == typeid(double) ) {
            type_ = __double;
        } else if( typeid(T) == typeid(bool) ) {
            type_ = __bool;
        } else if( typeid(uint64_t) == typeid(T) ){
            type_ = __dynvec_int64;
        } else if( typeid(float) == typeid(T) ){
            type_ = __float;
        } else if( typeid(int8_t) == typeid(T) ) {
            type_ = __dynvec_int8;
        } else if( typeid(char) == typeid(T) ){
            type_ = __dynvec_int8;
        } else {
            LOG(FATAL) << "Unsupport Type"  ;
        }
        T * ptr = (T*)malloc(sizeof(T));
        ptr[0] = data;
        data_ = reinterpret_cast<void*>(ptr);
    }
    template<typename T>
    Const(T * data, int lanes) {
        if( typeid(T) == typeid(int) ) {
            type_ = Type( INT,  lanes);
        } else if( typeid(T) == typeid(double) ) {
            type_ = Type(DOUBLE, lanes);
        } else if( typeid(T) == typeid(bool) ) {
            type_ = Type(BOOL,lanes);
        } else if( typeid(T) == typeid(float) ){
            type_ = Type(FLOAT,lanes);
        } else if( typeid(T) == typeid(int8_t) ){ 
            type_ = Type(INT8,lanes);
        } else{
            LOG(FATAL) << "Unsupport Type";
        }
        T * tmp = ( T * )malloc(sizeof(T)*lanes);
        for( int i = 0 ; i < lanes ; i++ ) {
            tmp[i] = data[i];
        }
        data_ = reinterpret_cast<void*>(tmp);
    }

    std::string get_type_str() {
        return type_.str();
    }
    template<typename class_name>
    void print_data(class_name* tmp_d, int lanes, std::stringstream &ss ) {
        int i;
        for( i = 0; i < lanes - 1; i++  ) {
            if(typeid(class_name)==typeid( int8_t ))
                ss << (int)tmp_d[i] << ","; 
            else
                ss << tmp_d[i] << ","; 
        }
        if(typeid(class_name)==typeid( int8_t ))
                ss << (int)tmp_d[i] ; 
        else
                ss << tmp_d[i] ; 
    }
    std::string get_data_str() {
        std::stringstream ss;
        const int lanes = type_.get_lanes();
        switch( type_.get_data_type() ) {
            case DOUBLE:
                print_data( reinterpret_cast<double*>(data_),lanes,ss);
                break;
            case INT:
                print_data( reinterpret_cast<int*>(data_),lanes,ss);
                break;
            case BOOL:
                print_data( reinterpret_cast<bool*>(data_),lanes,ss);
                break;
            case INT64:
                print_data( reinterpret_cast<int64_t*>(data_),lanes,ss);
                break;
            case INT8:
                print_data( reinterpret_cast<int8_t*>(data_),lanes,ss);
                break;
            case FLOAT:
                print_data( reinterpret_cast<float*>(data_),lanes,ss);
                break;
            default:
                LOG(FATAL) << "Unsupported";
                break;
        }
        return ss.str();
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    void * get_data() {
        return data_;
    }
};
/*class Provide:public StateMent{
    Varience * res_;
    StateMent * expr_;
    
    protected:
    Provide(Varience * res, StateMent * expr) : res_(res),expr_(expr) {
    }
    public:

    static constexpr const char* class_name_ = "provide";
    static StateMent * make( Varience * res,StateMent * expr) {
            StateMent * stat_ptr = new Provide(res,expr);
            return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    Varience * get_res() {
        return res_;
    }
    StateMent * get_expr() {
        return expr_;
    }


}*/
class LetStat:public StateMent{
    Varience * res_;
    StateMent * expr_;
    protected:
    LetStat(Varience * res, StateMent * expr) : res_(res),expr_(expr) {
            node_name_ = expr->node_name_;
            index_name_ = expr->index_name_;
            addr_name_ = expr->addr_name_;
        }
    public:

    static constexpr const char* class_name_ = "let_stat";
    static StateMent * make(Varience * res,StateMent * expr) {
        const Type & res_type = res->get_type();
        const Type & expr_type = expr->get_type();
        CHECK( res_type == expr_type ) << class_name_ <<": " <<res_type<< " "<< res->get_name() << " <-> " << expr_type << "does not match\n" <<
            expr->get_class_name();
        StateMent * stat_ptr = new LetStat(res,expr);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    Varience * get_res() {
        return res_;
    }
    StateMent * get_expr() {
        return expr_;
    }
};


class IncAddr : public Expr {
    StateMent* addr_;
    StateMent* inc_;
    
    protected:
    IncAddr(StateMent * addr, StateMent * inc) : addr_(addr),inc_(inc){
        type_ = addr->get_type();
    }
    public:

    static constexpr const char* class_name_ = "inc_addr";
    static StateMent * make( StateMent * addr , StateMent * inc) {
        const Type & addr_type = addr->get_type();
        const Type & data_type = inc->get_type();
        CHECK( (addr_type.is_pointer()) ) << addr_type << " " << data_type << "does not match\n";

            StateMent * stat_ptr = new IncAddr(addr,inc);
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() {
        return addr_;
    }
    StateMent * get_inc() {
        return inc_;
    }
};
class Scatter: public StateMent{
    StateMent * addr_;
    StateMent * index_;
    StateMent * data_;
    StateMent * mask_;
    protected:
        Scatter( StateMent * addr,StateMent * index,StateMent * data ,StateMent * mask): addr_(addr),index_(index),data_(data),mask_(mask) {
            node_name_ = addr->node_name_;
            addr_name_ = node_name_;
            index_name_ = index->addr_name_;

        }
    public:

    static constexpr const char* class_name_ = "scatter";
    static    StateMent * make( StateMent * addr, StateMent * index,StateMent * data, StateMent * mask ) {
            StateMent * stat_ptr = new Scatter(addr,index,data,mask);
            return stat_ptr;
        }
    static    StateMent * make( StateMent * addr, StateMent * index,StateMent * data ) {
            StateMent * mask = NULL;
            StateMent * stat_ptr = new Scatter(addr,index,data,mask);
            return stat_ptr;
        }

    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() {
        return addr_;
    }
    StateMent * get_index() {
        return index_;
    }
    StateMent * get_data() {
        return data_;
    }
    StateMent * get_mask() {
        return mask_;
    }
};

class Gather: public Expr{
    StateMent * addr_;
    StateMent * index_;
    StateMent * mask_;
    protected:
        Gather( StateMent * addr,StateMent * index,StateMent* mask ): addr_(addr),index_(index),mask_(mask) {
            Type * type_ptr_tmp = &addr->get_type();

            node_name_ = addr->node_name_;
            addr_name_ = node_name_;
            index_name_ = index->addr_name_;
            
            type_ = *type_ptr_tmp->get_pointer2type();
            type_.set_lanes(type_ptr_tmp->get_lanes());
        }
    public:

    static constexpr const char* class_name_ = "gather";
    static  StateMent * make( StateMent * addr, StateMent * index , StateMent * mask) {
            StateMent * stat_ptr = new Gather(addr,index,mask);
            return stat_ptr;
        }
    static  StateMent * make( StateMent * addr, StateMent * index ) {
        StateMent * mask = NULL;
        StateMent * stat_ptr = new Gather(addr,index,mask);
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent* get_addr() {
        return addr_;
    }
    StateMent* get_index() {
        return index_;
    }
    StateMent * get_mask() {
        return mask_;
    }

};

class Load : public Expr {
    StateMent * addr_;
    StateMent * mask_;
    bool has_mask_;
    bool is_alined_;
    protected:
    Load( StateMent * addr,bool is_alined):addr_(addr),is_alined_(is_alined) {
            has_mask_ = false;
            node_name_ = addr->node_name_;
            addr_name_ = node_name_;
            index_name_ = "";
            mask_ = NULL;
            Type * type_ptr_tmp = &addr->get_type();
            CHECK( type_ptr_tmp->is_pointer()) << "address should be pointer type\n" << *type_ptr_tmp;
            type_ = Type( *type_ptr_tmp->get_pointer2type());
    }
    Load( StateMent * addr, StateMent * mask, bool is_alined):addr_(addr),mask_(mask),is_alined_(is_alined) {
            has_mask_ = true;
            node_name_ = addr->node_name_;
            addr_name_ = node_name_;
            index_name_ = "";
            Type * type_ptr_tmp = &addr->get_type();
            CHECK( type_ptr_tmp->is_pointer()) << "address should be pointer type\n" << *type_ptr_tmp;
            type_ = Type( *type_ptr_tmp->get_pointer2type());
    }

    public:

    static constexpr const char* class_name_ = "load";
    static StateMent * make( StateMent * addr,bool is_alined = false) {
        StateMent * stat_ptr = new Load( addr ,is_alined);
        return stat_ptr;
    }
    static StateMent * make( StateMent * addr, StateMent * mask,bool is_alined = false) {
        StateMent * stat_ptr = new Load( addr,mask, is_alined );
        return stat_ptr;
    }

    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() {
        return addr_;
    } 
    bool has_mask() const {
        return has_mask_;
    }

    bool get_is_aligned() const {
        return is_alined_;
    }
    StateMent * get_mask() const {
        return mask_;
    }
};
class InsertElement :public StateMent {
    StateMent * to_;
    StateMent * from_;
    StateMent * index_;
    protected:
    InsertElement( StateMent * to ,StateMent * from, StateMent * index ) : to_(to),from_(from),index_(index){
        type_ = to->get_type();
    }

    public:

    static constexpr const char* class_name_ = "insert_element";
    static StateMent * make(StateMent * to,StateMent * from,StateMent * index) {
        StateMent * stat_ptr = new InsertElement(to, from,index);
        return stat_ptr;
    }

    virtual std::string get_class_name() {
        return class_name_;
    }
    virtual Type& get_type() {
        return type_;
    }
    StateMent * get_to() const {
        return from_;
    } 

    StateMent * get_from() const {
        return from_;
    } 
    StateMent * get_index() const {
        return index_;
    }
};



class ExtractElement :public StateMent {
    StateMent * from_;
    StateMent * index_;
    protected:
    ExtractElement( StateMent * from, StateMent * index ) : from_(from),index_(index){
        type_ = Type( from->get_type().get_data_type(), 1);
    }

    public:

    static constexpr const char* class_name_ = "extract_element";
    static StateMent * make(StateMent * from,StateMent * index) {

        StateMent * stat_ptr = new ExtractElement( from,index);
        return stat_ptr;
    }

    virtual std::string get_class_name() {
        return class_name_;
    }
    virtual Type& get_type() {
        return type_;
    }
    StateMent * get_from() const {
        return from_;
    } 
    StateMent * get_index() const {
        return index_;
    }
};



class Store :public StateMent {
    StateMent * addr_;
    StateMent * data_;
    StateMent * mask_;
    bool is_alined_ ;
    bool has_mask_;
    protected:
    Store( StateMent * addr, StateMent * data ,bool is_alined) : addr_(addr),data_(data),is_alined_(is_alined),has_mask_(false){
        node_name_ = addr->node_name_;
        addr_name_ = node_name_;
        index_name_ = "";
        type_ = data->get_type();
        mask_ =NULL;
    }

    Store( StateMent * addr, StateMent * data ,StateMent * mask,bool is_alined) : addr_(addr),data_(data),mask_(mask),is_alined_(is_alined),has_mask_(true){
        
        node_name_ = addr->node_name_;
        addr_name_ = node_name_;
        index_name_ = "";
        type_ = data->get_type();
    }

    public:

    static constexpr const char* class_name_ = "store";
    static StateMent * make(StateMent * addr,StateMent * data,StateMent * mask, bool is_alined=false) {
        const Type & addr_type = addr->get_type();
        const Type & data_type = data->get_type();
        CHECK( (*addr_type.get_pointer2type()) == data_type ) << addr_type << " <-> " << data_type << "does not match\n";
        StateMent * stat_ptr = new Store( addr,data ,mask,is_alined);
        return stat_ptr;
    }

    static StateMent * make(StateMent * addr,StateMent * data, bool is_alined=false) {
        const Type & addr_type = addr->get_type();
        const Type & data_type = data->get_type();
        CHECK( (*addr_type.get_pointer2type()) == data_type ) << addr_type << " <-> " << data_type << "does not match\n";
        StateMent * stat_ptr = new Store( addr,data ,is_alined);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    virtual Type& get_type() {
        return type_;
    }
    StateMent * get_addr() const {
        return addr_;
    } 
    StateMent * get_data() const {
        return data_;
    }
    bool has_mask() const {
        return has_mask_;
    }

    bool get_is_aligned() const {
        return is_alined_;
    }
    StateMent * get_mask() const {
        return mask_;
    }
};
class Select : public Expr {
    StateMent * v1_;
    StateMent * v2_;
    StateMent * index_;
    protected:

    Select( StateMent *v1,StateMent *v2, StateMent *index ) : v1_(v1),v2_(v2),index_(index) {
        CHECK( v1->get_type() == v2->get_type()) << "the type of v1 and v2 is not equal\n";
        CHECK( index->get_type() == __bool_v ) << "the lanes of index is not equal";
        type_ = v1->get_type();
    }
    public:

    static constexpr const char* class_name_ = "select";


    static StateMent * make( StateMent *v1,StateMent *v2, StateMent *index ){ 
        StateMent * stat_ptr = new Select(v1,v2,index);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_v2() {
        return v2_;
        
    }
    StateMent * get_index() {
        return index_;
    }


};

class Shuffle : public Expr {
    StateMent * v1_;
    StateMent * v2_;
    StateMent * index_;
    protected:
    Shuffle( StateMent *v1, StateMent *index ) : v1_(v1),index_(index) {
        type_ = v1->get_type();
        v2_ = NULL;
    }

    Shuffle( StateMent *v1,StateMent *v2, StateMent *index ) : v1_(v1),v2_(v2),index_(index) {
        CHECK( v1->get_type() == v2->get_type()) << "the type of v1 and v2 is not equal\n";
        CHECK( index->get_type() == __int_v ) << "the lanes of index is not equal";
        type_ = Type( v1->get_type().get_data_type(), index->get_type().get_lanes() );
    }
    public:

    static constexpr const char* class_name_ = "shuffle";
    static StateMent * make( StateMent *v1, StateMent *index ){ 
        StateMent * stat_ptr = new Shuffle(v1,index);
        return stat_ptr;
    }

    static StateMent * make( StateMent *v1,StateMent *v2, StateMent *index ){ 
        StateMent * stat_ptr = new Shuffle(v1,v2,index);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_v2() {
        return v2_;
        
    }
    StateMent * get_index() {
        return index_;
    }


};
class BroadCast : public Expr {
    
    StateMent * v1_;

    protected:

    BroadCast( StateMent * v1 ,const int vector) : v1_(v1){

        Type &  type_ptr_tmp = v1_->get_type();
        type_ = type_ptr_tmp;
        type_.set_lanes(vector);

    }
    public:
    static constexpr const char* class_name_ = "broadcast";
    static StateMent * make( StateMent * v1, const int vector ) {
        StateMent * stat_ptr = new BroadCast( v1,vector );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }

};
class DetectConflict : public Expr {

    StateMent * index_;
    protected:

    DetectConflict( StateMent * index ) : index_(index) {
        const int lanes = index_->get_type().get_lanes();
        type_ = Type( BOOL ,lanes );
    }
    public:
    static constexpr const char* class_name_ = "detect_conflict";
    static StateMent * make( StateMent * index ) {
        StateMent * stat_ptr = new DetectConflict( index);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_index() {
        return index_;
    }

};

class ComplexReduce : public Expr {
    StateMent * v1_;

    StateMent * shuffle_index_ptr_;
    StateMent * index_;
    int mask_;
    protected:

    ComplexReduce( StateMent * v1 , StateMent * index,StateMent * shuffle_index_ptr , int mask) : v1_(v1),shuffle_index_ptr_(shuffle_index_ptr),index_(index),mask_(mask) {
        Type * type_ptr_tmp = &v1_->get_type();
        type_ = *type_ptr_tmp;
    }
    public:
    static constexpr const char* class_name_ = "complex_reduce";
    static StateMent * make( StateMent * v1, StateMent * index, StateMent * shuffle_index_ptr,int mask = 0xffff ) {
        StateMent * stat_ptr = new ComplexReduce( v1, index, shuffle_index_ptr,mask );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_shuffle_index_ptr() {
        return shuffle_index_ptr_;
    }

    StateMent * get_index() {
        return index_;
    }
    int get_mask() {
        return mask_;
    }

};

class Reduce : public Expr {
    StateMent * v1_;

    protected:

    Reduce( StateMent * v1 ) : v1_(v1){
        Type * type_ptr_tmp = &v1_->get_type();
        type_ = *type_ptr_tmp;
        type_.set_lanes(1);
    }
    public:
    static constexpr const char* class_name_ = "reduce";
    static StateMent * make( StateMent * v1 ) {
        StateMent * stat_ptr = new Reduce( v1 );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }

};
class BitCast : public Expr {

    StateMent * v1_;
    protected:
    BitCast(StateMent * v1,const Type & to ) :v1_(v1) {
        type_ = to;
    }
    public:

    static constexpr const char* class_name_ = "bitcast";
    static StateMent * make( StateMent * v1,  const Type & to ) {
        StateMent * stat_ptr = new BitCast(v1,  to); 
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    std::string get_type_str() {
        return type_.str();
    }
    StateMent * get_v1() {
        return v1_;
    }

};
class ICmpEQ : public Expr {
    protected:
    StateMent * v1_;
    StateMent * v2_;

    ICmpEQ( StateMent * v1,StateMent * v2 ) : v1_(v1),v2_(v2) {
        type_ = __bool_v;
    }
    public:

    static constexpr const char* class_name_ = "ICmpEQ";
    static StateMent * make( StateMent * v1,StateMent * v2 ) {
        StateMent * stat_ptr = new ICmpEQ( v1,v2 ); 
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_v2() {
        return v2_;
    }

};

class Binary : public Expr {
    protected:
    StateMent * v1_;
    StateMent * v2_;

    Binary( StateMent * v1,StateMent * v2 ) : v1_(v1),v2_(v2) {
        type_ = v1->get_type();
        node_name_ = v1->index_name_;

    }
    public:

    static constexpr const char* class_name_ = "binary";
    static StateMent * make( StateMent * v1,StateMent * v2 ) {
        StateMent * stat_ptr = new Binary( v1,v2 ); 
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_v2() {
        return v2_;
    }

};
#define BINARY( CLASS_NAME ) \
class CLASS_NAME : public Binary{ \
    protected:\
    CLASS_NAME(  StateMent * v1,StateMent*v2 ): Binary( v1,v2) { \
    } \
    public:\
    static constexpr const char* class_name_ = #CLASS_NAME;\
    static StateMent *make( StateMent* v1,StateMent*v2 ) {\
        StateMent * stat_ptr = new CLASS_NAME( v1,v2);\
        return stat_ptr;\
    }\
    virtual std::string get_class_name() {\
        return class_name_;\
    }\
}

BINARY(Div);
BINARY(Minus);
BINARY(Mul);
BINARY(Add);

StateMent* CombinStatVec( const std::vector<StateMent*> &stat_vec );
#endif
