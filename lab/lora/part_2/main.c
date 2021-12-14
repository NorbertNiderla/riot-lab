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

#define ENABLE_DEBUG    (1)
#if ENABLE_DEBUG
#include "debug.h"
#endif

#define RECEIVER_PRIO         (THREAD_PRIORITY_MAIN - 1)
static kernel_pid_t sender_pid;
static char receiver_stack[THREAD_STACKSIZE_MAIN / 2];

static semtech_loramac_t loramac;
static sx127x_t sx127x;

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];

#define RECV_MSG_QUEUE                   (4U)
static msg_t _recv_queue[RECV_MSG_QUEUE];

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
    semtech_loramac_set_rx2_freq(&loramac, 869525000);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
     DEBUG("Starting join procedure\n");
    while (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        DEBUG("Join procedure failed, retrying in 30s\n");
        xtimer_ticks32 last_wakeup = xtimer_now();
        xtimer_periodic_wakeup(&last_wakeup, 30000000);
    }
    DEBUG("Join procedure succeeded\n");

    char keep_alive_message[] = "keep alive";
    while(1){
        uint8_t ret = semtech_loramac_send(&loramac, (uint8_t *)keep_alive_message, strlen(keep_alive_message));
        if(ret != SEMTECH_LORAMAC_TX_OK){
            DEBUG("keep alive not sent\n");
        } else {
            DEBUG("keep alive sent\n");
        }

        xtimer_ticks32 last_wakeup = xtimer_now();
        xtimer_periodic_wakeup(&last_wakeup, 90000000);
    }


    
    return 0;
}
