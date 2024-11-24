#include "main.h"
unsigned char clock_reg[3], return_time, sec;
char time[7];  //HHMMSS
char log[11]; //HHMMSSEVSP
char log_count = 0;
char *menu[] = { "View log", "Clear log", "Download log", "Set time", "Change passwrd"};   //menu


/*Function to get time from RTC (DS1307)*/
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

/*Function to display the time in CLCD*/
void display_time(void)
{
    get_time();                            //function call to get time fromRTC
    
    clcd_putch(time[0], LINE2(1)); //H
    clcd_putch(time[1], LINE2(2));//H
    
    clcd_putch(':', LINE2(3));
    
    clcd_putch(time[2], LINE2(4));//M
    clcd_putch(time[3], LINE2(5));//M
    
    clcd_putch(':', LINE2(6));
    
    clcd_putch(time[4], LINE2(7));//S
    clcd_putch(time[5], LINE2(8));//S
    
   
}

/*Function to display the time event and speed on the CLCD*/
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

/*Function to record the log in external EEPROM AT24C04*/
void record_log()
{

    if(log_count == 10)             //stored only 10 logs
        log_count = 0;
    
    char addr = 0x05 + log_count * 10;    //find address on eeprom
    log_count++;
    write_str_at24c04(addr, log); //hhmmssevsp
} 

/*Function to store car events*/
void log_car_event(char event[], unsigned char speed)
{
    get_time();
    strncpy(log, time, 6); // hhmmss
    strncpy(&log[6], event, 2);
    log[8] = speed / 10 + '0';
    log[9] = speed % 10 + '0';
    log[10] = '\0';
    
    record_log();     //fucntion call to store logs
}

/*Function to Login screen */
unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char user_password[4];
    static unsigned  char i, attempt_left;
    
    if(reset_flag == RESET_PASSWORD)            //for the first time reset user_password
    {
        key = ALL_RELEASED;                     // key released 
        attempt_left = 3;                       //reset attempts left
        
        user_password[0] = '\0';
        user_password[1] = '\0';
        user_password[2] = '\0';
        user_password[3] = '\0';
        i = 0;
        return_time = 5;                        //reset return time
    }
    if(return_time == 0)                        //if no action detected upto 5sec return to change the screen
    {
        return RETURN_BACK;   
    }
    
    __delay_ms(50);
    if(key == SW4 && i < 4)                             //if key = sw4 take 1 as user input
    {
        clcd_putch('*', LINE2(4 + i));                  //display *
        user_password[i] = '1';
        i++;
        return_time = 5;                            //reset to 5sec
    }
    else if(key == SW5 && i < 4)                    //if key == sw5 take 0 as user input
    {
        clcd_putch('*', LINE2(4 + i));              //display *
        user_password[i] = '0';
        i++;
        return_time = 5;
    }
    
    if(i == 4)                      //if i == 4 compare passwords
    {
        __delay_ms(1000);
        char stored_pwd[4];
        
        for(unsigned char j = 0; j < 4; j++)
            stored_pwd[j] = read_at24c04(j);  // read password from external EEPROM and stored in stored_pwd
        
        if(strncmp(stored_pwd, user_password, 4) == 0)    //comapre both passwords, if same 
        {
            clear_clcd_screen();
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);     //turn off the cursor
            __delay_us(100);
            clcd_print("Login Success!", LINE1(1));        // print message in clcd
            __delay_ms(2000);
            return LOGIN_SUCCESS;                           //return to main
        }
        else                                    //if passwords are not same
        {
            attempt_left--;                     //decrease attempts
            /*logic to if attempts are 0, block 1 minute*/
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
                attempt_left = 3;    //after 1minute reset attempt as 3
            }
            else                    //if attempts are not 0, display remaining attempts
            {
                clear_clcd_screen();
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                clcd_print("Wrong password", LINE1(0));
                clcd_putch(attempt_left + '0', LINE2(1));
                clcd_print("attempt left", LINE2(3));
                __delay_ms(3000);
                
            }
            /*logic for after  ENTER wrong password or attempt is zero*/
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

/*Function to display the menu's on the clcd screen*/
unsigned char menu_screen(unsigned char key, unsigned char reset_flag)
{
    static unsigned char menu_pos;
    /*logic for first time , reset menu posiition and timer sec*/
    if(reset_flag == RESET_MENU)
    {
        clear_clcd_screen();
        menu_pos = 0;
        return_time = 6;
    }
    if(return_time == 0)
        return RETURN_BACK;      //if upto 5 or 6 sec no activity detected return to main and change the screen
    
    if(key == SW4 && menu_pos > 0)   //if sw4 move UP
    {
        clear_clcd_screen();
        menu_pos--;                  //decrease menu pos
        return_time = 5;             //reset time for 5sec
    }
    else if(key == SW5 && menu_pos < 4)  //if sw5 move DOWN
    {
        clear_clcd_screen();
        menu_pos++;                 //increase menu pos
        return_time = 5;            // reset time for 5sec
    }
    
  /*==========  display the menus  ===================*/
    if (menu_pos < 4)
    {
        clcd_putch('*', LINE1(0));         // * for indicate menu position
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
    }
    else if (menu_pos == 4)
    {

        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos], LINE2(2));
        clcd_putch('*', LINE2(0));
    }
    return menu_pos;   // return menu position  tom main to select that particular menu]
}

/*Function to view the entered log on the CLCD screen*/
unsigned char view_log(unsigned char key, unsigned char reset_flag)
{

    clcd_print("View logs", LINE1(4));      
    static unsigned char log_pos = 0;
    
    /*For the first time reset the log postion ,( start from 0th log*/
    if(reset_flag == RESET_LOG_POS)
    {
        log_pos = 0;
        return_time = 5;
    }
    /*if no activity detected on last 5sec return to main*/
    if(return_time == 0)
        return RETURN_BACK;   
    /*if sw4 move to next log*/
    if(key == SW4 )
    {
        return_time = 5;
        ++log_pos;
        if(log_pos == 10 || log_pos == log_count)     //view last 10 logs
            log_pos = 0; 
    }
    /*if sw5 move to previous log*/
    else if(key == SW5 && log_pos > 0 )
    {
        return_time = 5;
        log_pos--;
    }
    
    
    
    unsigned char addr = 0x05 + (log_pos * 10);  //claculate address to read log from eeprom
    for(unsigned char i = 0; i < 11; i++)
        log[i] = read_at24c04(addr + i);        //read data from eeprom
    
    
    /*Display the logs on the CLCD screen*/
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

/*Function to clear the all entered logs*/
void clear_log()
{
    log_count = 0;              //change log count as 0 to clear the log
    
    /*clear the screen and display the message*/
    clear_clcd_screen();
    clcd_print("Logged data", LINE1(2));
    clcd_print("cleared..!", LINE2(2));
    __delay_ms(2500);
}

/*Function to download the logs to system*/
void download_log(void)
{ 
    int index = -1;
    char position = 0, log[11];
    unsigned char addr;
    log[10] = 0;
    
    if(log_count == 0)         //for no logs enterd
        puts("No Logs Available !");
    else
    {
        /*Logic to download the logs to the system using UART protocol*/
        puts("Logs :");
        putchar('\n');
        puts("#     Time       Event     Speed");
        putchar('\n');
        putchar('\r');
        
        while(index < log_count)
        {
            position = index + 1;
            index++;
            
            for(unsigned char i = 0; i < 10; i++)
            {
                addr = (position * 10) + 0x05 + i;
                log[i] = read_at24c04(addr);
            }
            
            putchar(index + '0');
            puts("   ");
            
            putchar(log[0]);//H
            putchar(log[1]);//H
            putchar(':');
            putchar(log[2]);//M
            putchar(log[3]);//M
            putchar(':');
            putchar(log[4]);//S
            putchar(log[5]);//S
            
            puts("      ");
            putchar(log[6]);
            putchar(log[7]);
            
            puts("        ");
            putchar(log[8]);
            putchar(log[9]);
            putchar('\n');
            putchar('\r');
        }
    }
}

/*Function to change the Time in RTC using I2C protocol*/
unsigned char change_time(unsigned char key, unsigned char reset_flag)
{
    static unsigned char new_time[3], blink_pos, blink, wait, done = 0;
    char buffer;
    
    
    if(reset_flag == RESET_TIME)
    {
        get_time();        //function to get the time from RTC 
        
        new_time[0] = ((time[0] & 0x0F) * 10) + (time[1] & 0x0F);//HH
        new_time[1] = ((time[2] & 0x0F) * 10) + (time[3] & 0x0F);//MM
        new_time[2] = ((time[4] & 0x0F) * 10) + (time[5] & 0x0F);//SS
        
        clcd_print("Time <HH:MM:SS>", LINE1(0));
        blink_pos = 2;
        blink = 0;
        wait = 0;
        done = 0;
        return_time = 10;
        key = ALL_RELEASED;
    }
    
    if(return_time == 0)
        return RETURN_BACK;                 //return to main
    
    if(done)
        return TASK_FAILURE;                // block re-enter 
    
    switch(key)
    {
        case SW4:                     //if sw4 increase value of that particular field
            new_time[blink_pos]++;
            return_time = 7;
            break;
        case SW5: 
            blink_pos = (blink_pos + 1) % 3;  //if sw5 move to next position
            return_time = 7;
            break;
            
        case SW4_L_P:           //if sw4 as long press , reset the time in the RTC
            
            get_time();
            
            /* Hour */
            buffer = (char)((new_time[0] / 10) << 4) | (new_time[0] % 10);
            clock_reg[0] = (clock_reg[0] & 0xC0) | buffer;
            write_ds1307(HOUR_ADDR, clock_reg[0]);          //write time in RTC
            
            /*Minute*/
            buffer = (char)((new_time[1] / 10) << 4) | (new_time[1] % 10);
            clock_reg[1] = (clock_reg[1] & 0x80) | buffer;
            write_ds1307(MIN_ADDR, clock_reg[1]);           //write time in RTC
            
            /*Seconds*/
            buffer = (char)((new_time[2] / 10) << 4) | (new_time[2] % 10);
            clock_reg[2] = (clock_reg[2] & 0x80) | buffer;
            write_ds1307(SEC_ADDR, clock_reg[2]);           //write time in RTC
            
            clear_clcd_screen();
            clcd_print("Time Changed", LINE1(2));
            clcd_print("Successfully", LINE2(2));
            done = 1;
            __delay_ms(1500);
            
            return TASK_SUCCESS; 
    }
    
    if(new_time[0] > 23) new_time[0] = 0;      //HH
    if(new_time[1] > 59) new_time[1] = 0;      //MM
    if(new_time[2] > 59) new_time[2] = 0;      //SS
    
    /*logic for cursor blink*/
    if(wait++ == 1)
    {
        wait = 0;
        blink = !blink;
        if(blink)
        {
            switch(blink_pos)
            {
                case 0:
                    clcd_print("  ", LINE2(0));
                    __delay_ms(170);
                    break;
                case 1:
                    clcd_print("  ", LINE2(3));
                    __delay_ms(170);
                    break;
                case 2:
                    clcd_print("  ", LINE2(6));
                    __delay_ms(170);
                    break;
            }
        }
    }
    
    /*    Display the time    */
    clcd_putch(new_time[0] / 10 + '0', LINE2(0)); //H
    clcd_putch(new_time[0] % 10 + '0', LINE2(1));//H
    
    clcd_putch(':', LINE2(2));
    
    clcd_putch(new_time[1] / 10 + '0', LINE2(3));//M
    clcd_putch(new_time[1] % 10 + '0', LINE2(4));//M
    
    clcd_putch(':', LINE2(5));
    
    clcd_putch(new_time[2]/ 10 + '0', LINE2(6));//S
    clcd_putch(new_time[2] % 10 + '0', LINE2(7));//S
    
    return TASK_FAILURE;
}

/*Function to change the stored password in external EEPROM*/
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

  

    /*checking reset flag and rest change password */
    if (reset_flag == RESET_PASSWORD)
    {
        strncpy(new_pwd, "    ", 4);
        pwd_pos = 0;
        pwd_changed = 0;
        return_time = 5;
    }

    

    if (!return_time)
        return RETURN_BACK;


    if (pwd_changed)
        return TASK_FAILURE;

   /* Logic for Read new password from user*/
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
    /*logic for read conformation password from user*/
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

  
    /*based on the key password stored*/
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



/*Function to claer the CLCD screen */
void clear_clcd_screen()
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}
