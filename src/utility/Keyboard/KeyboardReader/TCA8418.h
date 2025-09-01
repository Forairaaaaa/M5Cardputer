/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "KeyboardReader.h"
#include "../../Adafruit_TCA8418/Adafruit_TCA8418.h"
#include <Arduino.h>
#include <vector>
#include <memory>

/**
 * @brief TCA8418 I2C keyboard reader implementation
 */
class TCA8418KeyboardReader : public KeyboardReader {
public:
    TCA8418KeyboardReader(int interrupt_pin = -1);
    virtual ~TCA8418KeyboardReader() = default;

    void begin() override;
    void update() override;

private:
    std::unique_ptr<Adafruit_TCA8418> _tca8418;
    volatile bool _interrupt_flag;
    int _interrupt_pin;

    static void IRAM_ATTR _gpio_isr_handler(void* arg);
    void _remap_coordinates(uint8_t& row, uint8_t& col);
    void _process_key_events();
};
