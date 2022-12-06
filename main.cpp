#include "fsk.h"

#define WRITEBUF_SIZE 70

Serial pc(PA_9, PA_10);

u8 message[] = "b";

int main()
{
    pc.baud(115200);

    spi_init();

    fsk_init();
    wait(1);

    while(1)
    {
        Send_rf(message, strlen((char*)message));

        wait(5);
    }
}