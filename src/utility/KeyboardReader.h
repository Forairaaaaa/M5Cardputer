/**
 * @file KeyboardReader.h
 * @author Forairaaaaa
 * @brief Abstract keyboard reader interface
 * @version 0.1
 * @date 2023-09-22
 *
 * @copyright Copyright (c) 2023
 *
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
