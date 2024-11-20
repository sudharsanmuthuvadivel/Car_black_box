/* 
 * File:   main.h
 * Author: sudh1
 *
 * Created on November 15, 2024, 5:35 AM
 */

#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
#include "i2c.h"
#include"ds1307.h"
#include "clcd.h"
#include "adc.h"
#include "digital_keypad.h"
#include "car_block_box.h"
#include "at24c04.h"
#include "timer.h"
#include "string.h"

#define MAX_PASSWD_CNT          4
#define DASH_BOARD_SCREEN       0x01
#define LOGIN_SCREEN            0x02
#define MAIN_MENU_SCREEN        0x03
#define VIEW_LOG_SCREEN         0x04
#define VIEW_LOG                0x05
#define CLEAR_LOG               0x06
#define DOWNLOAD_LOG            0x07
#define SET_TIME                0x08
#define CHANGE_PASSWORD         0x09

#define RESET_PASSWORD          0x11
#define RESET_NOTHING           0x00
#define RESET_LOG_POS           0x23


#define RETURN_BACK             0x33
#define RETURN_DASH             0x31
#define RETURN_NOTHING          0x32
#define LOGIN_SUCCESS           0x44
#define RESET_MENU              0x55

#define TASK_FAILURE            0x66
#define TASK_SUCCESS            0x77


#endif	/* MAIN_H */
