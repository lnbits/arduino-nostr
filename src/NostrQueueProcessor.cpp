#include "NostrQueueProcessor.h"
#include <cstring>

NostrQueueProcessor::NostrQueueProcessor() : data(), front(0), rear(-1), itemCount(0) {}

bool NostrQueueProcessor::isFull() const {
    return itemCount == MAX_QUEUE_SIZE;
}

bool NostrQueueProcessor::isEmpty() const {
    return itemCount == 0;
}

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

const char *NostrQueueProcessor::peekFront() const {
    return data[front].data();
}