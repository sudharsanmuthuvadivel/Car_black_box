

#ifndef CAR_BLOCK_BOX_H
#define	CAR_BLOCK_BOX_H


/*=============  Function prototypes ==============*/
void display_dashboard( char event[], unsigned char speed);
void log_car_event( char event[], unsigned char speed);
void clear_clcd_screen(void);

unsigned char login(unsigned char key, unsigned char reset_flag);
unsigned char menu_screen(unsigned char key, unsigned char reset_flag);
unsigned char view_log(unsigned char key, unsigned char reset_flag);
void clear_log(void);
void download_log(void);
unsigned char change_password(unsigned key, unsigned char reset_flag);
unsigned char change_time(unsigned char key, unsigned char reset_flag);



#endif	/* CAR_BLOCK_BOX_H */

