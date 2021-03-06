#pragma once

#include "IReceiver.h"
#include <vector>
#include <cstdint>

enum class StreamType {
    None, Text, Binary
};

class Receiver : public IReceiver {
    ICallback* _callback;
    StreamType _activeStreamType;
    std::vector<char> _packet;
    uint32_t _bytesRead;
    uint32_t _binaryLeft;
    uint32_t _binaryHeaderLeft;

    void reset();
    const char* receiveBinary(const char* data, unsigned int& size);
    const char* receiveText(const char* data, unsigned int& size);

public:
    Receiver(Receiver const&) = delete;
    Receiver& operator==(Receiver const&) = delete;
    explicit Receiver(ICallback* callback);
    virtual void Receive(const char *data, unsigned int size) override;
};
