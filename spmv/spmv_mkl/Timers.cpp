#include "Timers.hpp"
#include <iostream>
//#define COUT cout
#define COUT cout
//using namespace std;

    map<string,double> Timer::timeSum ;
    map<string,double> Timer::timeStart;
//    map<string,double> Timer::timeEnd;
    map<string,int> Timer::count;
    map<string,unsigned long> Timer::timeSum_asm ;
    map<string,unsigned long> Timer::timeStart_asm;
//    map<string,unsigned long> Timer::timeEnd_asm;
    int Timer::_OpenTimer = 1;
    inline double getSystemTime()
    {

            struct timeval timer;
            gettimeofday(&timer, 0);
            return ((double)(timer.tv_sec) * 1e3 + (double)(timer.tv_usec)*1.0e-3);

    }


    inline unsigned long rpcc()
    {
            unsigned long time=0;
//            asm("rtc %0": "=r" (time) : );
            return time;
    }
    void Timer::startTimer(string in) {
            if(_OpenTimer) {
                timeSum[in] = 0 ;
                timeStart[in] = getSystemTime();
            }
    }
    void Timer::endTimer(string in) {
            if(_OpenTimer) {
                double timeEnd_in = getSystemTime();
                map<string,double>::iterator it = timeSum.find(in);
                if(it==timeSum.end()) {
                    timeSum[in] = 0 ;
                    count[in] = 0 ;
                }
                timeSum[in] += timeEnd_in - timeStart[in] ;
                count[in]++;
            }
    }
    void Timer::startTimer_asm(string in) {
            if(_OpenTimer) {
                timeStart_asm[in] = rpcc();
            }
    }
    void Timer::endTimer_asm(string in) {
            if(_OpenTimer) {
                unsigned long timeEnd_asm_in = rpcc();
                map<string,unsigned long>::iterator it = timeSum_asm.find(in);
                if(it==timeSum_asm.end()) {
                    timeSum_asm[in] = 0 ;
                    count[in] = 0 ;
                }
                timeSum_asm[in] += timeEnd_asm_in - timeStart_asm[in] ;
                count[in]++;
            }
    }
    void Timer::printTimer(string in,int times) {
        if(_OpenTimer) {
             map<string,double>::iterator it = timeSum.find(in);

            if(it==timeSum.end()) {
                std::cout<<"there is no Timer "<<in<<std::endl;
            }
            else{
                std::cout<< it->first <<" Time: "<<it->second / times
                    <<" count: "<<count[it->first]<<std::endl;
            }
        }
    }

    void Timer::printTimer(string in) {
        if(_OpenTimer) {
             map<string,double>::iterator it = timeSum.find(in);

            if(it==timeSum.end()) {
                std::cout<<"there is no Timer "<<in<<std::endl;
            }
            else{
                std::cout<< it->first <<" Time: "<<it->second
                    <<" count: "<<count[it->first]<<std::endl;
            }
        }
    }
    void Timer::printGFLOPS( string in , const long long int calculate_num ) {
         if(_OpenTimer) {
             map<string,double>::iterator it = timeSum.find(in);

            if(it==timeSum.end()) {
                std::cout<<"there is no Timer "<<in<<std::endl;
            }
            else{
                std::cout<< it->first <<" GFLOPS: "<<calculate_num / (it->second * 1e6 * count[it->first])
                    <<" count: "<<count[it->first]<<std::endl;
            }
        }
    }
    void Timer::printTimer_asm(string in) {
        if(_OpenTimer) {
             map<string,unsigned long>::iterator it = timeSum_asm.find(in);

            if(it==timeSum_asm.end()) {
                std::cout<<"there is no Timer "<<in<<std::endl;
            }
            else{
                string tmp =  it->first;
                unsigned long tmp_int = it->second;
                std::cout<< tmp <<" Time: "<< tmp_int <<" count: "<<count[tmp]<<std::endl;
            }
        }
    }


    void Timer::printTimer_asm() {
            if(_OpenTimer) {
                map<string,unsigned long>::iterator it;
                for(it=timeSum_asm.begin();it!=timeSum_asm.end();++it);
                    std::cout<< it->first <<" Time: "<<(it->second)<<" count: "
                        <<count[it->first]<<std::endl;
            }
    }

    void Timer::openTimer() {
        _OpenTimer = 1;
    }
    void Timer::closeTimer() {
        _OpenTimer = 0;
    }
    Timer::~Timer() {
            /*map<string,double>::iterator it;
            cout<<"Timer ~~~"<<endl;


            for(it=timeSum.begin();it!=timeSum.end();++it)
                cout<< it->first <<" Time: "<<it->second
                    <<" count: "<<count[it->first]<<endl;*/

    }

