/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "Keyboard.h"
#include "KeyboardReader/KeyboardReader.h"
#include "KeyboardReader/IOMatrix.h"
#include <Arduino.h>
#include <M5Unified.h>
#include <memory>

void Keyboard_Class::begin()
{
    // Reader injection
    auto board_type = M5.getBoard();
    if (board_type == m5::board_t::board_M5Cardputer) {
        _keyboard_reader = std::make_unique<IOMatrixKeyboardReader>();
    } else if (board_type == m5::board_t::board_M5CardputerADV) {
        // TODO
    } else {
        printf("[error] Keyboard: Unsupported board type: %d\n", board_type);
        return;
    }
    _keyboard_reader->begin();
}

void Keyboard_Class::begin(std::unique_ptr<KeyboardReader> reader)
{
    _keyboard_reader = std::move(reader);
    _keyboard_reader->begin();
}

uint8_t Keyboard_Class::getKey(Point2D_t keyCoor)
{
    uint8_t ret = 0;

    if ((keyCoor.x < 0) || (keyCoor.y < 0)) {
        return 0;
    }
    if (_keys_state_buffer.ctrl || _keys_state_buffer.shift || _is_caps_locked) {
        ret = _key_value_map[keyCoor.y][keyCoor.x].value_second;
    } else {
        ret = _key_value_map[keyCoor.y][keyCoor.x].value_first;
    }
    return ret;
}

void Keyboard_Class::updateKeyList()
{
    if (_keyboard_reader) {
        _keyboard_reader->updateKeyList(_key_list_buffer);
    }
}

uint8_t Keyboard_Class::isPressed()
{
    return _key_list_buffer.size();
}

bool Keyboard_Class::isChange()
{
    if (_last_key_size != _key_list_buffer.size()) {
        _last_key_size = _key_list_buffer.size();
        return true;
    } else {
        return false;
    }
}

bool Keyboard_Class::isKeyPressed(char c)
{
    if (_key_list_buffer.size()) {
        for (const auto& i : _key_list_buffer) {
            if (getKey(i) == c) return true;
        }
    }
    return false;
}

#include <cstring>

void Keyboard_Class::updateKeysState()
{
    _keys_state_buffer.reset();
    _key_pos_print_keys.clear();
    _key_pos_hid_keys.clear();
    _key_pos_modifier_keys.clear();

    // Get special keys
    for (auto& i : _key_list_buffer) {
        // modifier
        if (getKeyValue(i).value_first == KEY_FN) {
            _keys_state_buffer.fn = true;
            continue;
        }
        if (getKeyValue(i).value_first == KEY_OPT) {
            _keys_state_buffer.opt = true;
            continue;
        }

        if (getKeyValue(i).value_first == KEY_LEFT_CTRL) {
            _keys_state_buffer.ctrl = true;
            _key_pos_modifier_keys.push_back(i);
            continue;
        }

        if (getKeyValue(i).value_first == KEY_LEFT_SHIFT) {
            _keys_state_buffer.shift = true;
            _key_pos_modifier_keys.push_back(i);
            continue;
        }

        if (getKeyValue(i).value_first == KEY_LEFT_ALT) {
            _keys_state_buffer.alt = true;
            _key_pos_modifier_keys.push_back(i);
            continue;
        }

        // function
        if (getKeyValue(i).value_first == KEY_TAB) {
            _keys_state_buffer.tab = true;
            _key_pos_hid_keys.push_back(i);
            continue;
        }

        if (getKeyValue(i).value_first == KEY_BACKSPACE) {
            _keys_state_buffer.del = true;
            _key_pos_hid_keys.push_back(i);
            continue;
        }

        if (getKeyValue(i).value_first == KEY_ENTER) {
            _keys_state_buffer.enter = true;
            _key_pos_hid_keys.push_back(i);
            continue;
        }

        if (getKeyValue(i).value_first == ' ') {
            _keys_state_buffer.space = true;
        }
        _key_pos_hid_keys.push_back(i);
        _key_pos_print_keys.push_back(i);
    }

    for (auto& i : _key_pos_modifier_keys) {
        uint8_t key = getKeyValue(i).value_first;
        _keys_state_buffer.modifier_keys.push_back(key);
    }

    for (auto& k : _keys_state_buffer.modifier_keys) {
        _keys_state_buffer.modifiers |= (1 << (k - 0x80));
    }

    for (auto& i : _key_pos_hid_keys) {
        uint8_t k = getKeyValue(i).value_first;
        if (k == KEY_TAB || k == KEY_BACKSPACE || k == KEY_ENTER) {
            _keys_state_buffer.hid_keys.push_back(k);
            continue;
        }
        uint8_t key = _kb_asciimap[k];
        if (key) {
            _keys_state_buffer.hid_keys.push_back(key);
        }
    }

    // Deal what left
    for (auto& i : _key_pos_print_keys) {
        if (_keys_state_buffer.ctrl || _keys_state_buffer.shift || _is_caps_locked) {
            _keys_state_buffer.word.push_back(getKeyValue(i).value_second);
        } else {
            _keys_state_buffer.word.push_back(getKeyValue(i).value_first);
        }
    }
}
