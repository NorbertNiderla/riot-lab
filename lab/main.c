/*
 * Copyright (C) 2021 Warsaw University of Technology
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lab
 * @{
 *
 * @file
 * @brief       mcu programming (RIOT OS) - threading and irq
 *
 * @author      Norbert Niderla <norbert.niderla@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "board.h"       /* board specific definitions */
#include "periph/gpio.h" /* gpio api */
#include "stm32l072xx.h" /* mcu specific definitions */

/* threading includes */
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
/***/

#define ENABLE_DEBUG    (1)
#if ENABLE_DEBUG
#include "debug.h"
#endif

/* button manipulation macro */
#define USER_BUTTON       (BTN_B1_PIN)
/* led manipulation macros */
#define RED_LED_OFF       (LED3_OFF)
#define RED_LED_ON        (LED3_ON)
#define RED_LED_TOGGLE    (LED3_TOGGLE)
#define BLUE_LED_OFF      (LED2_OFF)
#define BLUE_LED_ON       (LED2_ON)
#define BLUE_LED_TOGGLE   (LED2_TOGGLE)
#define GREEN_LED_OFF     (LED1_OFF)
#define GREEN_LED_ON      (LED1_ON)
#define GREEN_LED_TOGGLE  (LED1_TOGGLE)

#define RED_LED_PERIOD         (250000)
#define GREEN_LED_PERIOD     (454321)

typedef enum {
    LED_BLINKING_TOGGLE = 0,
} msg_led_type;

static void user_button_callback(void *arg){
    (void) arg;
    msg_t msg;
    msg.type = LED_BLINKING_TOGGLE;
    static unsigned button_pressed = 0;
    static xtimer_ticks32_t start;
    xtimer_ticks32_t stop;

    if(button_pressed == 0){
        start = xtimer_now();
        button_pressed = 1;
    } else {
        stop = xtimer_now();
        button_pressed = 0;
        msg.content.value = xtimer_diff(stop, start).ticks32 >> 20;
        msg_send(&msg, *(kernel_pid_t*)arg);
    }
}

char red_thread_stack[THREAD_STACKSIZE_MAIN];

void *thread_blinking_red(void* arg){
    (void)arg;
    msg_t msg;
    unsigned counter;
    xtimer_ticks32_t last_wakeup;
    while(1){
        msg_receive(&msg);
        if(msg.type == LED_BLINKING_TOGGLE){
            counter = msg.content.value;
            last_wakeup = xtimer_now();
            while(counter!=0){
                RED_LED_TOGGLE;
                xtimer_periodic_wakeup(&last_wakeup, RED_LED_PERIOD);
                RED_LED_TOGGLE;
                xtimer_periodic_wakeup(&last_wakeup, RED_LED_PERIOD);
                counter--;
            }
        }
    }
    return NULL;
}

/* TODO: */
/* green led thread */
/* user button callback drukowanie ile czasu minęło */
/* globalna zmienna drukowana w pierwszym wątku */
/* red thread receiving message */
/* jakos rozdzielic mainy tak, żeby kolejne częsci mozna było zaczynać niezależnie */

int main(void)
{
    kernel_pid_t red_pid = thread_create(red_thread_stack, sizeof(red_thread_stack),
                            THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                            thread_blinking_red, NULL, "red");

    gpio_init_int(USER_BUTTON, GPIO_IN_PU, GPIO_BOTH, user_button_callback, (void*)&red_pid);
     
    xtimer_ticks32_t last_wakeup = xtimer_now();
    GREEN_LED_ON;
    while(1){
        GREEN_LED_TOGGLE;
        xtimer_periodic_wakeup(&last_wakeup, GREEN_LED_PERIOD);
    }

    /*if code get here, red_pid won't be passed to user_button_callback */
    return 0;
}
