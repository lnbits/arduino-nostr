/*
  NostrQueueProcessor - Arduino library for Nostr.
  Created by Black Coffee <bc@omg.lol>, March 29th 2023
  Released under MIT License
*/
#include "NostrQueueProcessor.h"
#include <cstring>

/**
 * @brief Construct a new Nostr Queue Processor:: Nostr Queue Processor object
 * 
 */
NostrQueueProcessor::NostrQueueProcessor() : data(), front(0), rear(-1), itemCount(0) {}

/**
 * @brief Is the message queue full?
 * 
 * @return true 
 * @return false 
 */
bool NostrQueueProcessor::isFull() const {
    return itemCount == MAX_QUEUE_SIZE;
}

/**
 * @brief Is the message queue empty?
 * 
 * @return true 
 * @return false 
 */
bool NostrQueueProcessor::isEmpty() const {
    return itemCount == 0;
}

/**
 * @brief Add a message to the queue
 * 
 * @param item 
 */
void NostrQueueProcessor::enqueue(const char item[MAX_ITEM_SIZE]) {
    if (!isFull()) {
        if (rear == MAX_QUEUE_SIZE - 1) {
            rear = -1;
        }

        std::strncpy(data[++rear].data(), item, MAX_ITEM_SIZE - 1);
        data[rear][MAX_ITEM_SIZE - 1] = '\0';
        itemCount++;
    }
}

/**
 * @brief Remove a message from the queue
 * 
 * @return const char* 
 */
const char *NostrQueueProcessor::dequeue() {
    if (!isEmpty()) {
        const char *item = data[front].data();
        front++;
        if (front == MAX_QUEUE_SIZE) {
            front = 0;
        }

        itemCount--;
        return item;
    }
    return nullptr; // Indicate an error (empty queue)
}

/**
 * @brief Get the first item in the queue without removing it
 * 
 * @return const char* 
 */
const char *NostrQueueProcessor::peekFront() const {
    return data[front].data();
}