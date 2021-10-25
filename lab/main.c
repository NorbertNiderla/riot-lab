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

#include "board.h"
#include "periph/gpio.h"
#include "periph_conf.h"
#include "stm32l072xx.h"

static void user_button_callback(void* arg){

    uint32_t* gpio_a_odr = arg; 
    if(((*gpio_a_odr)&LED2_MASK) == 0)
        puts("LED2 blink!");
    LED2_TOGGLE;
    
}


int main(void)
{
    LED2_OFF;
    if (gpio_init_int(BTN_B1_PIN, GPIO_IN_PU, GPIO_BOTH, user_button_callback, (void *)GPIOA->ODR) < 0) {
        puts("[FAILED] init BTN1!");
        return 1;
    }

    return 0;
}
