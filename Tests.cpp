#include "catch.hpp"
#include "Receiver.h"
#include <vector>
#include <string>

struct TestCallback : public ICallback {
    std::vector<std::string> binaries;
    std::vector<std::string> texts;

    virtual void BinaryPacket(const char *data, unsigned int size) override {
        binaries.emplace_back(data, size);
    }

    virtual void TextPacket(const char *data, unsigned int size) override {
        texts.emplace_back(data, size);
    }
};

char mixed_data[] = BINARY_PREFIX "\x00\x00\x00\x03" "abc" "text\r\n\r\n";

TEST_CASE("empty_chunk") {
    TestCallback callback;
    Receiver receiver(&callback);
    receiver.Receive(nullptr, 0);
    REQUIRE( callback.binaries.empty() );
    REQUIRE( callback.texts.empty() );
}

TEST_CASE("empty_binary") {
    TestCallback callback;
    Receiver receiver(&callback);
    receiver.Receive("$\x00\x00\x00\x00", 5);
    REQUIRE( callback.texts.empty() );
    REQUIRE( callback.binaries.size() == 1 );
    REQUIRE( callback.binaries[0] == "" );
}

TEST_CASE("basic_text") {
    TestCallback callback;
    Receiver receiver(&callback);
    std::string data = "simple text\r\n\r\n";
    receiver.Receive(&data[0], data.size());

    REQUIRE( callback.binaries.empty() );
    REQUIRE( callback.texts.size() == 1 );
    REQUIRE( callback.texts[0] == "simple text" );
}

TEST_CASE("basic_binary") {
    TestCallback callback;
    Receiver receiver(&callback);
    char data[] = BINARY_PREFIX "\x00\x00\x00\x03" "abc";
    receiver.Receive(&data[0], std::size(data));

    REQUIRE( callback.texts.empty() );
    REQUIRE( callback.binaries.size() == 1 );
    REQUIRE( callback.binaries[0] == "abc" );
}

TEST_CASE("basic_mixed") {
    TestCallback callback;
    Receiver receiver(&callback);
    receiver.Receive(&mixed_data[0], std::size(mixed_data));

    REQUIRE( callback.binaries.size() == 1 );
    REQUIRE( callback.texts.size() == 1 );
    REQUIRE( callback.binaries[0] == "abc" );
    REQUIRE( callback.texts[0] == "text" );
}

TEST_CASE("chunked_mixed") {
    TestCallback callback;
    Receiver receiver(&callback);

    for (auto ch : mixed_data) {
        receiver.Receive(&ch, 1);
    }

    REQUIRE( callback.binaries.size() == 1 );
    REQUIRE( callback.texts.size() == 1 );
    REQUIRE( callback.binaries[0] == "abc" );
    REQUIRE( callback.texts[0] == "text" );
}

TEST_CASE("complex_chunked_mixed") {
    TestCallback callback;
    Receiver receiver(&callback);

    char data[] = BINARY_PREFIX "\x00\x00\x00\x06" "binary"
                  "text1\r\n\r\n"
                  "text2\r\n\r\n"
                  BINARY_PREFIX "\x00\x00\x00\x07" "binary2"
                  "text3\r\n\r\r\n\r\n\r\n"
                  "padding";

    for (auto i = 0u; i < std::size(data); i += 3) {
        receiver.Receive(data + i, 3);
    }

    REQUIRE( callback.binaries.size() == 2 );
    REQUIRE( callback.binaries[0] == "binary" );
    REQUIRE( callback.binaries[1] == "binary2" );
    REQUIRE( callback.texts.size() == 3 );
    REQUIRE( callback.texts[0] == "text1" );
    REQUIRE( callback.texts[1] == "text2" );
    REQUIRE( callback.texts[2] == "text3\r\n\r\r\n" );
}

TEST_CASE("binary_prefix_inside_text") {
    TestCallback callback;
    Receiver receiver(&callback);
    std::string data = "text with $ in it\r\n\r\n";
    receiver.Receive(&data[0], data.size());

    REQUIRE( callback.binaries.empty() );
    REQUIRE( callback.texts.size() == 1 );
    REQUIRE( callback.texts[0] == "text with $ in it" );
}
