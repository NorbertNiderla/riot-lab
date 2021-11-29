/**
 * @ingroup     lab
 * @{
 *
 * @file
 * @brief       LoRaWAN excercise
 *
 * @author      Norbert Niderla <norbert.niderla@gmail.com>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "msg.h"
#include "thread.h"
#include "fmt.h"

#include "periph/rtc.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

#include "sx127x.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"

#define RECEIVER_PRIO         (THREAD_PRIORITY_MAIN - 1)
static kernel_pid_t sender_pid;
static char receiver_stack[THREAD_STACKSIZE_MAIN / 2];

static semtech_loramac_t loramac;
static sx127x_t sx127x;

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

static void _receive_message(void)
{
    uint8_t ret = semtech_loramac_recv(&loramac);
    if (ret == 9)  {
        loramac.rx_data.payload[loramac.rx_data.payload_len] = 0;
        printf("Received: %s\n", loramac.rx_data.payload);
        return;
    } else if(ret == 10){
        return;
    } else if(ret == 11){
        return;
    } else {
        printf("Receive error\n");
        return;
    }
}

static void *receiver(void *arg)
{
    (void)arg;
    while (1) {
        _receive_message();
    }

    /* this should never be reached */
    return NULL;
}

int main(void)
{
    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, CONFIG_LORAMAC_DEV_EUI_DEFAULT);
    fmt_hex_bytes(appeui, CONFIG_LORAMAC_APP_EUI_DEFAULT);
    fmt_hex_bytes(appkey, CONFIG_LORAMAC_APP_KEY_DEFAULT);

    /* Initialize the radio driver */
    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        return 1;
    }
    puts("Join procedure succeeded");

    char init_msg[] = "init_message";
    uint8_t ret = semtech_loramac_send(&loramac, (uint8_t *)init_msg, strlen(init_msg));
    while (ret != SEMTECH_LORAMAC_TX_DONE)  {
        printf("Cannot send message '%s', ret code: %d\n", message, ret);
        printf("Retrying...\n")
        ret = semtech_loramac_send(&loramac, (uint8_t *)init_msg, strlen(init_msg));
    }
    printf("init message sent\n");

    /* start the receiver thread */
    sender_pid = thread_create(receiver_stack, sizeof(receiver_stack),
                               RECEIVER_PRIO, 0, receiver, NULL, "receiver");
    return 0;
}
