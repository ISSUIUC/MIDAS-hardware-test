#pragma once

#include <Arduino.h>
#include "pins.h"
#include "driver/gpio.h"
#include "driver/twai.h"

class CANTWAI {
public:
    void init();
    int send(twai_message_t& msg);

    int recv(twai_message_t* msg);
};

void CANTWAI::init() {

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_1, (gpio_num_t)CAN_2, TWAI_MODE_NO_ACK);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    int r = twai_driver_install(&g_config, &t_config, &f_config);
    if(r == ESP_OK) {
        Serial.println("TWAI driver startup ok");
    } else {
        Serial.println("IM FUCKING KILLING MYSELF!!!");
        Serial.print("CAN Error: ");
        Serial.println(r, HEX);
        return;
    }

    if(twai_start() == ESP_OK) {
        Serial.println("TWAI start ok");
    } else {
        Serial.println("DIEEEEEEEEE");
        return;
    }
}

int CANTWAI::send(twai_message_t& msg) {
    int res = twai_transmit(&msg, pdMS_TO_TICKS(1000));
    if(res == ESP_OK) {
        return 1;
    }
    Serial.print("tx err: 0x");
    Serial.println(res, HEX);
    return 0;
}

int CANTWAI::recv(twai_message_t* msg) {
    int res = twai_receive(msg, pdMS_TO_TICKS(1000));
    if(res == ESP_OK) {
        return 1;
    } 
    Serial.print("rx err: 0x");
    Serial.println(res, HEX);
    return 0;
}