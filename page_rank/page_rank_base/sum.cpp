#include "Timers.hpp"
#include "csr_matrix.h"
#include "util.h"
#include "pagerank.h"
template<typename T>
void check_equal(const T * v1, const T * v2, const int num ) {
    bool flag = true;
    for( int i = 0 ; i < num ; i++ ) {
        if( (v1[i]-v2[i]) > 1e-6 || (v2[i]-v1[i]) > 1e-6 ) {
            flag = false;
            std::cout<< i<< " "  << v1[i]  << " "<< v2[i]<<"\n";
        }
    }
    if(flag) 
        std::cout<<"Correct"<<std::endl;
    else 
        std::cout<<"False"<<std::endl;
}

template<typename T>
void print_vec( T * data_ptr, const int num ) {
    for( int i = 0 ; i < num ; i++ ) {
        std::cout<< data_ptr[i] << " ";
    }
    std::cout<<std::endl;
}
//#define LITTEL_CASE2
int main( int argc , char const * argv[] ) {
//    int times = stoi(argv[2]);
    #if defined LITTEL_CASE2
        PageRankStructure page_rank_structure = little_test2();
        PageRankStructurePtr  page_rank_structure_ptr = & page_rank_structure;
    #else
        if(argc <= 1 ) {
            printf("Erro: You need to modify a file to read\n");
            return 0;
        }
        PageRankStructurePtr page_rank_structure_ptr = pagerank_read( argv[1]);
    #endif
   

    float * sum = page_rank_structure_ptr->sum;

    float * rank = page_rank_structure_ptr->rank;

    int * n1 = page_rank_structure_ptr->n1;

    int * n2 = page_rank_structure_ptr->n2;
    float * nneibor = page_rank_structure_ptr->nneibor;
    int nedges = page_rank_structure_ptr->nedges;
    int nnodes = page_rank_structure_ptr->nnodes;
    float * sum_bak = SIMPLE_MALLOC( float, nnodes );
    for( int i = 0 ; i < nnodes ; i++ )
        sum_bak[i] = sum[i];

//    print_vec( n1, nedges );

//    print_vec( n2, nedges );


    Timer::startTimer("aot");
    int times = 1000;
    for(int i = 0 ; i < times;i++) {
    //    func_base(sum_bak,n1,n2,rank,nneibor,nedges);
        for(int j=0;j<nedges;j++) {
            int nx = n1[j];
            int ny = n2[j];
            sum[ny] += rank[nx] / nneibor[nx];
        }

    }
    Timer::endTimer("aot");

    Timer::printTimer("aot",1000);

    Timer::printGFLOPS("aot",2*nedges,1000);
//    printf("%f",sum_bak[1]);
}
