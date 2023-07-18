/*
  NostrQueueProcessor - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#pragma once
#include <array>
#include <string>

class NostrQueueProcessor {
public:
    static const size_t MAX_QUEUE_SIZE = 5;
    static const size_t MAX_ITEM_SIZE = 2048;

    NostrQueueProcessor();

    bool isFull() const;
    bool isEmpty() const;
    void enqueue(const char item[MAX_ITEM_SIZE]);
    const char *dequeue();
    const char *peekFront() const;

private:
    std::array<std::array<char, MAX_ITEM_SIZE>, MAX_QUEUE_SIZE> data;
    int front;
    int rear;
    size_t itemCount;
};