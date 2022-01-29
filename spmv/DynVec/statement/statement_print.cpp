#include "statement_print.hpp"
class StateMentPrint{
    private:
int space_num_  ;
const int space_inc_num = 4;
void IncSpace() {
//    space_num_ += space_inc_num;
}
void DecSpace() {
//    space_num_ -= space_inc_num;
}
void PrintSpace(std::ostream &os) {
//    for( int  i = 0 ; i < space_num_ ; i++) {
//        os << " ";
//    }
}

void print_(StateMent * stat,std::ostream&os) ;

void print_(Block * stat,std::ostream&os) ;

void print_(For * stat,std::ostream&os) ;

void print_(Varience * stat , std::ostream&os) ;

void print_(Const * stat, std::ostream&os) ;

void print_(LetStat * stat,std::ostream&os) ;

void print_( IncAddr * stat ,std::ostream&os) ;

void print_(Nop * stat,std::ostream&os ) ;

void print_(Scatter * stat, std::ostream&os) ;


void print_(Gather * stat, std::ostream&os ) ;


void print_(Load * stat, std::ostream&os ) ;

void print_( Print * stat, std::ostream & os ) ;

void print_(Store * stat, std::ostream&os ) ;

void print_(Shuffle * stat, std::ostream&os ) ;

void print_(Select * stat, std::ostream&os ) ;

void print_(ICmpEQ * stat, std::ostream&os ) ;
void print_(Reduce * stat, std::ostream&os ) ;

void print_(BroadCast * stat, std::ostream&os ) ;

void print_(BitCast * stat, std::ostream&os ) ;

void print_(Binary * stat, std::ostream&os ) ;

void print_(Add * stat, std::ostream&os );

void print_(Div * stat, std::ostream&os ) ;
void print_(Mul * stat, std::ostream&os ) ;

void print_(Minus * stat, std::ostream&os ) ;

void print_(DetectConflict * stat, std::ostream&os ) ;
void print_(ComplexReduce * stat, std::ostream&os ) ;

void print_( ExtractElement * stat, std::ostream&os ) ;
void print_( InsertElement * stat, std::ostream&os ) ;
public:
StateMentPrint(){
    space_num_ = 0;
}


void print(StateMent * stat,std::ostream&os) ;
};
#define SET_DISPATCH(CLASS_NAME) SET_DISPATCH_TEMPLATE( ftype_ptr,CLASS_NAME,StateMent,print_ )
void StateMentPrint::print_(StateMent * stat,std::ostream&os) {
    printf("\nthe statement %s does not support\n", stat->get_class_name().c_str());
}
void StateMentPrint::print_(Block * stat,std::ostream&os) {

    const std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    for(unsigned int i = 0 ; i < state_vec->size() ; i++) {
        print((*state_vec)[i],os);
    }
}
void StateMentPrint::print_(For * stat,std::ostream&os) {
    PrintSpace(os);
    os << "for ( ";
    print(stat->get_var(),os);
    os << " = ";
    print(stat->get_begin(),os);
    os << " , ";
    print(stat->get_end(),os);
    os << " , ";
    print(stat->get_space(),os);
    os << " ) { \n";
    IncSpace();

    print(stat->get_stat(),os);
    DecSpace();
    PrintSpace(os);
    os << "}\n";

}
void StateMentPrint::print_(Varience * stat , std::ostream&os) {
    os << stat->get_type_str() << " " << stat->get_name(); 
}
void StateMentPrint::print_(Const * stat, std::ostream&os) {
    os << stat->get_type_str() << "(" << stat->get_data_str() <<")";
}
void StateMentPrint::print_(LetStat * stat,std::ostream&os) {

    PrintSpace(os);
    Varience * res_var = stat->get_res();
    if( res_var->get_is_const()) {
        os << "const ";
    }
    print_(stat->get_res(),os);
    os << " = ";
    print(stat->get_expr(),os);
    os << "\n";
}
void StateMentPrint::print_( IncAddr * stat ,std::ostream&os) {
    print(stat->get_addr(),os) ;
    os << " + ";
    print(stat->get_inc(),os);
}
void StateMentPrint::print_(Nop * stat,std::ostream&os ) {
    os << "nop\n";
}
void StateMentPrint::print_(Scatter * stat, std::ostream&os) {
    PrintSpace(os);
    os << "scatter ";
    print(stat->get_addr(),os);
    os << "(";
    print(stat->get_index(),os );
    StateMent * mask = stat->get_mask(); 
    
    if( mask != NULL ) {
        os << ",";
        print(mask,os );
    }
    os << ")";
    os << " <- ";
    print(stat->get_data(),os);

    os << "addr: " << stat->addr_name_ << " index: " << stat->index_name_ << " node: " << stat->node_name_;
    os << "\n";

}
void StateMentPrint::print_(Gather * stat, std::ostream&os ) {
    os << "gather ";
    print(stat->get_addr(),os);
    os << "(";
    print(stat->get_index(),os );
    StateMent * mask = stat->get_mask(); 
    
    if( mask != NULL ) {
        os << ",";
        print(mask,os );
    }

    os << ")";

    os << "addr: " << stat->addr_name_ << " index: " << stat->index_name_ << " node: " << stat->node_name_;
}
void StateMentPrint::print_(Load * stat, std::ostream&os ) {
    os << "load ";
    os << "(";
    print(stat->get_addr(),os);
    os << "\t";
    StateMent * mask_stat = stat->get_mask();
    if(mask_stat != NULL) {
        os << " , mask: ";
        print( mask_stat,os);
    }

    os << ")";
    os << "addr: " << stat->addr_name_ << " index: " << stat->index_name_ << " node: " << stat->node_name_;
}
void StateMentPrint::print_( Print * stat, std::ostream & os ) {
    os << "print ";
    print(stat->get_var(),os);
    os << "\n";
}
void StateMentPrint::print_( ExtractElement * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print(stat->get_from(),os);
    os << " [ ";
    print(stat->get_index(),os);
    os << "]";
}
void StateMentPrint::print_( InsertElement * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print( stat->get_to(),os );
    os << "[";
    print(stat->get_index(),os);
    os << "] <- ";
    print(stat->get_from(),os);
    os << "\n";
}

void StateMentPrint::print_(Store * stat, std::ostream&os ) {
    PrintSpace(os);
    os << stat->get_class_name() << " ";
    print(stat->get_addr(),os);
    os << " <- ";
    print(stat->get_data(),os);
    StateMent * mask_stat = stat->get_mask();
    if(mask_stat != NULL) {
        os << " mask: ";
        print( mask_stat,os);
    }
    os << "\n";
}
void StateMentPrint::print_(Select * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << " , ";
    print(stat->get_v2(),os);
    os << " , ";
    print(stat->get_index(),os);
    os << ")\n";
}

void StateMentPrint::print_(Shuffle * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << " , ";
    if(stat->get_v2() != NULL) {
        print(stat->get_v2(),os);
        os << " , ";
    }
    print(stat->get_index(),os);
    os << ")\n";
}
void StateMentPrint::print_(ComplexReduce * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os); 
    os << ",";
    print(stat->get_index(),os);
    os << ",";
    os << stat->get_mask();
    os << ")\n";
}
void StateMentPrint::print_(DetectConflict * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_index(),os);
    os << ")\n";
}


void StateMentPrint::print_(Reduce * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << ")\n";
}
void StateMentPrint::print_(BroadCast * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << ")\n";
}

void StateMentPrint::print_(BitCast * stat, std::ostream&os ) {

    os << stat->get_class_name() << " (";
    os << stat->get_type_str();
    os << ")";
    print( stat->get_v1(),os);
}
    void StateMentPrint::print_(ICmpEQ * stat,std::ostream& os ) {
        print(stat->get_v1(),os); 
        os <<  " == " ;
        print(stat->get_v2(),os);
    }

#define PRINT_BINARY( CLASSNAME ,OP) \
    void StateMentPrint::print_(CLASSNAME * stat,std::ostream& os ) {\
        print(stat->get_v1(),os); \
        os << " " OP " " ;\
        print(stat->get_v2(),os);\
        os << "addr: " << stat->addr_name_ << " index: " << stat->index_name_ << " node: " << stat->node_name_;\
    }

PRINT_BINARY( Binary,"op" )
PRINT_BINARY( Add, "+");

PRINT_BINARY( Div, "/");
PRINT_BINARY( Mul, "*");

PRINT_BINARY( Minus, "-");
void StateMentPrint::print(StateMent * stat,std::ostream&os) {

    if(stat == NULL) return;
    using FType = ir_func<void(StateMent*,std::ostream&)>; 
    static FType  * ftype_ptr = nullptr;
    if(ftype_ptr == nullptr) {
        ftype_ptr = new FType();
        SET_DISPATCH( StateMent );
        SET_DISPATCH( Block );
        SET_DISPATCH( For );
        SET_DISPATCH(Varience);

        SET_DISPATCH( Const );

        SET_DISPATCH( LetStat );
        SET_DISPATCH(IncAddr);
        SET_DISPATCH(Nop);
        SET_DISPATCH(Scatter);
        SET_DISPATCH(Gather);
        SET_DISPATCH( Load );
        SET_DISPATCH(Store);
        SET_DISPATCH(Shuffle);
        SET_DISPATCH( Reduce );
        SET_DISPATCH(BitCast);
        SET_DISPATCH(Binary);
        SET_DISPATCH(Add);
        SET_DISPATCH(Mul);

        SET_DISPATCH(Div);
        SET_DISPATCH(Minus);
        SET_DISPATCH(Print);
        SET_DISPATCH(BroadCast);

        SET_DISPATCH(ComplexReduce);
        SET_DISPATCH(DetectConflict  );

        SET_DISPATCH(ExtractElement );
        SET_DISPATCH(InsertElement );

        SET_DISPATCH( Select );

        SET_DISPATCH( ICmpEQ );
    }

    (*ftype_ptr)(stat,os);

}
std::ostream& operator << (std::ostream &stream ,  StateMent*statement ) {
    StateMentPrint * statement_print = new StateMentPrint();
    statement_print->print( statement,stream );
    delete statement_print;
    return stream;
}


#undef SET_DISPATCH
