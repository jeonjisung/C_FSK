#include "fsk.h"

u8 tx_buf[RX_BUFFER_SIZE];
u8 rx_buf[RX_BUFFER_SIZE];
u8 rxnbbyte;
u8 txnbbye;
u8 txpacket_size;
u8 rxpacket_size;
u8 fifothresh;
u8 state;

InterruptIn DIO0(PB_6);
InterruptIn DIO1(PB_7);
InterruptIn DIO2(PC_9);

void fsk_init()
{
    write_reg_single(REG_OPMODE, (read_reg_single(REG_OPMODE) & RF_OPMODE_MASK) | RF_OPMODE_SLEEP);
    set_frequency(FRF);
    write_reg_single(REG_OPMODE, (read_reg_single(REG_OPMODE) & RF_OPMODE_LONGRANGEMODE_MASK) | RF_OPMODE_LONGRANGEMODE_OFF);
    write_reg_single(REG_OPMODE, (read_reg_single(REG_OPMODE) & 0xF7) | 0x00);
    write_reg_single(REG_OPMODE, (read_reg_single(REG_OPMODE) & RF_OPMODE_MASK) | RF_OPMODE_STANDBY);
    write_reg_single(REG_PAYLOADLENGTH, RX_BUFFER_SIZE);
    write_reg_single(REG_FIFOTHRESH, 0x80 | 0x20);
    write_reg_single(REG_PACONFIG, (read_reg_single(REG_PACONFIG) & RF_PACONFIG_PASELECT_MASK) | RF_PACONFIG_PASELECT_PABOOST);
    write_reg_single(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);
    write_reg_single(REG_DIOMAPPING1, 0x00);

    rx_config();
    tx_config();

    set_mode(RF_OPMODE_RECEIVER);
}

void set_frequency(u32 frequency)
{
    u32 frf = frequency / FSTEP;
    write_reg_single(REG_FRFMSB, (frf & 0xFFFFFF) >> 16);
    write_reg_single(REG_FRFMID, (frf & 0xFFFF) >> 8);
    write_reg_single(REG_FRFLSB, frf & 0xFF);
}

void rx_config()
{
    write_reg_single(REG_BITRATEMSB, RF_BITRATEMSB_50000_BPS);
    write_reg_single(REG_BITRATELSB, RF_BITRATELSB_50000_BPS);

    write_reg_single(REG_RXCONFIG, (read_reg_single(REG_RXCONFIG) & RF_RXCONFIG_AFCAUTO_MASK) | RF_RXCONFIG_AFCAUTO_ON);

    write_reg_single(REG_RXBW, (read_reg_single(REG_RXBW) & RF_RXBW_MANT_MASK & RF_RXBW_EXP_MASK) | RF_RXBW_MANT_20 | RF_RXBW_EXP_3);
    write_reg_single(REG_AFCBW, (read_reg_single(REG_AFCBW) & RF_AFCBW_MANTAFC_MASK & RF_AFCBW_EXPAFC_MASK) | RF_AFCBW_MANTAFC_24 | RF_AFCBW_EXPAFC_2);

    write_reg_single(REG_PACKETCONFIG1, (read_reg_single(REG_PACKETCONFIG1) & RF_PACKETCONFIG1_PACKETFORMAT_MASK) | RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE);
    write_reg_single(REG_PACKETCONFIG2, (read_reg_single(REG_PACKETCONFIG2) | RF_PACKETCONFIG2_DATAMODE_PACKET));
}

void tx_config()
{
    write_reg_single(REG_FDEVMSB, RF_FDEVMSB_25000_HZ);
    write_reg_single(REG_FDEVLSB, RF_FDEVLSB_25000_HZ);

    write_reg_single(REG_BITRATEMSB, RF_BITRATEMSB_50000_BPS);
    write_reg_single(REG_BITRATELSB, RF_BITRATELSB_50000_BPS);

    write_reg_single(REG_PACKETCONFIG1, (read_reg_single(REG_PACKETCONFIG1) & RF_PACKETCONFIG1_PACKETFORMAT_MASK) | RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE);
    write_reg_single(REG_PACKETCONFIG2, (read_reg_single(REG_PACKETCONFIG2) | RF_PACKETCONFIG2_DATAMODE_PACKET));
}

void set_mode(u8 mode)
{
    write_reg_single(REG_OPMODE, (read_reg_single(REG_OPMODE) & RF_OPMODE_MASK) | mode);
}

void Send_rf(u8 *buffer, u8 size)
{
    DIO0.enable_irq();
    DIO1.enable_irq();

    DIO0.rise(&DIO0_int);
    DIO1.fall(&DIO1_fall);

    memcpy(tx_buf, buffer, size);

    set_mode(RF_OPMODE_STANDBY);

    u8 chunksize = 0;
    txnbbyte = 0;
    txpacket_size = size;

    if (size < 64)
        chunksize = size;
    else
        chunksize = 63;

    write_reg_single(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);

    write_reg_single(REG_DIOMAPPING1, (read_reg_single(REG_DIOMAPPING1) & RF_DIOMAPPING1_DIO0_MASK &
                                       RF_DIOMAPPING1_DIO1_MASK) |
                                          RF_DIOMAPPING1_DIO0_00 |
                                          RF_DIOMAPPING1_DIO1_00);

    state = TX_RUNNING;

    printf("tx : [%s]\r\n", tx_buf);
    set_mode(RF_OPMODE_TRANSMITTER);

    write_fifo(&txpacket_size, 1);
    write_fifo(tx_buf, chunksize);

    txnbbyte = chunksize;
}

void Recv_rf()
{
	DIO0.enable_irq();
	DIO1.enable_irq();

	DIO0.rise(&DIO0_int);
	DIO1.rise(&DIO1_int);
	
	set_mode(RF_OPMODE_STANDBY);
	rxnbbyte = 0;
	rxpacket_size = 0;
	memset(rx_buf, 0, RX_BUFFER_SIZE);
	memset(tx_buf, 0, RX_BUFFER_SIZE);

	//wait(0.3);

	//write_reg_single(REG_RXCONFIG, RF_RXCONFIG_AFCAUTO_ON | RF_RXCONFIG_AGCAUTO_ON | RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT);
	write_reg_single(REG_IRQFLAGS1, RF_IRQFLAGS1_RSSI | RF_IRQFLAGS1_PREAMBLEDETECT);
	write_reg_single(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);

	write_reg_single(REG_DIOMAPPING1, (read_reg_single(REG_DIOMAPPING1) & RF_DIOMAPPING1_DIO0_MASK &
		RF_DIOMAPPING1_DIO1_MASK) |
		RF_DIOMAPPING1_DIO0_00 |
		RF_DIOMAPPING1_DIO1_00);

	state = RX_RUNNING;
	set_mode(RF_OPMODE_RECEIVER);
	//wait(0.3);
}

bool Rx()
{
	bool remain;
	u8 rxchunksize = 0;

	if ((rxpacket_size == 0) && (rxnbbyte == 0)) read_fifo(&rxpacket_size, 1);

	if ((rxpacket_size - rxnbbyte) < 32)
	{
		rxchunksize = rxpacket_size - rxnbbyte;
		remain = true;
	}
	else
	{
		rxchunksize = 32;
		remain = false;
	}

	if (rxchunksize > 0)
	{
		read_fifo(rx_buf + rxnbbyte, rxchunksize);
		//hexprint("read rx int", rx_buf, rxnbbyte);
		rxnbbyte += rxchunksize;
	}
	
	return remain;
}


void Tx()
{
	//printf("tx\r\n");

	//printf("txirq1 0 0x%02X\r\n", read_reg_single(REG_IRQFLAGS1));
	//printf("txirq2 0 0x%02X\r\n", read_reg_single(REG_IRQFLAGS2));
	u8 chunksize = 0;

	//printf("tx p c n1 [%d %d %d]\r\n", packet_size, chunksize, nbbyte);
	if ((txpacket_size - txnbbyte) < 32) chunksize = txpacket_size - txnbbyte;
	else chunksize = 32;

	//printf("tx p c n2 [%d %d %d]\r\n", txpacket_size, chunksize, txnbbyte);

	if (chunksize > 0)
	{
		write_fifo(tx_buf+txnbbyte, chunksize);
		txnbbyte += chunksize;
	}
}

void DIO0_int()
{
    if (state == TX_RUNNING)
    {
        if (read_reg_single(REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT)
        {
            DIO0.disable_irq();
            DIO0.disable_irq();

            write_reg_single(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);
            set_mode(RF_OPMODE_SLEEP);

            Recv_rf();
        }
    }
    else if (state == RX_RUNNING){
        if(read_reg_single(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY){
            if(Rx())
            {
                DIO0.disable_irq();
                DIO0.disable_irq();

                printf("read rx [%s]\r\n", rx_buf);
				//hexprint("read rx", rx_buf, rxpacket_size);

				set_mode(RF_OPMODE_SLEEP);
				write_reg_single(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);
            }
        }
    }
}

void DIO1_int(){
    if (state == RX_RUNNING)
	{
		if ((read_reg_single(REG_IRQFLAGS2)) & RF_IRQFLAGS2_FIFOLEVEL)
		{
			//printf("fifolevel\r\n");
			Rx();
		}
	}
}

void DIO1_fall()
{
    if (state == TX_RUNNING)
	{
		//printf("opmode fall 0x%02X\r\n", read_reg_single(REG_OPMODE));
		Tx();
	}
}