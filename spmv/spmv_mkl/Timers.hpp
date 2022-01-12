/*---------------------------------------------------------------------------*\
Class: Timer
Author: Changxi.Liu
Copyright
-------------------------------------------------------------------------------
//to using Timer , you should call openTimer function first to wake up Timer count
\*---------------------------------------------------------------------------*/

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
//#include <iostream.h>
#include <map>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include<iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;

class Timer
{
    public:    //get time
//        inline double getSystemTime();
        static int _OpenTimer;
        static map<string,double> timeSum ;
        static map<string,double> timeStart;
//        static map<string,double> timeEnd;
        static map<string,unsigned long> timeSum_asm ;
        static map<string,unsigned long> timeStart_asm;
//        static map<string,unsigned long> timeEnd_asm;

        static map<string,int> count;
        void static startTimer(string in) ;
        static void endTimer(string in);
        ~Timer();
        static void printTimer();
        static void printTimer(string in) ;

        static void printTimer(string in,int times) ;
        void static startTimer_asm(string in) ;
        static void endTimer_asm(string in);
        static void printTimer_asm();
        static void openTimer();
        static void closeTimer();
        static void printTimer_asm(string in) ;
        static void printGFLOPS( string in , const long long int calculate_num ) ;
        

};

//extern map<string,double> Timer::timeSum ;
//extern map<string,double> Timer::timeStart;
//extern map<string,double> Timer::timeEnd;
//extern map<string,int> Timer::count;

#endif

