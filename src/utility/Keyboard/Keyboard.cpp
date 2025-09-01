/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "Keyboard.h"
#include "../common.h"
#include "KeyboardReader/KeyboardReader.h"
#include "KeyboardReader/IOMatrix.h"
#include "KeyboardReader/TCA8418.h"
#include <Arduino.h>
#include <M5Unified.h>
#include <memory>

void Keyboard_Class::begin()
{
    // Reader injection based on board type
    auto board_type = M5.getBoard();
    if (board_type == m5::board_t::board_M5Cardputer) {
        _keyboard_reader = std::make_unique<IOMatrixKeyboardReader>();
    } else if (board_type == m5::board_t::board_M5CardputerADV) {
        _keyboard_reader = std::make_unique<TCA8418KeyboardReader>();
    } else {
        printf("[error] Keyboard: Unsupported board type: %d\n", (int)board_type);
        _keyboard_reader = std::make_unique<KeyboardReader>();
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
        _keyboard_reader->update();
    }
}

uint8_t Keyboard_Class::isPressed()
{
    return keyList().size();
}

bool Keyboard_Class::isChange()
{
    uint8_t current_size = keyList().size();
    if (_last_key_size != current_size) {
        _last_key_size = current_size;
        return true;
    } else {
        return false;
    }
}

bool Keyboard_Class::isKeyPressed(char c)
{
    const auto& keys = keyList();
    if (keys.size()) {
        for (const auto& i : keys) {
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

    const auto& keys = keyList();

    // 预分配容器大小以避免重复分配
    const size_t key_count = keys.size();
    if (key_count > 0) {
        _key_pos_print_keys.reserve(key_count);
        _key_pos_hid_keys.reserve(key_count);
        _key_pos_modifier_keys.reserve(8);
        _keys_state_buffer.modifier_keys.reserve(8);
        _keys_state_buffer.hid_keys.reserve(key_count);
        _keys_state_buffer.word.reserve(key_count);
    }

    for (const auto& key_pos : keys) {
        const KeyValue_t key_value = getKeyValue(key_pos);
        const uint8_t key_code     = key_value.value_first;

        switch (key_code) {
            // 修饰键处理
            case KEY_FN:
                _keys_state_buffer.fn = true;
                continue;

            case KEY_OPT:
                _keys_state_buffer.opt = true;
                continue;

            case KEY_LEFT_CTRL:
                _keys_state_buffer.ctrl = true;
                _key_pos_modifier_keys.push_back(key_pos);
                _keys_state_buffer.modifier_keys.push_back(key_code);
                _keys_state_buffer.modifiers |= (1 << (key_code - 0x80));
                continue;

            case KEY_LEFT_SHIFT:
                _keys_state_buffer.shift = true;
                _key_pos_modifier_keys.push_back(key_pos);
                _keys_state_buffer.modifier_keys.push_back(key_code);
                _keys_state_buffer.modifiers |= (1 << (key_code - 0x80));
                continue;

            case KEY_LEFT_ALT:
                _keys_state_buffer.alt = true;
                _key_pos_modifier_keys.push_back(key_pos);
                _keys_state_buffer.modifier_keys.push_back(key_code);
                _keys_state_buffer.modifiers |= (1 << (key_code - 0x80));
                continue;

            // 功能键处理
            case KEY_TAB:
                _keys_state_buffer.tab = true;
                _key_pos_hid_keys.push_back(key_pos);
                _keys_state_buffer.hid_keys.push_back(key_code);
                continue;

            case KEY_BACKSPACE:
                _keys_state_buffer.del = true;
                _key_pos_hid_keys.push_back(key_pos);
                _keys_state_buffer.hid_keys.push_back(key_code);
                continue;

            case KEY_ENTER:
                _keys_state_buffer.enter = true;
                _key_pos_hid_keys.push_back(key_pos);
                _keys_state_buffer.hid_keys.push_back(key_code);
                continue;

            case ' ':
                _keys_state_buffer.space = true;
                break;  // 空格键需要继续后续处理

            default:
                break;  // 普通按键继续后续处理
        }

        // 处理普通按键和空格键
        _key_pos_hid_keys.push_back(key_pos);
        _key_pos_print_keys.push_back(key_pos);

        // 直接处理HID键值转换
        if (key_code != ' ') {  // 空格已在上面处理
            const uint8_t hid_key = _kb_asciimap[key_code];
            if (hid_key) {
                _keys_state_buffer.hid_keys.push_back(hid_key);
            }
        }

        // 直接处理字符输出
        if (_keys_state_buffer.ctrl || _keys_state_buffer.shift || _is_caps_locked) {
            _keys_state_buffer.word.push_back(key_value.value_second);
        } else {
            _keys_state_buffer.word.push_back(key_value.value_first);
        }
    }
}
