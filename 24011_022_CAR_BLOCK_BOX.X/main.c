/*
 * File:   main.c
 */

#include "main.h"

#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)

static void init_config(void) {
   
    //initialize i2c
    init_i2c(100000);
    //initialize rtc
    init_ds1307();
    //initialize clcd
    init_clcd();
    //initialize dkp
    init_digital_keypad();
    //initialize adc
    init_adc();
    
    //initialize timer2
    init_timer2();
    
    GIE = 1; // enable global interrupt 
    PEIE = 1;  // peripheral interrupt enable for timer2.
    
}

void main(void) {
    //variable declaration
    unsigned char control_flag = DASH_BOARD_SCREEN, reset_flag, menu_pos;
    unsigned char speed = 0, key, delay = 0, long_press = 0;
    char event[3] = "ON";
    char* gear[] = {"GR", "GN", "G1", "G2", "G3", "G4"};
    unsigned char gr = 0;
    init_config();
    log_car_event(event,  speed);
    
    write_str_at24c04(0x00, "1001");
    while (1) {
        
        key = read_digital_keypad();
        
        
        
        if(key == SW1)
        {
            strcpy(event, "CO"); //accident or collision
            log_car_event(event, speed);
        }
        else if(key == SW2 && gr < 6)
        {
            strcpy(event, gear[gr]);
            gr++;
            log_car_event(event, speed);
        }
        else if(key == SW3 && gr > 0)
        {
            gr--;
            strcpy(event, gear[gr]);
            log_car_event(event, speed);
        }//
        else if((key == SW4 || key == SW5) && control_flag == DASH_BOARD_SCREEN )
        {
            control_flag = LOGIN_SCREEN;
            reset_flag = RESET_PASSWORD;
            clear_clcd_screen();
            clcd_print("Enter Password", LINE1(1));
            
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            TMR2ON = 1;     
        }
        else if(key == SW4_L_P  && control_flag == MAIN_MENU_SCREEN)
        {
            switch(menu_pos)
            {
                case 0:
                    control_flag = VIEW_LOG;
                    reset_flag = RESET_LOG_POS;
                    clear_clcd_screen();
                    TMR2ON = 1;
                    break;
                case 1:
                    control_flag = CLEAR_LOG;
                    clear_clcd_screen();
                    break;
                case 2:
                    control_flag = DOWNLOAD_LOG;
                    break;
                case 3:
                    control_flag = SET_TIME;
                    break;
                case 4:
                    control_flag = CHANGE_PASSWORD;
                    reset_flag = RESET_PASSWORD;
                    clear_clcd_screen();
                    TMR2ON = 1;
                    break;
                            
            }
        }
        
        
        
        
        
        
        speed = (unsigned char)(read_adc() / 10.25);//0-1023 -> 0-99
        
        switch(control_flag)
        {
            case DASH_BOARD_SCREEN:
                display_dashboard(event, speed);
                break;
            case LOGIN_SCREEN:
                switch(login(key, reset_flag))
                {
                    case RETURN_BACK:
                        control_flag = DASH_BOARD_SCREEN;
                        clear_clcd_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON = 0; 
                        break;
                        
                    case LOGIN_SUCCESS:
                        control_flag = MAIN_MENU_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag = RESET_MENU;
                        TMR2ON = 1; 
                        continue;
                }
                break;
            case MAIN_MENU_SCREEN:
                menu_pos = menu_screen(key, reset_flag);
                 if(menu_pos == RETURN_BACK)
                {
                    clear_clcd_screen();
                    control_flag = DASH_BOARD_SCREEN;
                    TMR2ON = 0; 
                }
                break;
                
            case VIEW_LOG:
                
                if(view_log(key, reset_flag) == RETURN_BACK)
                {
                    control_flag = DASH_BOARD_SCREEN;
                    clear_clcd_screen();   
                    TMR2ON = 0;
                }  
                break;
            case CLEAR_LOG:
                clear_log();
                clear_clcd_screen();
                control_flag = DASH_BOARD_SCREEN;
                break;
                
            case CHANGE_PASSWORD:
                
                switch(change_password(key, reset_flag))
                {
                    case RETURN_BACK:
                        clear_clcd_screen();
                        control_flag = DASH_BOARD_SCREEN;
                        TMR2ON = 0;
                        break;
                        
                    case TASK_SUCCESS:
                        clear_clcd_screen();
                        control_flag = MAIN_MENU_SCREEN;
                        reset_flag = RESET_MENU;
                        TMR2ON = 1;
                        continue;
                }
                break;
        }
        
        reset_flag = RESET_NOTHING;
        
    }
    return;
}
