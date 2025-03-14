#include "../tftp/details/packets.hpp"
#include <gtest/gtest.h>

using namespace tftp::packets;

#define EXPECT_DATA(Buffer, BaseOffset, Data)                                                                          \
    {                                                                                                                  \
        for (std::size_t Idx_ = 0, Size = Data.size(); Idx_ != Size; ++Idx_) {                                         \
            EXPECT_EQ(Buffer[BaseOffset + Idx_], Data[Idx_]);                                                          \
        }                                                                                                              \
    }

#define EXPECT_STRING(Buffer, BaseOffset, String)                                                                      \
    {                                                                                                                  \
        EXPECT_DATA(Buffer, BaseOffset, String);                                                                       \
        EXPECT_EQ(Buffer[BaseOffset + String.size()], 0x00);                                                           \
    }

/// Test that Request packet serialization is going fine and everything is converting to network byte order
TEST(Request, Serialization) {
    std::string Filename = "example_filename.cpp";
    std::string Mode = "netascii";
    auto Packet = Request{types::ReadRequest, Filename, Mode};

    std::vector<std::uint8_t> Buffer;
    auto PacketSize = Packet.serialize(std::back_inserter(Buffer));
    EXPECT_EQ(PacketSize, sizeof(std::uint16_t) + Filename.size() + Mode.size() + 2);

    // type field
    // check that conversion to big-endian was done
    EXPECT_EQ(Buffer[0], 0x00);
    EXPECT_EQ(Buffer[1], 0x01);

    // filename field
    std::size_t BaseOffset = 2;
    EXPECT_STRING(Buffer, BaseOffset, Filename);

    // mode field
    BaseOffset += Filename.size() + 1;
    EXPECT_STRING(Buffer, BaseOffset, Mode);

    EXPECT_EQ(Buffer.size(), PacketSize);
}

/// Test that Request packet with options serialization is going fine and everything is converting to network byte order
TEST(Request, OptionSerialization) {
    std::string Filename = "example_filename.cpp";
    std::string Mode = "netascii";
    std::vector<std::string> OptionsNames = {"saveFiles", "discardQualifiers", "secret"};
    std::vector<std::string> OptionsValues = {"true", "false", "Ix0e86yG8YpFzwz1gS0XxJW3"};
    auto Packet = Request{types::ReadRequest, Filename, Mode, OptionsNames, OptionsValues};

    std::size_t OptionsSize = 0;
    for (std::size_t Idx = 0; Idx != OptionsNames.size(); ++Idx) {
        OptionsSize += OptionsNames[Idx].size() + OptionsValues[Idx].size() + 2;
    }

    std::vector<std::uint8_t> Buffer;
    auto PacketSize = Packet.serialize(std::back_inserter(Buffer));
    EXPECT_EQ(PacketSize, sizeof(std::uint16_t) + Filename.size() + Mode.size() + OptionsSize + 2);

    // type field
    EXPECT_EQ(Buffer[0], 0x00);
    EXPECT_EQ(Buffer[1], 0x01);
    std::size_t BaseOffset = 2;

    // filename field
    EXPECT_STRING(Buffer, BaseOffset, Filename);
    BaseOffset += Filename.size() + 1;

    // mode field
    EXPECT_STRING(Buffer, BaseOffset, Mode);
    BaseOffset += Mode.size() + 1;

    // option names and values
    for (std::size_t Idx = 0; Idx != OptionsNames.size(); ++Idx) {
        EXPECT_STRING(Buffer, BaseOffset, OptionsNames[Idx]);
        BaseOffset += OptionsNames[Idx].size() + 1;
        EXPECT_STRING(Buffer, BaseOffset, OptionsValues[Idx]);
        BaseOffset += OptionsValues[Idx].size() + 1;
    }

    EXPECT_EQ(Buffer.size(), PacketSize);
}

/// Test that Data packet serialization is going fine and everything is converting to network byte order
TEST(Data, Serialization) {
    std::vector<std::uint8_t> DataBuffer;
    for (auto I = 0; I != 255; ++I) {
        DataBuffer.push_back(I);
    }
    auto Packet = Data{static_cast<std::uint16_t>(DataBuffer.size()), DataBuffer};

    std::vector<std::uint8_t> Buffer;
    auto PacketSize = Packet.serialize(std::back_inserter(Buffer));
    EXPECT_EQ(PacketSize, sizeof(std::uint16_t) + sizeof(std::uint16_t) + DataBuffer.size());

    // type field
    EXPECT_EQ(Buffer[0], 0x00);
    EXPECT_EQ(Buffer[1], 0x03);

    // block field
    EXPECT_EQ(Buffer[2], 0x00);
    EXPECT_EQ(Buffer[3], 0xFF);

    // data field
    EXPECT_DATA(Buffer, 4, DataBuffer);

    EXPECT_EQ(Buffer.size(), PacketSize);
}

/// Test that Acknowledgment packet serialization is going fine and everything is converting to network byte order
TEST(Acknowledgment, Serialization) {
    auto Packet = Acknowledgment{255};

    std::vector<std::uint8_t> Buffer;
    auto PacketSize = Packet.serialize(std::back_inserter(Buffer));
    EXPECT_EQ(PacketSize, sizeof(std::uint16_t) + sizeof(std::uint16_t));

    // type field
    EXPECT_EQ(Buffer[0], 0x00);
    EXPECT_EQ(Buffer[1], 0x04);

    // block field
    EXPECT_EQ(Buffer[2], 0x00);
    EXPECT_EQ(Buffer[3], 0xFF);

    EXPECT_EQ(Buffer.size(), PacketSize);
}

/// Test that Error packet serialization is going fine and everything is converting to network byte order
TEST(Error, Serialization) {
    std::string ErrorMessage = "Something went wrong...";
    auto Packet = Error{0x01, ErrorMessage};

    std::vector<std::uint8_t> Buffer;
    auto PacketSize = Packet.serialize(std::back_inserter(Buffer));
    EXPECT_EQ(PacketSize, sizeof(std::uint16_t) + sizeof(std::uint16_t) + ErrorMessage.size() + 1);

    // type field
    EXPECT_EQ(Buffer[0], 0x00);
    EXPECT_EQ(Buffer[1], 0x05);

    // errorCode field
    EXPECT_EQ(Buffer[2], 0x00);
    EXPECT_EQ(Buffer[3], 0x01);

    // errorMessage field
    EXPECT_DATA(Buffer, 4, ErrorMessage);

    EXPECT_EQ(Buffer.size(), PacketSize);
}

/// Test that Option Acknowledgment packet serialization is going fine and everything is converting to network byte
/// order
TEST(OptionAcknowledgment, Serialization) {
    std::unordered_map<std::string, std::string> Options = {
        {"saveFiles", "true"}, {"discardQualifiers", "false"}, {"secret", "Ix0e86yG8YpFzwz1gS0XxJW3"}};
    auto Packet = OptionAcknowledgment{Options};

    std::size_t OptionsSize = 0;
    for (auto It = Options.cbegin(), End = Options.cend(); It != End; ++It) {
        OptionsSize += It->first.size() + It->second.size() + 2;
    }

    std::vector<std::uint8_t> Buffer;
    auto PacketSize = Packet.serialize(std::back_inserter(Buffer));
    EXPECT_EQ(PacketSize, sizeof(std::uint16_t) + OptionsSize);

    std::size_t BaseOffset = sizeof(std::uint16_t);
    // option names and values
    for (const auto &[Key, Value] : Options) {
        EXPECT_STRING(Buffer, BaseOffset, Key);
        BaseOffset += Key.size() + 1;
        EXPECT_STRING(Buffer, BaseOffset, Value);
        BaseOffset += Value.size() + 1;
    }

    EXPECT_EQ(Buffer.size(), PacketSize);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}