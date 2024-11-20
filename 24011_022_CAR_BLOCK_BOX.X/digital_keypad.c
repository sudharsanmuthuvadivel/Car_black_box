/*
 * File:   output_led_pattern.c
 * Author: Sudharsan M
 *
 * Created on October 21, 2024, 6:59 AM
 */
#include <xc.h>
#include "digital_keypad.h"

void init_digital_keypad(void)
{
    /* Set Keypad Port as input */
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | INPUT_LINES;
}

unsigned char read_digital_keypad()
{
    static unsigned char once, pre_key, long_press;
    unsigned char  key = KEYPAD_PORT & INPUT_LINES;
    
    if(key != ALL_RELEASED && once == 0)
    {
        once = 1;
        pre_key = key;
        long_press = 0;
    }
    else if(key == ALL_RELEASED && once == 1)
    {
        once = 0;
        if(long_press < 15)
            return pre_key;
    }
    
    /*Longic to Find Switch is long pressed*/
    if(once == 1 && long_press < 16)
        long_press++;
    
    else if(once == 1 && long_press == 16 && key == SW4)
    {
        long_press++;
        return SW4_L_P;
    }
    else if(once == 1 && long_press == 16 && key == SW5)
    {
        long_press++;
        return SW5_L_P;
    }
    
    return ALL_RELEASED;
}
