/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>

struct Point2D_t {
    int x;
    int y;
};

/**
 * @brief Abstract interface for keyboard reading implementations
 */
class KeyboardReader {
public:
    virtual ~KeyboardReader() = default;

    /**
     * @brief Initialize the keyboard reader
     */
    virtual void begin() = 0;

    /**
     * @brief Update the list of currently pressed keys
     * @param keyList Reference to vector that will be filled with pressed key coordinates
     */
    virtual void updateKeyList(std::vector<Point2D_t>& keyList) = 0;
};
