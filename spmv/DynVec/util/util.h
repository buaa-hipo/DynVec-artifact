#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include "mm_malloc.h"


#define SCALAR double
#define COLUMN_TYPE unsigned short int
//#define COLUMN_TYPE int
#define SLAVE_FUNC(funcname) slave_##funcname
#define ROUNDING_UP(num1,num2) (((num1)-1)/(num2)+1)
#define DIV(a,b) ((a)/(b))
#define ROUNDING_UP_4(num1) ( ((((num1)-1)>>2)+1) <<2 )
#define ROUNDING_UP_2(num1) ( (num1)+(num1)&0x1 )
#define DIV4UP(num1)  ((((num1)-1)>>2)+1)
int binarySearchLower(const int *arr, const int start,const int end,const int target) ;
double sum_double(const double * array,const int size) ;
double varience_double(const double * array1 , const double * array2,const int array_size) ;
void set_double (double * array , double data, const int size) ;

#define ANONYMOUSLIB_X86_CACHELINE 64
#define ROUNDING_UP_256( addr) ( ((((addr) - 1)>>5)+1)<<5  )
//#define ASSINED_SIZE(size) ( ((((size)-1)>>5)+1) <<5 )
//#define SIMPLE_MALLOC(type,size) ((type*)malloc(  ASSINED_SIZE(sizeof(type)*(size))) )
#define SIMPLE_MALLOC(type,size) ((type*)_mm_malloc(  ROUNDING_UP_256( sizeof(type)*(size) ) , ANONYMOUSLIB_X86_CACHELINE ))
int sum_int( const int * array , const int size );
#define MY_PRINT 1

#define my_printf_int_if(str,if_case) do {                          \
                                if((if_case&&MY_PRINT)) {                       \
                                     printf(#str" :%d \n",str);     \
                                }                                   \
                            } while(0)

#define my_print_vector_int_if(a1,a2,if_case) do {         \
                                if(if_case&&MY_PRINT)                 \
                                printf(#a1"\n") ;print_vector_int(a1,a2);       \
                                } while(0)


#define my_printf_str(str) do {                                     \
                                if(MY_PRINT) {                      \
                                printf("\n"#str"\n");                        \
                                fflush(stdout);                     \
                                }                                   \
                            } while(0)


#define my_printf_int(str) do { \
                            if(MY_PRINT)    \
                            printf(#str" :%d \n",str); \
                            } while(0)
#define my_printf_double(str) do { \
                                if(MY_PRINT) {\
                                printf(#str" :%f \n",str); \
                                }   \
                                }while(0)
#define my_print_vector_double(a1,a2) do {  \
                                    if(MY_PRINT)                        \
                                    printf(#a1"\n") ;print_vector_double(a1,a2);  \
                                    } while(0)
#define my_print_vector_int(a1,a2) do {         \
                                if(MY_PRINT) {  \
                                printf(#a1"\n") ;print_vector_int(a1,a2);      \
                                }                                               \
                                } while(0)
#define my_print_vector_xchar(a1,a2) do {         \
                                if(MY_PRINT) {  \
                                printf(#a1"\n") ;print_vector_xchar(a1,a2);      \
                                }                                               \
                                } while(0)

void print_vector_double(const double * const a, const int len);
void print_vector_int(const int * a, const int len) ;

void print_vector_xchar(const unsigned char * a, const int len) ;
void write_vector_int_to_File( const char * filename ,const  double * data , const int size );
#endif
