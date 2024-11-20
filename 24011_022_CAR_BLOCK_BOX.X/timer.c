
#include "main.h"

void init_timer2(void)
{
    /*loading pre load register with 250*/
    PR2 = 250;
    
    /*timer2 interrupt Enable bit*/
    TMR2IE = 1;
    
    /*using pre scaler value as 1:16*/
    T2CKPS0 = 1;
    T2CKPS1 = 1;
    
    /*Timer2 in On mode*/
    TMR2ON = 0;
}