#include "type.hpp"
Type __void = Type(VOID,NOT_VEC);
Type __int = Type( INT,NOT_VEC );

Type __bool = Type( BOOL,NOT_VEC );
Type __dynvec_int8 = Type( INT8,NOT_VEC );

Type __int4 = Type( INT4,NOT_VEC );
Type __dynvec_int64 = Type( INT64,NOT_VEC );
Type __float = Type( FLOAT, NOT_VEC );
Type __double = Type( DOUBLE,  NOT_VEC );
Type __double_ptr = Type( &__double,NOT_VEC);
Type __int_ptr = Type( &__int,NOT_VEC);

Type __dynvec_int64_ptr = Type( &__dynvec_int64,NOT_VEC);

Type __int_ptr_ptr = Type( &__int_ptr,NOT_VEC);

Type __dynvec_int64_ptr_ptr = Type( &__dynvec_int64_ptr,NOT_VEC);
Type __double_ptr_ptr = Type( &__double_ptr,NOT_VEC);
Type __float_ptr = Type( &__float, NOT_VEC);
Type __dynvec_int8_ptr = Type( &__dynvec_int8,NOT_VEC);

Type __dynvec_int8_v4 = Type( INT8,VECTOR4 );
Type __double_ptr_v4 = Type( &__double, VECTOR4 );
Type __double_v4 = Type( DOUBLE,VECTOR4 );
Type __bool_v4 = Type( BOOL, VECTOR4 );
Type __int_v4 = Type( INT,VECTOR4 );
Type __double_v4_pointer = Type( &__double_v4, NOT_VEC );
Type __int_v4_pointer = Type(&__int_v4, NOT_VEC);

Type __dynvec_int64_v4 = Type( INT64,VECTOR4 );
Type __dynvec_int8_v4_ptr = Type(&__dynvec_int8_v4, NOT_VEC);
Type __dynvec_int64_v4_ptr = Type(&__dynvec_int64_v4, NOT_VEC);
Type __double_pointer_v4 = Type( &__double, VECTOR4 );

Type __float_v8 = Type( FLOAT,VECTOR8 );

Type __float_v4 = Type( FLOAT,VECTOR4 );
Type __double_v8 = Type( DOUBLE,VECTOR8 );
Type __bool_v8 = Type( BOOL, VECTOR8);
Type __int_v8 = Type( INT,VECTOR8 );

Type __dynvec_int64_v8 = Type( INT64,VECTOR8 );

Type __dynvec_int8_v8 = Type( INT8,VECTOR8 );
Type __double_v8_pointer = Type( &__double_v8, NOT_VEC );
Type __int_v8_ptr = Type(&__int_v8, NOT_VEC);

Type __dynvec_int8_v8_ptr = Type(&__dynvec_int8_v8, NOT_VEC);

Type __dynvec_int64_v8_ptr = Type(&__dynvec_int64_v8, NOT_VEC);

Type __double_ptr_v8 = Type( &__double, VECTOR8 );
Type __float_ptr_v8 = Type( &__float, VECTOR8 );

Type __float_ptr_v4 = Type( &__float, VECTOR4 );
Type __int_ptr_v8 = Type( &__int, VECTOR8 );

Type __int_ptr_v4 = Type( &__int, VECTOR4 );
Type __dynvec_int8_ptr_v8 = Type( &__dynvec_int8, VECTOR8 );

Type __dynvec_int8_ptr_v4 = Type( &__dynvec_int8, VECTOR4 );
Type __dynvec_int64_ptr_v8 = Type( &__dynvec_int64, VECTOR8 );

Type __int_v16 = Type(INT,VECTOR16);
Type __dynvec_int8_v16 = Type( INT8,VECTOR16 );
Type __bool_v16 = Type( BOOL, VECTOR16);
Type __float_v16 = Type(FLOAT,VECTOR16);
Type __float_v16_ptr = Type( &__float_v16, NOT_VEC );

Type __float_v4_ptr = Type( &__float_v4, NOT_VEC );
Type __float_v8_ptr = Type( &__float_v8, NOT_VEC );
Type __float_ptr_v16 = Type( &__float, VECTOR16 );

Type __int_ptr_v16 = Type( &__int, VECTOR16 );

Type __dynvec_int8_ptr_v16 = Type( &__dynvec_int8, VECTOR16 );
Type __int_v16_ptr = Type( &__int_v16, NOT_VEC );
Type __dynvec_int8_v16_ptr = Type( &__dynvec_int8_v16, NOT_VEC );

Type __float_v;
Type __float_v_ptr;
Type __double_ptr_v ;

Type __double_v;
Type __bool_v ;
Type __int_v ;
Type __dynvec_int64_v ;

Type __int_dv ;
Type __dynvec_int8_v;


Type __double_v_ptr;
Type __int_v_ptr;
Type __int_dv_ptr;
Type __float_ptr_v ;

Type __dynvec_int8_ptr_v ;
Type __dynvec_int8_v_ptr;

Type __dynvec_int64_v_ptr;
Type __int_ptr_v;

Type type_scalar_ptr2vector_ptr( const Type type ) {
    Type ret_type;
    if(type == __int_ptr) {
        ret_type = __int_v_ptr;
    } else if( type == __double_ptr ) {
        ret_type = __double_v_ptr;
    } else if( type == __float_ptr ) {
        ret_type = __float_v_ptr;
    } else {
        LOG(FATAL) << "Unsupported";
    }
    return ret_type;
}
Type type_scalar_ptr2ptr_vector( const Type type ) {
    Type ret_type;
    if(type == __int_ptr) {
        ret_type = __int_ptr_v;
    } else if( type == __double_ptr ) {
        ret_type = __double_ptr_v;
    } else if( type == __float_ptr ) {
        ret_type = __float_ptr_v;
    } else {
        LOG(FATAL) << "Unsupported";
    }
    return ret_type;
}
void Type::init_type(const int vector) {
    //LOG(INFO) << vector;
    if(vector == VECTOR8) {
         __double_v = __double_v8;
         __bool_v = __bool_v8;
         __int_v = __int_v8;
         __double_ptr_v = __double_ptr_v8;
         __int_dv = __int_v16;
         __dynvec_int8_v = __dynvec_int8_v8;

         __dynvec_int64_v = __dynvec_int64_v8;
         __double_v_ptr = __double_v8_pointer;
         __int_v_ptr = __int_v8_ptr;

         __int_dv_ptr = __int_v16_ptr;
         __dynvec_int8_v_ptr = __dynvec_int8_v8_ptr;

         __dynvec_int64_v_ptr = __dynvec_int64_v8_ptr;
         __float_ptr_v = __float_ptr_v8;

         __int_ptr_v = __int_ptr_v8;
         __dynvec_int8_ptr_v = __dynvec_int8_ptr_v8;
         __float_v_ptr = __float_v8_ptr;

          __float_v = __float_v8;
    } else if(vector == VECTOR16) {
        __float_v = __float_v16;
         __float_v_ptr = __float_v16_ptr; 
        __double_v = __double_v8;
        __bool_v = __bool_v16;
        __int_v = __int_v16;
        __dynvec_int8_v = __dynvec_int8_v16;

        __double_v_ptr = __double_v8_pointer;
        __int_v_ptr = __int_v16_ptr;
        __dynvec_int8_v_ptr = __dynvec_int8_v16_ptr;

        __float_ptr_v = __float_ptr_v16;

        __int_ptr_v = __int_ptr_v16;
        __dynvec_int8_ptr_v = __dynvec_int8_ptr_v16; 
    } else if(vector == VECTOR4) {
         __double_v = __double_v4;
         __bool_v = __bool_v4;
         __int_v = __int_v4;
         __double_ptr_v = __double_ptr_v4;
         __dynvec_int8_v = __dynvec_int8_v4;

         __dynvec_int64_v = __dynvec_int64_v4;
         __double_v_ptr = __double_v4_pointer;
         __int_v_ptr = __int_v4_pointer;

         __dynvec_int8_v_ptr = __dynvec_int8_v4_ptr;

         __dynvec_int64_v_ptr = __dynvec_int64_v4_ptr;


         __float_ptr_v = __float_ptr_v4;

         __int_ptr_v = __int_ptr_v4;
         __dynvec_int8_ptr_v = __dynvec_int8_ptr_v4;
         __float_v_ptr = __float_v4_ptr;

    } else {
        LOG(FATAL) << "Unsupported";
    }
}


