/* 
 * File:   car_block_box.h
 * Author: sudh1
 *
 * Created on November 15, 2024, 5:30 AM
 */

#ifndef CAR_BLOCK_BOX_H
#define	CAR_BLOCK_BOX_H

void display_dashboard( char event[], unsigned char speed);
void log_car_event( char event[], unsigned char speed);
void clear_clcd_screen(void);
unsigned char login(unsigned char key, unsigned char reset_flag);
unsigned char menu_screen(unsigned char key, unsigned char reset_flag);

unsigned char view_log(unsigned char key, unsigned char reset_flag);
void clear_log(void);
unsigned char change_password(unsigned key, unsigned char reset_flag);
void clear_passwd(unsigned char arr[]);

#endif	/* CAR_BLOCK_BOX_H */

