#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tftp::packets {

namespace types {

/// Trivial File Transfer Protocol packet type
enum Type : std::uint16_t {
    /// Read request (RRQ) operation code
    ReadRequest = 0x01,
    /// Write request (WRQ) operation code
    WriteRequest = 0x02,
    /// Data (DATA) operation code
    DataPacket = 0x03,
    /// Acknowledgment (ACK) operation code
    AcknowledgmentPacket = 0x04,
    /// Error (ERROR) operation code
    ErrorPacket = 0x05,
    /// Option Acknowledgment (OACK) operation code
    OptionAcknowledgmentPacket = 0x06
};

} // namespace types

namespace errors {

/// Trivial File Transfer Protocol error code
enum Error : std::uint16_t {
    /// Not defined, see error message (if any)
    NotDefined = 0,
    /// File not found
    FileNotFound = 1,
    /// Access violation
    AccessViolation = 2,
    /// Disk full or allocation exceeded (RFC 1350) or file too large (RFC2349)
    DiskFull = 3,
    /// Illegal TFTP operation
    IllegalOperation = 4,
    /// Unknown transfer ID
    UnknownTransferID = 5,
    /// File already exists
    FileAlreadyExists = 6,
    /// No such user
    NoSuchUser = 7,
    /// Wrong blocksize (RFC 2348)
    WrongBlocksize = 8
};

} // namespace errors

namespace modes {

/// Trivial File Transfer Protocol transfer mode
enum TransferMode {
    /// netascii transfer mode
    NetAscii,
    /// octet (binary) transfer mode
    Octet
};

} // namespace modes

/// Read/Write Request (RRQ/WRQ) Trivial File Transfer Protocol packet
class Request final {
  public:
    /// Use with parsing functions only
    Request() = default;
    /// @param[Type] Assumptions: The \p type is either ::ReadRequest or ::WriteRequest
    Request(types::Type Type, std::string_view Filename, std::string_view Mode)
        : Type_(Type), Filename(Filename), Mode(Mode) {
        assert(Type == types::ReadRequest || Type == types::WriteRequest);
    }
    /// @param[Type] Assumptions: The \p type is either ::ReadRequest or ::WriteRequest
    Request(types::Type Type, std::string &&Filename, std::string &&Mode) noexcept
        : Type_(Type), Filename(std::move(Filename)), Mode(std::move(Mode)) {
        assert(Type == types::ReadRequest || Type == types::WriteRequest);
    }
    /// @param[Type] Assumptions: The \p type is either ::ReadRequest or ::WriteRequest
    Request(types::Type Type, std::string_view Filename, std::string_view Mode,
            const std::vector<std::string> &OptionsNames, const std::vector<std::string> &OptionsValues)
        : Request(Type, Filename, Mode) {
        this->OptionsNames = OptionsNames;
        this->OptionsValues = OptionsValues;
    }
    /// @param[Type] Assumptions: The \p type is either ::ReadRequest or ::WriteRequest
    Request(types::Type Type, std::string &&Filename, std::string &&Mode, std::vector<std::string> &&OptionsNames,
            std::vector<std::string> &&OptionsValues) noexcept
        : Type_(Type), Filename(std::move(Filename)), Mode(std::move(Mode)), OptionsNames(std::move(OptionsNames)),
          OptionsValues(std::move(OptionsValues)) {
        assert(Type == types::ReadRequest || Type == types::WriteRequest);
    }

    /// Convert packet to network byte order and serialize it into the given buffer by the iterator
    /// @param[It] Requirements: \p *(It) must be assignable from \p std::uint8_t
    /// @return Size of the packet (in bytes)
    template <class OutputIterator> std::size_t serialize(OutputIterator It) const noexcept {
        assert(OptionsNames.size() == OptionsValues.size());

        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 8);

        for (auto Byte : Filename) {
            *(It++) = static_cast<std::uint8_t>(Byte);
        }
        *(It++) = '\0';

        for (auto Byte : Mode) {
            *(It++) = static_cast<std::uint8_t>(Byte);
        }
        *(It++) = '\0';

        std::size_t OptionsSize = 0;
        for (std::size_t Idx = 0; Idx != OptionsNames.size(); ++Idx) {
            for (auto Byte : OptionsNames[Idx]) {
                *(It++) = static_cast<std::uint8_t>(Byte);
            }
            *(It++) = '\0';
            for (auto Byte : OptionsValues[Idx]) {
                *(It++) = static_cast<std::uint8_t>(Byte);
            }
            *(It++) = '\0';
            OptionsSize += OptionsNames[Idx].size() + OptionsValues[Idx].size() + 2;
        }

        return sizeof(Type_) + Filename.size() + Mode.size() + OptionsSize + 2;
    }

    std::uint16_t getType() const noexcept { return Type_; }

    std::string_view getFilename() const noexcept { return std::string_view(Filename.data(), Filename.size()); }

    std::string_view getMode() const noexcept { return std::string_view(Mode.data(), Mode.size()); }

    std::string_view getOptionName(std::size_t Idx) const noexcept {
        return std::string_view(OptionsNames[Idx].data(), OptionsNames[Idx].size());
    }

    std::string_view getOptionValue(std::size_t Idx) const noexcept {
        return std::string_view(OptionsValues[Idx].data(), OptionsValues[Idx].size());
    }

  private:
    std::uint16_t Type_;
    std::string Filename;
    std::string Mode;
    std::vector<std::string> OptionsNames;
    std::vector<std::string> OptionsValues;
};

/// Data Trivial File Transfer Protocol packet
class Data final {
  public:
    /// Use with parsing functions only
    Data() = default;
    /// @param[Block] Assumptions: The \p Block value is greater than one
    /// @param[Buffer] Assumptions: The \p Buffer size is greater or equal than 0 and less or equal than 512
    Data(std::uint16_t Block, const std::vector<std::uint8_t> &Buffer)
        : Block(Block), DataBuffer(Buffer.begin(), Buffer.end()) {
        // The block numbers on data packets begin with one and increase by one for each new block of data
        assert(Block >= 1);
        // The data field is from zero to 512 bytes long
        assert(Buffer.size() >= 0 && Buffer.size() <= 512);
    }
    /// @param[Block] Assumptions: The \p Block value is greater than one
    /// @param[Buffer] Assumptions: The \p Buffer size is greater or equal than 0 and less or equal than 512
    Data(std::uint16_t Block, std::vector<std::uint8_t> &&Buffer) noexcept : Block(Block) {
        // The block numbers on data packets begin with one and increase by one for each new block of data
        assert(Block >= 1);
        // The data field is from zero to 512 bytes long
        assert(Buffer.size() >= 0 && Buffer.size() <= 512);
        this->DataBuffer = std::move(Buffer);
    }

    /// Convert packet to network byte order and serialize it into the given buffer by the iterator
    /// @param[It] Requirements: \p *(It) must be assignable from \p std::uint8_t
    /// @return Size of the packet (in bytes)
    template <class OutputIterator> std::size_t serialize(OutputIterator It) const noexcept {
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 8);
        *(It++) = static_cast<std::uint8_t>(htons(Block) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Block) >> 8);
        for (auto Byte : DataBuffer) {
            *(It++) = Byte;
        }

        return sizeof(Type_) + sizeof(Block) + DataBuffer.size();
    }

    std::uint16_t getType() const noexcept { return Type_; }

    std::uint16_t getBlock() const noexcept { return Block; }

    const std::vector<std::uint8_t> &getData() const noexcept { return DataBuffer; }

  private:
    std::uint16_t Type_ = types::DataPacket;
    std::uint16_t Block;
    std::vector<std::uint8_t> DataBuffer;
};

/// Acknowledgment Trivial File Transfer Protocol packet
class Acknowledgment final {
  public:
    /// Use with parsing functions only
    Acknowledgment() = default;
    /// @param[Block] Assumptions: the \p Block is equal or greater than one
    explicit Acknowledgment(std::uint16_t Block) noexcept : Block(Block) {
        // The block numbers on data packets begin with one and increase by one for each new block of data
        assert(Block >= 1);
    }

    std::uint16_t getType() const noexcept { return Type_; }

    std::uint16_t getBlock() const noexcept { return Block; }

    /// Convert packet to network byte order and serialize it into the given buffer by the iterator
    /// @param[It] Requirements: \p *(It) must be assignable from \p std::uint8_t
    /// @return Size of the packet (in bytes)
    template <class OutputIterator> std::size_t serialize(OutputIterator It) const noexcept {
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 8);
        *(It++) = static_cast<std::uint8_t>(htons(Block) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Block) >> 8);

        return sizeof(Type_) + sizeof(Block);
    }

  private:
    std::uint16_t Type_ = types::AcknowledgmentPacket;
    std::uint16_t Block;
};

/// Error Trivial File Transfer Protocol packet
class Error final {
  public:
    /// Use with parsing functions only
    Error() = default;
    /// @param[ErrorCode] Assumptions: The \p ErrorCode is equal or greater than zero and less or equal than eight
    Error(std::uint16_t ErrorCode, std::string_view ErrorMessage) : ErrorCode(ErrorCode), ErrorMessage(ErrorMessage) {
        assert(ErrorCode >= 0 && ErrorCode <= 8);
    }
    /// @param[ErrorCode] Assumptions: The \p ErrorCode is equal or greater than zero and less or equal than eight
    Error(std::uint16_t ErrorCode, std::string &&ErrorMessage)
        : ErrorCode(ErrorCode), ErrorMessage(std::move(ErrorMessage)) {
        assert(ErrorCode >= 0 && ErrorCode <= 8);
    }

    std::uint16_t getType() const noexcept { return Type_; }

    std::uint16_t getErrorCode() const noexcept { return ErrorCode; }

    std::string_view getErrorMessage() const noexcept {
        return std::string_view(ErrorMessage.data(), ErrorMessage.size());
    }

    /// Convert packet to network byte order and serialize it into the given buffer by the iterator
    /// @param[It] Requirements: \p *(It) must be assignable from \p std::uint8_t
    /// @return Size of the packet (in bytes)
    template <class OutputIterator> std::size_t serialize(OutputIterator It) const noexcept {
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 8);
        *(It++) = static_cast<std::uint8_t>(htons(ErrorCode) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(ErrorCode) >> 8);

        for (auto Byte : ErrorMessage) {
            *(It++) = Byte;
        }
        *(It++) = '\0';

        return sizeof(Type_) + sizeof(ErrorCode) + ErrorMessage.size() + 1;
    }

  private:
    std::uint16_t Type_ = types::ErrorPacket;
    std::uint16_t ErrorCode;
    std::string ErrorMessage;
};

/// Option Acknowledgment Trivial File Transfer Protocol packet
class OptionAcknowledgment final {
  public:
    /// Use with parsing functions only
    OptionAcknowledgment() = default;
    OptionAcknowledgment(std::unordered_map<std::string, std::string> Options) : Options(std::move(Options)) {}

    /// Convert packet to network byte order and serialize it into the given buffer by the iterator
    /// @param[It] Requirements: \p *(It) must be assignable from \p std::uint8_t
    /// @return Size of the packet (in bytes)
    template <class OutputIterator> std::size_t serialize(OutputIterator It) const noexcept {
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 0);
        *(It++) = static_cast<std::uint8_t>(htons(Type_) >> 8);

        std::size_t OptionsSize = 0;
        for (const auto &[Key, Value] : Options) {
            for (auto Byte : Key) {
                *(It++) = static_cast<std::uint8_t>(Byte);
            }
            *(It++) = '\0';
            for (auto Byte : Value) {
                *(It++) = static_cast<std::uint8_t>(Byte);
            }
            *(It++) = '\0';
            OptionsSize += Key.size() + Value.size() + 2;
        }

        return sizeof(Type_) + OptionsSize;
    }

    std::uint16_t getType() const noexcept { return Type_; }

    /// @return Iterator to the first option (name and value) pair
    auto begin() noexcept { return Options.begin(); }

    /// @return Constant iterator to the first option (name and value) pair
    auto begin() const noexcept { return Options.begin(); }

    /// @return Constant iterator to the first option (name and value) pair
    auto cbegin() const noexcept { return Options.cbegin(); }

    /// @return Iterator to the element following the last option (name and value) pair
    auto end() noexcept { return Options.end(); }

    /// @return Constant iterator to the element following the last option (name and value) pair
    auto end() const noexcept { return Options.end(); }

    /// @return Constant iterator to the element following the last option (name and value) pair
    auto cend() const noexcept { return Options.cend(); }

    /// Check if there's an option with the specified name
    bool hasOption(const std::string &OptionName) const noexcept { return Options.count(OptionName); }

    /// Get option value by its name
    /// @throws std::out_of_range if there's no option with the specified name
    std::string_view getOptionValue(const std::string &OptionName) const noexcept { return Options.at(OptionName); }

  private:
    std::uint16_t Type_ = types::OptionAcknowledgmentPacket;
    // According to the RFC, the order in which options are specified is not significant, so it's fine
    std::unordered_map<std::string, std::string> Options;
};

} // namespace tftp::packets
