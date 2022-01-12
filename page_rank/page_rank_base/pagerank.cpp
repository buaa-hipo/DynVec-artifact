#include "pagerank.h"
void func_base( float * sum,const int * n1,const int * n2,const float * rank,const int * nneibor,const int nedges  ) {
    for(int j=0;j<nedges;j++) {
        int nx = n1[j];
        int ny = n2[j];
        sum[ny] += rank[nx] / nneibor[nx];
    }
}

