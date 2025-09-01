/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "TCA8418.h"
#include "../../Adafruit_TCA8418/Adafruit_TCA8418_registers.h"
#include <Arduino.h>
#include <M5Unified.h>

// Default interrupt pin for M5Cardputer ADV
#define DEFAULT_TCA8418_INT_PIN 11

TCA8418KeyboardReader::TCA8418KeyboardReader(int interrupt_pin) : _interrupt_flag(false), _interrupt_pin(interrupt_pin)
{
    if (_interrupt_pin < 0) {
        _interrupt_pin = DEFAULT_TCA8418_INT_PIN;
    }
}

void IRAM_ATTR TCA8418KeyboardReader::_gpio_isr_handler(void* arg)
{
    TCA8418KeyboardReader* reader = static_cast<TCA8418KeyboardReader*>(arg);
    reader->_interrupt_flag       = true;
}

void TCA8418KeyboardReader::begin()
{
    // Initialize TCA8418
    _tca8418 = std::make_unique<Adafruit_TCA8418>();

    if (!_tca8418->begin()) {
        Serial.println("[ERROR] TCA8418KeyboardReader: Failed to initialize TCA8418");
        return;
    }

    // Configure 7x8 matrix (same as reference implementation)
    _tca8418->matrix(7, 8);
    _tca8418->flush();

    // Setup interrupt pin
    if (_interrupt_pin >= 0) {
        pinMode(_interrupt_pin, INPUT);
        attachInterruptArg(digitalPinToInterrupt(_interrupt_pin), _gpio_isr_handler, this, CHANGE);
    }

    // Enable interrupts
    _tca8418->enableInterrupts();

    Serial.println("[INFO] TCA8418KeyboardReader: Initialized successfully");
}

void TCA8418KeyboardReader::_remap_coordinates(uint8_t& row, uint8_t& col)
{
    // Remap to match the same coordinate system as IOMatrix
    // Based on the reference implementation's remap function
    uint8_t original_row = row;
    uint8_t original_col = col;

    // Col mapping
    col = original_row * 2;
    if (original_col > 3) {
        col++;
    }

    // Row mapping
    row = (original_col + 4) % 4;
}

void TCA8418KeyboardReader::_process_key_events()
{
    _key_list.clear();

    // Process all available events
    while (_tca8418->available()) {
        uint8_t event_raw = _tca8418->getEvent();

        // Parse event: bit 7 = state (0=press, 1=release), bits 0-6 = key code
        bool is_pressed   = !(event_raw & 0x80);  // Inverted: 0 means pressed
        uint16_t key_code = event_raw & 0x7F;

        if (key_code > 0) {
            key_code--;  // Convert to 0-based index

            // Convert key code to row/col
            uint8_t row = key_code / 10;
            uint8_t col = key_code % 10;

            // Remap coordinates to match IOMatrix layout
            _remap_coordinates(row, col);

            // Only add pressed keys to the list
            if (is_pressed) {
                Point2D_t key_pos;
                key_pos.x = col;
                key_pos.y = row;
                _key_list.push_back(key_pos);
            }
        }
    }
}

void TCA8418KeyboardReader::update()
{
    // Check if we have interrupt-based events to process
    if (_interrupt_flag) {
        _process_key_events();

        // Try to clear the interrupt flag
        // Based on reference implementation
        if (_tca8418) {
            // Clear interrupt status register
            _tca8418->writeRegister8(TCA8418_REG_INT_STAT, 1);

            // Check if interrupt is actually cleared (no more pending events)
            uint8_t int_stat = _tca8418->readRegister8(TCA8418_REG_INT_STAT);
            if ((int_stat & 0x01) == 0) {
                _interrupt_flag = false;
            }
        }
    }
}
