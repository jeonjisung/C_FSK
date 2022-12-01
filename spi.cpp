#include "spi.h"	

SPI spi(MOSI, MISO, SCK);
DigitalOut nss(NSS);

void spi_init()
{
	NSS_HIGH;

	spi.format(8, 0);
	spi.frequency(10000000);

	wait_us(100);
}

void write_reg_single(u8 reg, u8 data)
{
	write_reg_multi(reg, &data, 1);
}

void write_reg_multi(u8 reg, u8* data, u8 size)
{
	NSS_LOW;
	spi.write(reg | 0x80);

	for (int i = 0; i < size; ++i)
	{
		spi.write(data[i]);
	}

	NSS_HIGH;
}

u8 read_reg_single(u8 reg)
{
	u8 byte = 0;
	read_reg_multi(reg, &byte, 1);
	return byte;
}

void read_reg_multi(u8 reg, u8* buffer, u8 size)
{
	NSS_LOW;
	spi.write(reg & 0x7F);

	for (int i = 0; i < size; ++i)
	{
		buffer[i] = spi.write(0);
	}

	NSS_HIGH;
}

void write_fifo(u8 *buffer, u8 size)
{
    write_reg_multi(0, buffer, size);
}

void read_fifo(u8 *buffer, u8 size)
{
    read_reg_multi(0, buffer, size);
}