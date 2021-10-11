#include "SpiSlave.h"
#include <Arduino.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include <esp_types.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "soc/rtc_periph.h"
#include "driver/spi_slave.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"


/*
    SPI receiver (slave) example.
    This example is supposed to work together with the SPI sender. It uses the standard SPI pins (MISO, MOSI, SCLK, CS) to
    transmit data over in a full-duplex fashion, that is, while the master puts data on the MOSI pin, the slave puts its own
    data on the MISO pin.
    This example uses one extra pin: GPIO_HANDSHAKE is used as a handshake pin. After a transmission has been set up and we're
    ready to send/receive data, this code uses a callback to set the handshake pin high. The sender will detect this and start
    sending a transaction. As soon as the transaction is done, the line gets set low again.
*/

/*
    Pins in use. The SPI Master can use the GPIO mux, so feel free to change these if needed.
*/
/*  #if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
    #define GPIO_HANDSHAKE 2
    #define GPIO_MOSI GPIO_NUM_12
    #define GPIO_MISO GPIO_NUM_13
    #define GPIO_SCLK GPIO_NUM_15
    #define GPIO_CS GPIO_NUM_14

    #elif CONFIG_IDF_TARGET_ESP32C3
    #define GPIO_HANDSHAKE 3
    #define GPIO_MOSI GPIO_NUM_7
    #define GPIO_MISO GPIO_NUM_2
    #define GPIO_SCLK GPIO_NUM_6
    #define GPIO_CS GPIO_NUM_10

    #elif CONFIG_IDF_TARGET_ESP32S3
    #define GPIO_HANDSHAKE 2
    #define GPIO_MOSI GPIO_NUM_11
    #define GPIO_MISO GPIO_NUM_13
    #define GPIO_SCLK GPIO_NUM_12
    #define GPIO_CS GPIO_NUM_10

    #endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2


    #ifdef CONFI  G_IDF_TARGET_ESP32
    #define RCV_HOST    HSPI_HOST

    #else
    #define RCV_HOST    SPI2_HOST

    #endif
*/

// Note: We have a more special board with the Az Delivery D1 R32. See https://www.amazon.de/-/en/AZDelivery-NodeMCU-Development-Bluetooth-Compatible/dp/B08BTXPXBH
#define GPIO_HANDSHAKE 2
#define GPIO_MOSI GPIO_NUM_23
#define GPIO_MISO GPIO_NUM_19
#define GPIO_SCLK GPIO_NUM_18
#define GPIO_CS GPIO_NUM_5
#define RCV_HOST SPI2_HOST

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void my_post_setup_cb(spi_slave_transaction_t *trans) {
    WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1 << GPIO_HANDSHAKE));
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void my_post_trans_cb(spi_slave_transaction_t *trans) {
    WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1 << GPIO_HANDSHAKE));
}

void arduino_spi_slave() {
    // have to send on master in, *slave out*
    /*  pinMode(MISO, OUTPUT);

        // turn on SPI in slave mode
        SPCR |= bit(SPE);

        // turn on interrupts
        SPCR |= bit(SPIE);

        volatile char buf [20] = "Hello, world!";
        volatile int pos;
        volatile bool active;

        // SPI interrupt routine
        ISR (SPI_STC_vect) {
         byte c = SPDR;

         if (c == 1)      {
             // starting new sequence?
             active = true;
             pos = 0;
             SPDR = buf [pos++];   // send first byte
             return;
         }

         if (!active)    {
             SPDR = 0;
             return;
         }

         SPDR = buf [pos];
         if (buf [pos] == 0 || ++pos >= sizeof (buf)) {
             active = false;
         }
        }  // end of interrupt service routine (ISR) SPI_STC_vect
    */
}

SpiSlave::SpiSlave() {
    int n = 0;
    esp_err_t ret;

    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    //Configuration for the SPI slave interface
    // Order is important here for intializers...
    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = GPIO_CS,
        .flags = 0,
        .queue_size = 3,
        .mode = 0,
        .post_setup_cb = my_post_setup_cb,
        .post_trans_cb = my_post_trans_cb
    };

    //Configuration for the handshake line
    // Order is important here for intializers...
    gpio_config_t io_conf = {
        .pin_bit_mask = (1 << GPIO_HANDSHAKE),
        .mode = GPIO_MODE_OUTPUT,
        // TODO what does this meanfor powersave reasons, the GPIO should not be floating, select pullup
        .pull_up_en = GPIO_PULLUP_DISABLE  ,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    //Configure handshake line as output
    gpio_config(&io_conf);
    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    //Initialize SPI slave interface
    // From spi_common_dma_t --> we disable dma and limit transaction sizes to 64 bytes. We send byte by byte anyway
    // This can prohibit you from transmitting and receiving data longer than 64 bytes.
    ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, 0);
    assert(ret == ESP_OK);

    WORD_ALIGNED_ATTR char sendbuf[20] = "";
    WORD_ALIGNED_ATTR char recvbuf[20] = "";
    memset(recvbuf, 0, 33);
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));

    while (1) {
        //Clear receive buffer, set send buffer to something sane
        memset(recvbuf, 0xA5, 20);
        sprintf(sendbuf, "rec: %04d.", n);

        //Set up a transaction of 20 bytes to send/receive
        t.length = 20 * 8;
        t.tx_buffer = sendbuf;
        t.rx_buffer = recvbuf;
        /*  This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
            initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
            by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
            .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
            data.
        */
        ret = spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);

        //spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
        //received data from the master. Print it.
        Serial.printf("Received: %s\n", recvbuf);
        n++;
    }
}

SpiSlave::~SpiSlave() {
    //SPI.end();
}
