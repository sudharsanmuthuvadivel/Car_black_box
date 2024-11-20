#include "main.h"
unsigned char clock_reg[3], return_time, sec;
char time[7];  //HHMMSS
char log[11]; //HHMMSSEVSP
char log_count = 0;
char *menu[] = { "View log", "Clear log", "Download log", "Set time", "Change passwrd"};

void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
    
     // HH -> BCD TO ASCII
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';

    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}
void display_time(void)
{
    get_time();
    
    clcd_putch(time[0], LINE2(1)); //H
    clcd_putch(time[1], LINE2(2));//H
    
    clcd_putch(':', LINE2(3));
    
    clcd_putch(time[2], LINE2(4));//M
    clcd_putch(time[3], LINE2(5));//M
    
    clcd_putch(':', LINE2(6));
    
    clcd_putch(time[4], LINE2(7));//S
    clcd_putch(time[5], LINE2(8));//S
    
   
}
void display_dashboard(char event[], unsigned char speed)
{
    clcd_print("TIME     E  SP", LINE1(2));
    
    //display time from rtc
    display_time();
    //display event
    clcd_print(event,LINE2(11));
    //display speed
    clcd_putch((speed / 10) + '0', LINE2(14));  //int to char
    clcd_putch((speed % 10) + '0', LINE2(15));
}
void record_log()
{
    //eeprom write
    //write log string
    
    if(log_count == 10)
        log_count = 0;
    
    char addr = 0x05 + log_count * 10;
    log_count++;
    write_str_at24c04(addr, log); //hhmmssevsp
} 
void log_car_event(char event[], unsigned char speed)
{
    get_time();
    strncpy(log, time, 6); // hhmmss
    strncpy(&log[6], event, 2);
    log[8] = speed / 10 + '0';
    log[9] = speed % 10 + '0';
    log[10] = '\0';
    
    record_log();
}
unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char user_password[4];
    static unsigned  char i, attempt_left;
    //clcd_print("Enter Password", LINE1(1));
    if(reset_flag == RESET_PASSWORD)
    {
        key = ALL_RELEASED;
        attempt_left = 3;
        
        user_password[0] = '\0';
        user_password[1] = '\0';
        user_password[2] = '\0';
        user_password[3] = '\0';
        i = 0;
        return_time = 5; 
    }
    if(return_time == 0)
    {
        return RETURN_BACK;   
    }
    
    __delay_ms(50);
    if(key == SW4 && i < 4)
    {
        clcd_putch('*', LINE2(4 + i));
        user_password[i] = '1';
        i++;
        return_time = 5;
    }
    else if(key == SW5 && i < 4)
    {
        clcd_putch('*', LINE2(4 + i));
        user_password[i] = '0';
        i++;
        return_time = 5;
    }
    
    if(i == 4)
    {
        __delay_ms(1000);
        char stored_pwd[4];
        
        for(unsigned char j = 0; j < 4; j++)
            stored_pwd[j] = read_at24c04(j);
        
        if(strncmp(stored_pwd, user_password, 4) == 0)
        {
            clear_clcd_screen();
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_us(100);
            clcd_print("Login Success!", LINE1(1));
            __delay_ms(2000);
            return LOGIN_SUCCESS;
        }
        else
        {
            attempt_left--;
            if(attempt_left == 0)
            {
                clear_clcd_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("You are blocked", LINE1(0));
                clcd_print("Wait for", LINE2(0));
                clcd_print("secs", LINE2(12));
                //__delay_ms(3000); // -> 15 mint
                
                sec = 60;
                while(sec)
                {
                    clcd_putch(sec/10 + '0', LINE2(9));
                    clcd_putch(sec%10 + '0', LINE2(10));
                }
                attempt_left = 3;
            }
            else
            {
                clear_clcd_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("Wrong password", LINE1(0));
                clcd_putch(attempt_left + '0', LINE2(1));
                clcd_print("attempt left", LINE2(3));
                __delay_ms(3000);
                
            }
            clear_clcd_screen();
            clcd_print("Enter Password", LINE1(1));
            clcd_write(LINE2(4), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            i = 0;
            return_time = 5;
        }
    }
    return RETURN_NOTHING;
}
unsigned char menu_screen(unsigned char key, unsigned char reset_flag)
{
    static unsigned char menu_pos;
    if(reset_flag == RESET_MENU)
    {
        clear_clcd_screen();
        menu_pos = 0;
        return_time = 6;
    }
    if(return_time == 0)
        return RETURN_BACK;
    
    if(key == SW4 && menu_pos < 4)
    {
        clear_clcd_screen();
        menu_pos++;
        return_time = 5;
    }
    else if(key == SW5 && menu_pos > 0)
    {
        clear_clcd_screen();
        menu_pos--;
        return_time = 5;
    }
    if(menu_pos == 4)
    {
        clcd_putch('*', LINE2(0));
        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos],LINE2(2));
    }
    else
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1],LINE2(2));
    }
    return menu_pos;
}

unsigned char view_log(unsigned char key, unsigned char reset_flag)
{

    clcd_print("View logs", LINE1(4));
    static unsigned char log_pos = 0;
    if(reset_flag == RESET_LOG_POS)
    {
        log_pos = 0;
        return_time = 5;
    }
    if(return_time == 0)
        return RETURN_BACK;   
 
    if(key == SW4 )
    {
        return_time = 5;
        if(++log_pos == log_count || log_pos == 10)
            log_pos = 0; 
    }
    else if(key == SW5 && log_pos > 0 )
    {
        return_time = 5;
        log_pos--;
    }
    
    
    
    unsigned char addr = 0x05 + (log_pos * 10);
    for(unsigned char i = 0; i < 11; i++)
        log[i] = read_at24c04(addr + i);
    
    clcd_putch(log_pos % 10 + '0', LINE2(0));
    clcd_putch(log[0], LINE2(2));
    clcd_putch(log[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(log[2], LINE2(5));
    clcd_putch(log[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(log[4], LINE2(8));
    clcd_putch(log[5], LINE2(9));
    
    clcd_putch(log[6], LINE2(11));
    clcd_putch(log[7], LINE2(12));
    
    clcd_putch(log[8], LINE2(14));
    clcd_putch(log[9], LINE2(15));
    
    return RETURN_NOTHING;
}
void clear_log()
{
    log_count = 0;
    clear_clcd_screen();
    clcd_print("Logged data", LINE1(2));
    clcd_print("cleared..!", LINE2(2));
    __delay_ms(2500);
}

unsigned char change_password(unsigned key, unsigned char reset_flag)
{
    static unsigned char pwd_pos = 0, pwd_changed = 0;
    static unsigned char toggle_cursor = 0, blink_delay = 0;
    static char new_pwd[9];
    
    if (blink_delay++ == 5)
    {
        blink_delay = 0;
        toggle_cursor = !toggle_cursor;
    }

    /*____________________________________________________________________*/

    /*checking reset flag and rest change password */
    if (reset_flag == RESET_PASSWORD)
    {
        strncpy(new_pwd, "    ", 4);
        pwd_pos = 0;
        pwd_changed = 0;
        return_time = 5;
    }

    /*____________________________________________________________________*/

    if (!return_time)
        return RETURN_BACK;

    /*____________________________________________________________________*/

    if (pwd_changed)
        return TASK_FAILURE;

    /*____________________________________________________________________*/

    if (pwd_pos < 4)
    {
        clcd_print("Enter new pwd:  ", LINE1(0));

        /* blinking the cursor */
        if (toggle_cursor == 0)
        {
            clcd_putch((unsigned char) 0xFF, LINE2(pwd_pos));
        }
        else
        {
            clcd_putch(' ', LINE2(pwd_pos));
        }
    }
    else if (pwd_pos > 3 && pwd_pos < 8)
    {
        clcd_print("Re-enter new pwd", LINE1(0));

        /* blinking the cursor */
        if (toggle_cursor == 0)
        {
            clcd_putch((unsigned char) 0xFF, LINE2(pwd_pos));
        }
        else
        {
            clcd_putch(' ', LINE2(pwd_pos));
        }
    }

    /*____________________________________________________________________*/

    switch (key)
    {
        case SW5: /* storing new password as 0*/
            new_pwd[pwd_pos] = '0';
            clcd_putch('*', LINE2(pwd_pos));
            pwd_pos++;
            return_time = 5;
            if (pwd_pos == 4)
                clcd_print("                 ", LINE2(0));
            break;

        case SW4: /* storing new password as 1 */
            new_pwd[pwd_pos] = '1';
            clcd_putch('*', LINE2(pwd_pos));
            pwd_pos++;
            return_time = 5;
            if (pwd_pos == 4)
                clcd_print("                 ", LINE2(0));
            break;
    }

    /*____________________________________________________________________*/

    if (pwd_pos == 8)
    {
        if (strncmp(new_pwd, &new_pwd[4], 4) == 0) /* if both entered password matched */
        {
            /* successfully changing password */
            new_pwd[8] = 0;
            write_str_at24c04(0x00, &new_pwd[4]); /* storing new password */
            pwd_pos++;
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            clcd_print("Password changed", LINE1(0));
            clcd_print("successfully ", LINE2(2));
            pwd_changed = 1;
            __delay_ms(1000);
            return TASK_SUCCESS;
        }
        else
        {
            /* displaying password change fail */
            clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
            clcd_print("Password  change", LINE1(0));
            clcd_print("failed", LINE2(5));
            pwd_changed = 1;
            __delay_ms(1000);

            return TASK_SUCCESS;
        }
    }

    return TASK_FAILURE;
}




void clear_clcd_screen()
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}
