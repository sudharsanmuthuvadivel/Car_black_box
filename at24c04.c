

#include "main.h"

/*(void init_at24c04(void)
{
    unsigned char dummy;
    dummy = read_at24c04(SEC_ADDR);
    dummy = dummy & 0x7F;
    write_at24c04(SEC_ADDR, dummy);
}*/

unsigned char read_at24c04(unsigned char addr)
{
    unsigned char data;
    i2c_start();
    i2c_write(AT24C04_SLAVE_WRITE);
    i2c_write(addr);
    i2c_rep_start();
    i2c_write(AT24C04_SLAVE_READ);
    data = i2c_read(0);
    i2c_stop();
    
    return data;
}

void write_at24c04(unsigned char addr, unsigned char data)
{
    i2c_start();
    i2c_write(AT24C04_SLAVE_WRITE);
    i2c_write(addr);
    i2c_write(data);
    i2c_stop();
    
    __delay_ms(3);
}
void write_str_at24c04(unsigned char addr, char *data)
{
    while(*data != '\0')
    {
        write_at24c04(addr, *data);
        data++;
        addr++;
    }
}