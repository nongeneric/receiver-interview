#pragma once

#include "IReceiver.h"
#include <vector>
#include <cstdint>

enum class StreamType {
    None, Text, Binary
};

class Receiver : public IReceiver {
    ICallback* _callback;
    StreamType _activeStreamType = StreamType::None;
    std::vector<char> _packet;
    uint32_t _bytesRead = 0;
    uint32_t _binaryLeft = 0;
    uint32_t _binaryHeaderLeft = 0;
    uint32_t _textEndingSize = 0;

    void receiveBinary(const char* data, unsigned int size);
    void receiveText(const char* data, unsigned int size);

public:
    Receiver(Receiver const&) = delete;
    Receiver& operator==(Receiver const&) = delete;
    explicit Receiver(ICallback* callback);
    virtual void Receive(const char *data, unsigned int size) override;
};
