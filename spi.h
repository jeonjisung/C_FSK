#ifndef MYSPI_H
#define MYSPI_H

#include "mbed.h"
#include <stdio.h>
#include <string.h>
#include "reg.h"

#define MOSI PB_5
#define MISO PB_4
#define SCK PB_3
#define NSS PA_15

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define NSS_HIGH nss = 1;
#define NSS_LOW nss = 0;

void write_fifo(u8* buffer, u8 size);
void read_fifo(u8* buffer, u8 size);
void write_reg_single(u8 reg, u8 data);
void write_reg_multi(u8 reg, u8* data, u8 size);
u8 read_reg_single(u8 reg);
void read_reg_multi(u8 reg, u8* buffer, u8 size);
void spi_init();


#endif // !1