#include "Receiver.h"
#include <algorithm>
#include <assert.h>

static constexpr char g_textEnding[] { '\r', '\n', '\r', '\n' };

Receiver::Receiver(ICallback* callback) : _callback(callback) {
    assert(callback);
    reset();
}

void Receiver::reset() {
    _activeStreamType = StreamType::None;
    _packet.clear();
    _bytesRead = 0;
    _binaryLeft = 0;
    _binaryHeaderLeft = sizeof(uint32_t);
}

const char* Receiver::receiveBinary(const char* data, unsigned int& size) {
    while (size && _binaryHeaderLeft) {
        _binaryHeaderLeft--;
        _binaryLeft |= *data << (_binaryHeaderLeft * 8); // assuming big endian
        size--;
        data++;
    }

    if (_binaryHeaderLeft)
        return data;

    auto toRead = std::min(size, _binaryLeft);
    _packet.resize(_packet.size() + toRead);
    std::copy(data, data + toRead, end(_packet) - toRead);
    _binaryLeft -= toRead;
    size -= toRead;
    data += toRead;
    if (_binaryLeft == 0) {
        _callback->BinaryPacket(_packet.data(), _packet.size());
        reset();
        return data;
    }

    return data;
}

const char* Receiver::receiveText(const char* data, unsigned int& size) {
    while (size) {
        auto ch = *data;
        _packet.push_back(ch);
        size--;
        data++;
        if (ch == g_textEnding[0] || ch == g_textEnding[1]) {
            if (_packet.size() >= std::size(g_textEnding) &&
                std::equal(std::begin(g_textEnding),
                           std::end(g_textEnding),
                           end(_packet) - std::size(g_textEnding))) {
                _callback->TextPacket(&_packet[0], _packet.size() - std::size(g_textEnding));
                reset();
                return data;
            }
        }
    }
    return data;
}

void Receiver::Receive(const char *data, unsigned int size) {
    while (size) {
        if (_activeStreamType == StreamType::None) {
            if (*data == *BINARY_PREFIX) {
                _activeStreamType = StreamType::Binary;
                size--;
                data++;
            } else {
                _activeStreamType = StreamType::Text;
            }
        }

        if (_activeStreamType == StreamType::Binary) {
            data = receiveBinary(data, size);
        } else {
            data = receiveText(data, size);
        }
    }
}
