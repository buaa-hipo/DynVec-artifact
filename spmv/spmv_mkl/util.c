#include "util.h"
#include <stdio.h>
#include <math.h>
#define TARGET(a,b) ((a) >= (b))
#define TARGET_LESS(a,b) ((a) < (b))
int binarySearchLower(const int *arr, int start, int end,const int target) {//supremum
    int mid;
    if(start == end) {

        return start ;
    }
    if(  TARGET(arr[start],target)) {
        mid = start ;
    }
    else {
            start++;
            mid = (start  + end) / 2;
            while(start < end ) {
          //      if (arr[mid] == target && arr[mid] != target) {
                if( TARGET_LESS(arr[mid],target) ){

                    start = mid + 1;

                }
                else {
                    if(TARGET(arr[mid] , target) && TARGET_LESS(arr[mid-1],target))
                    {
                        break;
                    }
                    else {
                        end = mid - 1;
                    }

                }
                mid = (end + start) / 2;
            }

    }
    return mid;
}


void set_double (double * array , double data, const int size) {
    int i;
    for(  i = 0 ; i < size ; i++) {
        array[i] = data;
    }
}
#define ERRO 1e-12
double varience_double(const double * array1 , const double * array2,const int array_size) {
    double varience = 0 ;
    int i ;
    for ( i = 0; i < array_size; ++i) {
        if( fabs( array1[i]-array2[i]) > ERRO ) {
            printf("%d %lf %lf",i,array1[i],array2[i]);
            varience += (array1[i] - array2[i]) * (array1[i] - array2[i]);
        }
    }
    return varience;
}
double sum_double(const double * array,const int size) {
    double sum  = 0 ;
    int i ;
    for( i = 0 ; i < size ; i++) {
        sum += array[i];
    }
    return sum;
}
int sum_int( const int * array , const int size ) {
    int sum = 0 ;
    int i  ;
    for (i = 0; i < size; ++i) {
        sum += array[i];
    }
    return sum;
}

inline void print_vector_double(const double * a, const  int len) {
    int i;
    for( i = 0 ; i < len ; i++) {
        printf("%lf ",a[i]);
    }
    printf("\n");
}

inline void print_vector_int(const int * a,const int len) {
    int i;
    for( i = 0 ; i < len ; i++) {
        printf("%d ",a[i]);
    }
    printf("\n");

}
void write_vector_int_to_File( const char * filename ,const  double * data , const int size ) {

    FILE * file = fopen(filename , "w");
    int i;
    for ( i = 0; i < size; ++i) {
        fprintf(file, "%f\n", data[i] );
    }
    fclose(file);
}

