/**
 * @file IOMatrixKeyboardReader.cpp
 * @author Forairaaaaa
 * @brief IO Matrix keyboard reader implementation
 * @version 0.1
 * @date 2023-09-22
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "IOMatrixKeyboardReader.h"
#include <driver/gpio.h>
#include "Arduino.h"

#define digitalWrite(pin, level) gpio_set_level((gpio_num_t)pin, level)
#define digitalRead(pin)         gpio_get_level((gpio_num_t)pin)

void IOMatrixKeyboardReader::_set_output(const std::vector<int>& pinList, uint8_t output)
{
    output = output & 0B00000111;

    digitalWrite(pinList[0], (output & 0B00000001));
    digitalWrite(pinList[1], (output & 0B00000010));
    digitalWrite(pinList[2], (output & 0B00000100));
}

uint8_t IOMatrixKeyboardReader::_get_input(const std::vector<int>& pinList)
{
    uint8_t buffer    = 0x00;
    uint8_t pin_value = 0x00;

    for (int i = 0; i < 7; i++) {
        pin_value = (digitalRead(pinList[i]) == 1) ? 0x00 : 0x01;
        pin_value = pin_value << i;
        buffer    = buffer | pin_value;
    }

    return buffer;
}

void IOMatrixKeyboardReader::begin()
{
    for (auto i : output_list) {
        gpio_reset_pin((gpio_num_t)i);
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_OUTPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_PULLUP_PULLDOWN);
        digitalWrite(i, 0);
    }

    for (auto i : input_list) {
        gpio_reset_pin((gpio_num_t)i);
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_PULLUP_ONLY);
    }

    _set_output(output_list, 0);
}

void IOMatrixKeyboardReader::updateKeyList(std::vector<Point2D_t>& keyList)
{
    keyList.clear();
    Point2D_t coor;
    uint8_t input_value = 0;

    for (int i = 0; i < 8; i++) {
        _set_output(output_list, i);
        input_value = _get_input(input_list);
        /* If key pressed */

        if (input_value) {
            /* Get X */
            for (int j = 0; j < 7; j++) {
                if (input_value & (0x01 << j)) {
                    coor.x = (i > 3) ? X_map_chart[j].x_1 : X_map_chart[j].x_2;

                    /* Get Y */
                    coor.y = (i > 3) ? (i - 4) : i;
                    // printf("%d,%d\t", coor.x, coor.y);

                    /* Keep the same as picture */
                    coor.y = -coor.y;
                    coor.y = coor.y + 3;

                    keyList.push_back(coor);
                }
            }
        }
    }
}
