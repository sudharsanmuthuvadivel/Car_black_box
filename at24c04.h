

#ifndef AT24C04_H
#define	AT24C04_H

#define AT24C04_SLAVE_WRITE             0b10100000 // 0xA0
#define AT24C04_SLAVE_READ              0b10100001 // 0xA1


//void init_at24c04(void);
unsigned char read_at24c04(unsigned char);
void write_at24c04(unsigned char addr, unsigned char data);
void write_str_at24c04(unsigned char addr, char *data);
#endif	/* AT24C04_H */

