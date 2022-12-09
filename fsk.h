#ifndef FSK_H
#define FSK_H
#include "spi.h"

#define FRF 915000000
#define FSTEP 61

#define RX_BUFFER_SIZE 255

#define STANDBY 0
#define RX_RUNNING 1
#define TX_RUNNING 2

extern u8 tx_buf[RX_BUFFER_SIZE];
extern u8 rx_buf[RX_BUFFER_SIZE];
extern u8 rxnbbyte;
extern u8 txnbbyte;
extern u8 txpacket_size;
extern u8 rxpacket_size;
extern u8 fifothresh;
extern u8 state;

extern int ismaster;

void fsk_init();
void set_frequency(u32 frequency);
void rx_config();
void tx_config();
void set_mode(u8 mode);
void Send_rf(u8 *buffer, u8 size);
void Recv_rf();
bool Rx();
void Tx();
void DIO0_up();
void DIO1_up();
void DIO1_down();

#endif // !1