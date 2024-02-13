#include <stdio.h>

#include <unistd.h>
#include <thread>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

#include "linux_spi.h"
#include "CppTimerCallback.h"

#define SPI_RAW_RX_TX_BUFFER_SIZE      2060U

class Callback_SPI_Rx : public CppTimerCallback::Runnable {
	void run() {
		fprintf(stdout,"\n\t callbackSPI_Rx \n! ");
		fflush(stdout);
	}
};


class Callback_SPI_Tx : public CppTimerCallback::Runnable {
	void run() {
		fprintf(stdout,"\n\t callbackSPI_Tx! \n");
		fflush(stdout);
	}
};


int main( int, const char**) {
    enum multi_taks_ID { SPI_Rx_task, SPI_Tx_task, SPI_TxQueueHandler_task, SPI_RxQueueHandler_task  };

	CppTimerCallback TimerSPI_Rx;
	Callback_SPI_Rx  callbackSPI_Rx;

	CppTimerCallback TimerSPI_Tx;
	Callback_SPI_Tx  callbackSPI_Tx;


	TimerSPI_Rx.registerEventRunnable(callbackSPI_Rx);
    TimerSPI_Rx.taskID = SPI_Rx_task;
	TimerSPI_Rx.startms(2000); // TimerSPI_Rx.startns(250000000);

	TimerSPI_Tx.registerEventRunnable(callbackSPI_Tx);
    TimerSPI_Tx.taskID = SPI_Tx_task;
	TimerSPI_Tx.startms(200);

	for (;;);
	
	// do nothing and keep sleeping for 2 secs
	std::this_thread::sleep_for(std::chrono::seconds(2));

	TimerSPI_Rx.unregisterEventRunnable();
        TimerSPI_Rx.stop();

	TimerSPI_Tx.unregisterEventRunnable();
        TimerSPI_Tx.stop();

	printf("End\n");


return EXIT_SUCCESS; // good job
}



void test_read_write(Linux_SPI *spi_dev)
{

    uint8_t Tx_buff[SPI_RAW_RX_TX_BUFFER_SIZE];
    //uint8_t Rx_buff[6] = {68, 69, 70, 71, 72, 73};
    uint8_t Rx_buff[SPI_RAW_RX_TX_BUFFER_SIZE]; 
        
    int ret = 0;


    ret = spi_dev->write(&Tx_buff, sizeof(Tx_buff));
    printf("\t sle send %s;   ret == %d\n", Tx_buff, ret);

    //sleep(1); 
    ret = 0;
    ret = spi_dev->read(&Rx_buff, sizeof(Rx_buff));
    printf("\t sle read ret == %d;  Rx_buff[] == %s\n", ret, Rx_buff);

}



void test_send_tr(Linux_SPI *spi_dev)
{
    struct spi_ioc_transfer mesg[2];
    uint16_t val = 1600;
    int ret, i;
    uint16_t buf[1056];
    memset(buf,  0, sizeof(buf));
    memset(mesg, 0, sizeof(mesg));


    mesg[0].bits_per_word = 8;
    mesg[0].rx_buf        = (uintptr_t)NULL;
    mesg[0].tx_buf        = (uintptr_t)&val;
    mesg[0].len           = 6;
    mesg[0].cs_change     = 1;
//    mesg[0].delay_usecs   = 10000;


    mesg[1].bits_per_word = 8;
    mesg[1].rx_buf        = (uintptr_t)buf;
    mesg[1].tx_buf        = (uintptr_t)NULL;
    mesg[1].len           = 1056;
    mesg[1].cs_change     = 0;



    ret = spi_dev->send_tr(mesg, 2);


    printf("send_tr ret == %d\n", ret);


    for(i = 0; i < 1056; i++)
        printf("buf[%d] == %d\n", i, buf[i]);
 
 
}



void init_spi_dev(Linux_SPI *spi_dev)
{


    if( spi_dev->dev_open("/dev/spidev0.2") != 0 )
    {
        printf("Error: dev_open %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }


    if( spi_dev->set_mode(SPI_MODE_1) != 0 )
    {
        printf("Error: %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }


    if( spi_dev->set_bits_per_word(8) != 0 )
    {
        printf("Error: %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }


    if( spi_dev->set_max_speed_hz(4000000) != 0 )
    {
        printf("Error: %s\n", spi_dev->strerror(spi_dev->get_errno()));
        exit(-1);
    }


}



void run_test()
{

    Linux_SPI spi_dev;


    init_spi_dev(&spi_dev);

    test_read_write(&spi_dev);
 //   test_send_tr(&spi_dev);


    spi_dev.dev_close();
}



