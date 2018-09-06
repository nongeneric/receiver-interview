#pragma once

#define BINARY_PREFIX "$"

struct IReceiver {
    virtual void Receive(const char* data, unsigned int size) = 0;
    ~IReceiver() = default;
};

struct ICallback {
    virtual void BinaryPacket(const char* data, unsigned int size) = 0;
    virtual void TextPacket(const char* data, unsigned int size) = 0;
    ~ICallback() = default;
};
