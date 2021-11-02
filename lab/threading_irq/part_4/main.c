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

/* leds period times (can be changed) */
#define RED_LED_PERIOD         (250000)
#define GREEN_LED_PERIOD     (250000)
#define BLUE_LED_PERIOD     (250000)

char stack_thread_led[THREAD_STACKSIZE_MAIN];
kernel_pid_t red_pid;

void *thread_red(void* arg){
    /* tutaj napisz działanie wątku odbierającego wiadomości
    o czasie przytrzymania przycisku */
    /* funkcje: msg_receive */
    /* struktury: msg_t */
}

static unsigned user_button_pressed_time = 0;

static void user_button_callback(void *arg){
    /* tak zmodyfikuj tę funkcję, żeby czas przytrzymania przycisku
    przesyłała do czerwonego wątku */
    /* funkcje: msg_send */
    /* struktury: msg_t */
    unsigned* pressed_time = (unsigned*)arg;
    static unsigned button_pressed = 0;
    static xtimer_ticks32_t start;
    xtimer_ticks32_t stop;

    if(button_pressed == 0){
        start = xtimer_now();
        button_pressed = 1;
    } else {
        stop = xtimer_now();
        *pressed_time = xtimer_diff(stop, start).ticks32;
        button_pressed = 0;
    }
}

char stack_thread_blinking_green[THREAD_STACKSIZE_MAIN];

void *thread_blinking_green(void* arg){
    /*z tego wątku usuń funkcjonalności związane z drukowaniem
    czasu przytrzymania przycisku */
    unsigned* pressed_time = (unsigned*)arg;
    unsigned last_pressed_time = 0;
    xtimer_ticks32_t last_wakeup = xtimer_now();
    GREEN_LED_ON;
    while(1){
        GREEN_LED_TOGGLE;
        xtimer_periodic_wakeup(&last_wakeup, GREEN_LED_PERIOD);
        if(last_pressed_time != *pressed_time){
            last_pressed_time = *pressed_time;
            DEBUG("User button pressed time[us]: %d\n", last_pressed_time);
        }
    }    
}

int main(void)
{
    kernel_pid_t green_pid = thread_create(stack_thread_blinking_green, sizeof(stack_thread_blinking_green),
                            THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                            thread_blinking_green, (void*)&user_button_pressed_time, "green");

    /* tutaj napisz uruchomienie czerwonego wątku i przekazanie pid do obsługi przerwania */

    gpio_init_int(USER_BUTTON, GPIO_IN_PU, GPIO_BOTH, user_button_callback, (void*)&user_button_pressed_time);

    /* jeśli wykonanie kodu dotrze do tego miejsca,
    zmienne zdefiniowane w main() przestaną być dostępne */
    return 0;
}
