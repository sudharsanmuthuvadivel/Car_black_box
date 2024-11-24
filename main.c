/*
 * Name : Sudharsan M
 * Date : 20.11.2024
 * Project : Implement Car Black Box.
 */

#include "main.h"

#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT disabled)

/*Function to initiate all required peripheral configuration*/
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
    //initialize UART
    init_uart(9600);
    //initialize timer2
    init_timer2();
    
    GIE = 1; // enable global interrupt 
    PEIE = 1;  // peripheral interrupt enable for timer2.
    
}
/*Main Function*/
void main(void) {
    //variable declaration
    unsigned char control_flag = DASH_BOARD_SCREEN, reset_flag, menu_pos;
    unsigned char speed = 0, key, delay = 0, long_press = 0;
    char event[3] = "ON";
    char* gear[] = {"GR", "GN", "G1", "G2", "G3", "G4"};
    unsigned char gr = 0;
    init_config();
    
    log_car_event(event,  speed); // initially Car should be On and Speed is 0
    
    write_str_at24c04(0x00, "1001");  //store Password in External EEPROM
    
    while (1) {                     //super loop
        
        key = read_digital_keypad(); // function call to read which key pressed
         
        if(key == SW1)              //check if key is SW1, event is accident or collision
        {
            strcpy(event, "CO");     
            log_car_event(event, speed);
        }
        else if(key == SW2 && gr < 6)   //SW2 for Gear increasing
        {
            strcpy(event, gear[gr]);
            gr++;
            log_car_event(event, speed);
        }
        else if(key == SW3 && gr > 0)  // SW3 for Gear decreasing
        {
            gr--;
            strcpy(event, gear[gr]);
            log_car_event(event, speed);
        }
        /*check if key is sw4 or sw5 , read password from user to enter main menu screen*/
        else if((key == SW4 || key == SW5) && control_flag == DASH_BOARD_SCREEN )
        {
            control_flag = LOGIN_SCREEN;
            reset_flag = RESET_PASSWORD;
            clear_clcd_screen();                    //function call to clear the clcd screen
            clcd_print("Enter Password", LINE1(1));
            
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE); // function call to blink the cursor in the clcd
            __delay_us(100);
            TMR2ON = 1;                                 //timer2 ON
        }
        /*SW4 long press in main menu screen to select various operation based on menu position*/
        else if(key == SW4_L_P  && control_flag == MAIN_MENU_SCREEN)
        {
            switch(menu_pos)
            {
                case 0:                                 //view log
                    clear_clcd_screen();
                    log_car_event("VL", speed);
                    control_flag = VIEW_LOG;        //chenage control flag to change the different screen
                    reset_flag = RESET_LOG_POS;
                    
                    TMR2ON = 1;
                    break;
                case 1:
                    control_flag = CLEAR_LOG;       //clear log
                    clear_clcd_screen();
                    break;
                case 2:                             //download the logs
                    clear_clcd_screen();
                    log_car_event("DL", speed);
                    clcd_print("Open", LINE1(5));
                    clcd_print("Cutecom", LINE2(4));
                    control_flag = DOWNLOAD_LOG;
                    break;
                case 3:                           //reset time in RTC
                    clear_clcd_screen();
                    log_car_event("ST", speed);
                    control_flag = SET_TIME;
                    reset_flag = RESET_TIME;
                    TMR2ON = 1;
                    break;
                case 4:                         //change password
                    control_flag = CHANGE_PASSWORD;
                    reset_flag = RESET_PASSWORD;
                    clear_clcd_screen();
                    TMR2ON = 1;
                    break;
                            
            }
        }
        else if (key == SW4_L_P && control_flag == VIEW_LOG)
        {
            clear_clcd_screen();
            control_flag = MAIN_MENU_SCREEN;
            reset_flag = RESET_MENU;
            TMR2ON = 1; 
        }
        else if(key == SW4_L_P && control_flag == CHANGE_PASSWORD)
        {
            clear_clcd_screen();
            control_flag = MAIN_MENU_SCREEN;
            reset_flag = RESET_MENU;
            TMR2ON = 1; 
        }
        else if(key == SW5_L_P && control_flag == MAIN_MENU_SCREEN)
        {
            clear_clcd_screen();
            control_flag = DASH_BOARD_SCREEN;
            TMR2ON = 0;
        }
        
        /*measure speed form pot by using ADC*/
        speed = (unsigned char)(read_adc() / 10.25);//0-1023 -> 0-99 
        
        
  /*================ Switch case for various operations =======================*/
        
        switch(control_flag)
        {
            case DASH_BOARD_SCREEN:
                display_dashboard(event, speed);   //Function call for Dash board screen
                break;
                
            case LOGIN_SCREEN:                     
                switch(login(key, reset_flag))    //function call for Login screen
                {
                    case RETURN_BACK:             //No action detected upto 5sec , screen change to Dash board screen
                        control_flag = DASH_BOARD_SCREEN;
                        clear_clcd_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);   //instruction to Off the cursor
                        __delay_us(100);
                        TMR2ON = 0;                                     //timer2 OFF
                        break;
                        
                    case LOGIN_SUCCESS:                     //if correct password change to Main menu screen
                        control_flag = MAIN_MENU_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag = RESET_MENU;            //for first time reset menu position
                        TMR2ON = 1;                          //timer2 ON
                        continue;
                }
                break;
            case MAIN_MENU_SCREEN:
                menu_pos = menu_screen(key, reset_flag);       //function call for menu screen
                 if(menu_pos == RETURN_BACK)            //if no action detected upto 5sec screen change to dash board screen
                {
                    clear_clcd_screen();
                    control_flag = DASH_BOARD_SCREEN;
                    TMR2ON = 0;                         //timer2 OFF
                }
                break;
                
            case VIEW_LOG:
                
                if(view_log(key, reset_flag) == RETURN_BACK)   //function call for view all entered logs
                {
                    control_flag = DASH_BOARD_SCREEN;         //if no action detected upto 5sec screen change to dash board screen
                    clear_clcd_screen();   
                    TMR2ON = 0;
                }  
                break;
            case CLEAR_LOG:
                clear_log();                            //function call to clear all entered logs
                clear_clcd_screen();                        //clear the clcd screen
                control_flag = DASH_BOARD_SCREEN;        //move to dash board screen
                break;
                
            case DOWNLOAD_LOG:              
                download_log();                     //function call for download all entered logs to system
                __delay_ms(2000);                    //wait 2sec
                control_flag = MAIN_MENU_SCREEN;    //change to main menu screen
                reset_flag = RESET_MENU;
                TMR2ON = 1;                         //timer2 ON
                continue;
                
            case SET_TIME:
                switch(change_time(key, reset_flag))  //function call for change time in RTC
                {
                    case TASK_SUCCESS:              
                        control_flag = MAIN_MENU_SCREEN;      //after change the time move to main menu screen
                        reset_flag = RESET_MENU;
                        clear_clcd_screen();
                        TMR2ON = 1;
                        continue;
                    case RETURN_BACK:                       //if no action detected move to dash board screen
                        clear_clcd_screen();
                        control_flag = DASH_BOARD_SCREEN;
                        TMR2ON = 0;
                        break;
                }
                break;
                
            case CHANGE_PASSWORD:
                
                switch(change_password(key, reset_flag))        //function call for change password 
                {
                    case RETURN_BACK:
                        clear_clcd_screen();
                        control_flag = DASH_BOARD_SCREEN;   // change to dash board screen
                        TMR2ON = 0;
                        break;
                        
                    case TASK_SUCCESS:             //if password successfully stored , move to menu screen
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
