#ifndef TYPE_HPP
#define TYPE_HPP
#include "configure.hpp"
typedef enum {FLOAT = 0, DOUBLE = 1, INT = 2, BOOL=3,INT8=4,PTR=5,INT64=6,VOID=7,INT4=8} DataType;
#define NOT_VEC (1)
#include <sstream>
#include <string>
#include <sstream>
#include "log.h"
class Type {
    DataType data_type_;
    int lanes_;
    Type * pointer2type_;
    public:

    static void init_type(const int vector);
    Type( Type * pointer2type, int lanes ) : data_type_(PTR),lanes_(lanes),pointer2type_(pointer2type) {
    }
    Type(DataType data_type,  int lanes ) : data_type_(data_type),lanes_(lanes){ 
        pointer2type_ = nullptr;
    }
    Type() {
        pointer2type_ = nullptr;
    }
    DataType get_data_type()const {
        return data_type_;
    }
    void set_lanes(const int lanes) {

        lanes_ = lanes;
    }

    int get_lanes()const {

        return lanes_;
    }
    Type * get_pointer2type()const {
        return pointer2type_;
    }
    std::string str()const {
        
        std::stringstream ss ;
        std::string pointer = std::string();
        if( pointer2type_ == nullptr ) {
        switch( data_type_) {
            case FLOAT:
                ss << "float";
                break;
            case DOUBLE:
                ss << "double";
                break;
            case INT :
                ss << "int";
                break;
            case INT8:
                ss << "int8";
                break;
            case INT64:
                ss << "int64";
                break;
            case VOID:
                ss << "void";
                break;
            case BOOL:
                ss << "bool";
                break;
            default:
                LOG(FATAL) << "Undef Typeid" << data_type_;
        }
        ss << " ";

        if(lanes_ != 1) {
            ss << " v" << lanes_;
        }
        ss << " ";
        } else {
          pointer = pointer2type_->str() + "*";
          ss << pointer;
          if(lanes_ != 1) {
            ss  << " v" << lanes_;
          }

        } 
        return ss.str();
    }
    inline bool operator==( const Type & t1 ) const {
        if( this->get_pointer2type() != nullptr && t1.get_pointer2type() != nullptr ) {
            return ( this->lanes_ == t1.get_lanes())&&(*(this->get_pointer2type())) == (*(t1.get_pointer2type())) ;
        }
        if( this->get_pointer2type() != nullptr || t1.get_pointer2type() != nullptr ) {
            return false;
        } else {
        if( this->lanes_ == t1.lanes_ &&
            this->data_type_ == t1.data_type_) {
            return true;
        } else {
            return false;
        }
        }
    }
    friend std::ostream& operator<< ( std::ostream & stream, const Type& type )  {
        stream << type.str();
        return stream;
    }
    bool is_pointer() const {
        return pointer2type_ != nullptr;
    }
};

extern Type __void ;
extern Type __int ;

extern Type __bool ;
extern Type __float ;
extern Type __double;
extern Type __dynvec_int8;

extern Type __int4;
extern Type __dynvec_int64;
extern Type __double_ptr;
extern Type __int_ptr ;
extern Type __dynvec_int64_ptr;

extern Type __double_ptr_ptr ;
extern Type __int_ptr_ptr ;
extern Type __dynvec_int64_ptr_ptr;
extern Type __float_ptr ;
extern Type __dynvec_int8_ptr;


extern Type __float_v;
extern Type __float_v_ptr;
extern Type __double_ptr_v ;

extern Type __double_v;
extern Type __bool_v ;
extern Type __int_v ;
extern Type __dynvec_int64_v ;

extern Type __int_dv ;
extern Type __dynvec_int8_v;


extern Type __double_v_ptr;
extern Type __int_v_ptr;
extern Type __int_dv_ptr;
extern Type __float_ptr_v ;
extern Type __dynvec_int8_v_ptr;

extern Type __dynvec_int64_v_ptr;
extern Type __int_ptr_v;

Type type_scalar_ptr2vector_ptr( const Type type ) ;
Type type_scalar_ptr2ptr_vector( const Type type );

#endif

