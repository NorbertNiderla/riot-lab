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

#include "board.h" /* board specific definitions */
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

#define RED_PERIOD         (250000)
#define GREEN_PERIOD     (454321)

typedef enum {
    LED_BLINKING_TOGGLE = 0,
} msg_led_value;

static void user_button_callback(void *arg){
    (void) arg;
    LED2_TOGGLE;
    msg_t msg;
    msg.type = LED_BLINKING_TOGGLE;
    msg_send(&msg, *(kernel_pid_t*)arg);
}

char red_thread_stack[THREAD_STACKSIZE_MAIN];
char green_thread_stack[THREAD_STACKSIZE_MAIN];
static kernel_pid_t red_pid;
static kernel_pid_t green_pid;

void *thread_blinking_red(void* arg){
    (void) arg;

    DEBUG("thread_blinking_red: started\n");
    LED3_ON;
    msg_t msg;
    while(1){
        msg_receive(&msg);
        LED3_TOGGLE;
    }
    return NULL;
}

void *thread_blinking_green(void* arg){
    (void) arg;
    
    DEBUG("thread_blinking_green: started\n");
    xtimer_ticks32_t last_wakeup = xtimer_now();
    LED1_ON;
    
    while(1){
        LED1_TOGGLE;
        xtimer_periodic_wakeup(&last_wakeup, GREEN_PERIOD);
    }

    return NULL;
}

int main(void)
{
    red_pid = thread_create(red_thread_stack, sizeof(red_thread_stack),
                            THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
                            thread_blinking_red, NULL, "red");
    green_pid = thread_create(green_thread_stack, sizeof(green_thread_stack),
                            THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                            thread_blinking_green, NULL, "green");

    DEBUG("main: Red thread created: %d\n", red_pid);
    DEBUG("main: Green thread created: %d\n", green_pid);

    LED2_OFF;
    if (gpio_init_int(BTN_B1_PIN, GPIO_IN_PU, GPIO_BOTH, user_button_callback, (void*)&red_pid) < 0) {
        puts("[FAILED] init BTN1!");
        return 1;
    }
    
    return 0;
}
