#include "../tftp/details/parsers.hpp"
#include <gtest/gtest.h>

using namespace tftp::packets;

/// Test that Request packet parsing is going fine
TEST(Request, Parse) {
    std::uint8_t PacketBytes[] = {// type
                                  0x00, 0x01,
                                  // filename
                                  0x2f, 0x73, 0x72, 0x76, 0x2f, 0x74, 0x66, 0x74, 0x70, 0x2f, 0x52, 0x65, 0x61, 0x64,
                                  0x46, 0x69, 0x6c, 0x65, 0x00,
                                  // mode
                                  0x6e, 0x65, 0x74, 0x61, 0x73, 0x63, 0x69, 0x69, 0x00};
    auto Length = sizeof(PacketBytes) / sizeof(std::uint8_t);

    auto Res = Parser<Request>::parse(PacketBytes, Length);
    auto [Packet, BytesRead] = Res.get();

    ASSERT_EQ(Packet.getType(), types::ReadRequest);
    ASSERT_EQ(Packet.getFilename(), "/srv/tftp/ReadFile");
    ASSERT_EQ(Packet.getMode(), "netascii");

    ASSERT_EQ(Res.isSuccess(), true);
    ASSERT_EQ(BytesRead, Length);
}

/// Test that Request packet with options parsing is going fine
TEST(Request, OptionParse) {
    std::uint8_t PacketBytes[] = {// type
                                  0x00, 0x01,
                                  // filename
                                  0x2f, 0x73, 0x72, 0x76, 0x2f, 0x74, 0x66, 0x74, 0x70, 0x2f, 0x52, 0x65, 0x61, 0x64,
                                  0x46, 0x69, 0x6c, 0x65, 0x00,
                                  // mode
                                  0x6e, 0x65, 0x74, 0x61, 0x73, 0x63, 0x69, 0x69, 0x00,
                                  // saveFiles option name
                                  0x73, 0x61, 0x76, 0x65, 0x46, 0x69, 0x6C, 0x65, 0x73, 0x00,
                                  // saveFiles option value
                                  0x74, 0x72, 0x75, 0x65, 0x00,
                                  // discardQualifiers option name
                                  0x64, 0x69, 0x73, 0x63, 0x61, 0x72, 0x64, 0x51, 0x75, 0x61, 0x6C, 0x69, 0x66, 0x69,
                                  0x65, 0x72, 0x73, 0x00,
                                  // discardQualifiers option value
                                  0x66, 0x61, 0x6C, 0x73, 0x65, 0x00,
                                  // secret option name
                                  0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x00,
                                  // secret option value
                                  0x49, 0x78, 0x30, 0x65, 0x38, 0x36, 0x79, 0x47, 0x38, 0x59, 0x70, 0x46, 0x7A, 0x77,
                                  0x7A, 0x31, 0x67, 0x53, 0x30, 0x58, 0x78, 0x4A, 0x57, 0x33, 0x00};
    auto Length = sizeof(PacketBytes) / sizeof(std::uint8_t);

    auto Res = Parser<Request>::parse(PacketBytes, Length);
    auto [Packet, BytesRead] = Res.get();

    ASSERT_EQ(Packet.getType(), types::ReadRequest);
    ASSERT_EQ(Packet.getFilename(), "/srv/tftp/ReadFile");
    ASSERT_EQ(Packet.getMode(), "netascii");

    std::vector<std::string> OptionsNames = {"saveFiles", "discardQualifiers", "secret"};
    std::vector<std::string> OptionsValues = {"true", "false", "Ix0e86yG8YpFzwz1gS0XxJW3"};
    for (std::size_t Idx = 0; Idx != OptionsNames.size(); ++Idx) {
        ASSERT_EQ(Packet.getOptionName(Idx), OptionsNames[Idx]);
        ASSERT_EQ(Packet.getOptionValue(Idx), OptionsValues[Idx]);
    }

    ASSERT_EQ(Res.isSuccess(), true);
    ASSERT_EQ(BytesRead, Length);
}

/// Test that Data packet parsing is going fine
TEST(Data, Parse) {
    std::uint8_t PacketBytes[] = {// type
                                  0x00, 0x03,
                                  // block number
                                  0x00, 0x01,
                                  // data
                                  0x53, 0x6f, 0x6d, 0x65, 0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x73, 0x2e,
                                  0x2e, 0x2e, 0x0d, 0x0a};
    auto Length = sizeof(PacketBytes) / sizeof(std::uint8_t);

    auto Res = Parser<Data>::parse(PacketBytes, Length);
    auto [Packet, BytesRead] = Res.get();

    ASSERT_EQ(Packet.getType(), types::DataPacket);
    ASSERT_EQ(Packet.getBlock(), 0x01);
    const auto &Data = Packet.getData();

    std::size_t BaseOffset = 2 * sizeof(std::uint16_t);
    for (std::size_t Idx = 0; Idx != Data.size(); ++Idx) {
        ASSERT_EQ(Data[Idx], PacketBytes[BaseOffset + Idx]);
    }

    ASSERT_EQ(Res.isSuccess(), true);
    ASSERT_EQ(BytesRead, Length);
}

/// Test that Acknowledgment packet parsing is going fine
TEST(Acknowledgment, Parse) {
    std::uint8_t PacketBytes[] = {// type
                                  0x00, 0x04,
                                  // block number
                                  0x00, 0x01};
    auto Length = sizeof(PacketBytes) / sizeof(std::uint8_t);

    auto Res = Parser<Acknowledgment>::parse(PacketBytes, Length);
    auto [Packet, BytesRead] = Res.get();

    ASSERT_EQ(Packet.getType(), types::AcknowledgmentPacket);
    ASSERT_EQ(Packet.getBlock(), 0x01);

    ASSERT_EQ(Res.isSuccess(), true);
    ASSERT_EQ(BytesRead, Length);
}

/// Test that Error packet parsing is going fine
TEST(Error, Parse) {
    std::uint8_t PacketBytes[] = {// type
                                  0x00, 0x05,
                                  // errorCode
                                  0x00, 0x01,
                                  // errorMessage
                                  0x46, 0x69, 0x6c, 0x65, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64,
                                  0x00};
    auto Length = sizeof(PacketBytes) / sizeof(std::uint8_t);

    auto Res = Parser<Error>::parse(PacketBytes, Length);
    auto [Packet, BytesRead] = Res.get();

    ASSERT_EQ(Packet.getType(), types::ErrorPacket);
    ASSERT_EQ(Packet.getErrorCode(), 0x01);
    ASSERT_EQ(Packet.getErrorMessage(), "File not found");

    ASSERT_EQ(Res.isSuccess(), true);
    ASSERT_EQ(BytesRead, Length);
}

/// Test that Option Acknowledgment packet parsing is going fine
TEST(OptionAcknowledgment, Parse) {
    std::uint8_t PacketBytes[] = {// type
                                  0x00, 0x06,
                                  // saveFiles option name
                                  0x73, 0x61, 0x76, 0x65, 0x46, 0x69, 0x6C, 0x65, 0x73, 0x00,
                                  // saveFiles option value
                                  0x74, 0x72, 0x75, 0x65, 0x00,
                                  // discardQualifiers option name
                                  0x64, 0x69, 0x73, 0x63, 0x61, 0x72, 0x64, 0x51, 0x75, 0x61, 0x6C, 0x69, 0x66, 0x69,
                                  0x65, 0x72, 0x73, 0x00,
                                  // discardQualifiers option value
                                  0x66, 0x61, 0x6C, 0x73, 0x65, 0x00,
                                  // secret option name
                                  0x73, 0x65, 0x63, 0x72, 0x65, 0x74, 0x00,
                                  // secret option value
                                  0x49, 0x78, 0x30, 0x65, 0x38, 0x36, 0x79, 0x47, 0x38, 0x59, 0x70, 0x46, 0x7A, 0x77,
                                  0x7A, 0x31, 0x67, 0x53, 0x30, 0x58, 0x78, 0x4A, 0x57, 0x33, 0x00};
    auto Length = sizeof(PacketBytes) / sizeof(std::uint8_t);

    auto Res = Parser<OptionAcknowledgment>::parse(PacketBytes, Length);
    auto [Packet, BytesRead] = Res.get();

    ASSERT_EQ(Packet.getType(), types::OptionAcknowledgmentPacket);

    std::unordered_map<std::string, std::string> Options = {
        {"saveFiles", "true"}, {"discardQualifiers", "false"}, {"secret", "Ix0e86yG8YpFzwz1gS0XxJW3"}};
    for (const auto &[Key, Value] : Options) {
        ASSERT_EQ(Packet.getOptionValue(Key), Value);
    }

    ASSERT_EQ(Res.isSuccess(), true);
    ASSERT_EQ(BytesRead, Length);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
