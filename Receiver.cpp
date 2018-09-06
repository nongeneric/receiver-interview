#include "Receiver.h"
#include <algorithm>
#include <assert.h>

static constexpr char g_textEnding[] { '\r', '\n', '\r', '\n' };

Receiver::Receiver(ICallback* callback) : _callback(callback) {
    assert(callback);
}

void Receiver::receiveBinary(const char* data, unsigned int size) {
    while (size && _binaryHeaderLeft) {
        _binaryHeaderLeft--;
        _binaryLeft |= *data << (_binaryHeaderLeft * 8); // assuming big endian
        size--;
        data++;
    }

    if (_binaryHeaderLeft)
        return;

    auto toRead = std::min(size, _binaryLeft);
    std::copy(data, data + toRead, std::back_inserter(_packet));
    _binaryLeft -= toRead;

    if (_binaryLeft == 0) {
        _callback->BinaryPacket(&_packet[0], _packet.size());
        _packet.clear();

        size -= toRead;
        _activeStreamType = StreamType::None;
        Receive(data + toRead, size);
    }
}

void Receiver::receiveText(const char* data, unsigned int size) {
    while (size) {
        if (*data == g_textEnding[_textEndingSize]) {
            _textEndingSize++;
        } else {
            _textEndingSize = 0;
        }
        _packet.push_back(*data);
        data++;
        size--;
        if (_textEndingSize == std::size(g_textEnding)) {
            _callback->TextPacket(&_packet[0], _packet.size() - std::size(g_textEnding));
            _packet.clear();
            _textEndingSize = 0;
            _activeStreamType = StreamType::None;
            Receive(data, size);
            return;
        }
    }
}

void Receiver::Receive(const char *data, unsigned int size) {
    if (size == 0)
        return;

    if (_activeStreamType == StreamType::None) {
        if (*data == *BINARY_PREFIX) {
            _activeStreamType = StreamType::Binary;
            size--;
            data++;
            _bytesRead = 0;
            _binaryLeft = 0;
            _binaryHeaderLeft = sizeof(uint32_t);
        } else {
            _activeStreamType = StreamType::Text;
        }
    }

    if (_activeStreamType == StreamType::Binary) {
        receiveBinary(data, size);
    } else {
        receiveText(data, size);
    }
}
