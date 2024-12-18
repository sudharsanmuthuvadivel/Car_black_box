
#include<xc.h>
#include "main.h"

extern unsigned char sec, return_time;

void __interrupt() isr(void)
{
    static unsigned int count = 0;
    
    if(TMR2IF)         
    {
        //1:16 prescale
        if(++count == 1250) //1sec
        {
            if(sec != 0)
                sec--;
            else if(return_time != 0)
                return_time--;
            count = 0;
            
        }
        TMR2IF = 0;
    }
}