#pragma once

#include "packets.hpp"
#include <optional>
#include <variant>

namespace tftp::packets {

/// The result of parsing a single packet
template <typename T> struct ParseResult {
    T Packet;
    std::size_t BytesRead;
};

/// Return type of `parse` functions
template <typename T> struct ParseReturn : public std::variant<ParseResult<T>, std::nullopt_t> {
    using base = std::variant<ParseResult<T>, std::nullopt_t>;
    using base::base;

    ParseResult<T> get() const noexcept { return std::get<ParseResult<T>>(*this); }

    bool isSuccess() const noexcept { return !std::holds_alternative<std::nullopt_t>(*this); }
};

template <typename T> struct Parser {
    using PacketType = T;
};

template <> struct Parser<Request> {
    /// Parse read/write request packet from buffer converting all fields to host byte order
    /// @param[Buffer] Assumptions: \p Buffer is not a nullptr, it's size is greater or equal than \p Len
    /// @param[Len] Assumptions: \p Len is greater than zero
    /// @n If parsing wasn't successful, \p Packet remains in valid but unspecified state
    static ParseReturn<Request> parse(const std::uint8_t *Buffer, std::size_t Len) {
        assert(Buffer != nullptr);
        assert(Len > 0);

        std::uint16_t Type_;
        std::string Filename;
        std::string Mode;
        std::vector<std::string> OptionsNames;
        std::vector<std::string> OptionsValues;
        std::string Name;
        std::string Value;

        std::size_t Step = 0;
        std::size_t BytesRead = 0;
        for (std::size_t Idx = 0; Idx != Len; ++Idx) {
            const auto Byte = Buffer[Idx];
            BytesRead++;

            switch (Step) {
            // Opcode (2 bytes)
            case 0:
                Type_ = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 1:
                Type_ |= std::uint16_t(Byte) << 8;
                Type_ = ntohs(Type_);
                if (Type_ != types::ReadRequest && Type_ != types::WriteRequest) {
                    Step = 0;
                    continue;
                }
                Step++;
                break;
            // Filename
            case 2:
                if (Byte == 0u) {
                    Step++;
                } else {
                    Filename.push_back(Byte);
                }
                break;
            // Mode
            case 3:
                if (Byte == 0u) {
                    if (Idx == Len - 1) {
                        return ParseResult<Request>{Request{(types::Type)Type_, std::move(Filename), std::move(Mode)},
                                                    BytesRead};
                    }
                    Step++;
                } else {
                    Mode.push_back(Byte);
                }
                break;
            // Option name
            case 4:
                if (Byte == 0u) {
                    OptionsNames.push_back(std::move(Name));
                    Step++;
                } else {
                    Name.push_back(Byte);
                }
                break;
            // Option value
            case 5:
                if (Byte == 0u) {
                    OptionsValues.push_back(std::move(Value));

                    if (Idx == Len - 1) {
                        return ParseResult<Request>{Request{(types::Type)Type_, std::move(Filename), std::move(Mode),
                                                            std::move(OptionsNames), std::move(OptionsValues)},
                                                    BytesRead};
                    }
                    Step--;
                } else {
                    Value.push_back(Byte);
                }
                break;
            default:
                assert(false);
            }
        }
        return {std::nullopt};
    }
};

template <> struct Parser<Data> {
    /// Parse data packet from buffer converting all fields to host byte order
    /// @param[Buffer] Assumptions: \p Buffer is not a nullptr, it's size is greater or equal than \p Len
    /// @param[Len] Assumptions: \p Len is greater than zero
    /// @n If parsing wasn't successful, \p Packet remains in valid but unspecified state
    static ParseReturn<Data> parse(const std::uint8_t *Buffer, std::size_t Len) {
        assert(Buffer != nullptr);
        assert(Len > 0);

        std::uint16_t Type_;
        std::uint16_t Block;
        std::vector<std::uint8_t> DataBuffer;

        std::size_t Step = 0;
        std::size_t BytesRead = 0;
        for (std::size_t Idx = 0; Idx != Len; ++Idx) {
            const auto Byte = Buffer[Idx];
            BytesRead++;

            switch (Step) {
            // Opcode (2 bytes)
            case 0:
                Type_ = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 1:
                Type_ |= std::uint16_t(Byte) << 8;
                Type_ = ntohs(Type_);
                if (Type_ != types::DataPacket) {
                    Step = 0;
                    continue;
                }
                Step++;
                break;
            // Block # (2 bytes)
            case 2:
                Block = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 3:
                Block |= std::uint16_t(Byte) << 8;
                Block = ntohs(Block);
                Step++;
                break;
            // buffer
            case 4:
                DataBuffer.push_back(Byte);

                if (Idx == Len - 1) {
                    return ParseResult<Data>{Data{Block, std::move(DataBuffer)}, BytesRead};
                }

                break;
            default:
                assert(false);
            }
        }
        return {std::nullopt};
    }
};

template <> struct Parser<Acknowledgment> {
    /// Parse acknowledgment packet from buffer converting all fields to host byte order
    /// @param[Buffer] Assumptions: \p Buffer is not a nullptr, it's size is greater or equal than \p Len
    /// @param[Len] Assumptions: \p Len is greater than zero
    /// @n If parsing wasn't successful, \p Packet remains in valid but unspecified state
    static ParseReturn<Acknowledgment> parse(const std::uint8_t *Buffer, std::size_t Len) {
        assert(Buffer != nullptr);
        assert(Len > 0);

        std::uint16_t Type_;
        std::uint16_t Block;

        std::size_t Step = 0;
        std::size_t BytesRead = 0;
        for (std::size_t Idx = 0; Idx != Len; ++Idx) {
            const auto Byte = Buffer[Idx];
            BytesRead++;

            switch (Step) {
            // Opcode (2 bytes)
            case 0:
                Type_ = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 1:
                Type_ |= std::uint16_t(Byte) << 8;
                Type_ = ntohs(Type_);
                if (Type_ != types::AcknowledgmentPacket) {
                    Step = 0;
                    continue;
                }
                Step++;
                break;
            // Block # (2 bytes)
            case 2:
                Block = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 3:
                Block |= std::uint16_t(Byte) << 8;
                Block = ntohs(Block);
                return ParseResult<Acknowledgment>{Acknowledgment{Block}, BytesRead};
            default:
                assert(false);
            }
        }
        return {std::nullopt};
    }
};

template <> struct Parser<Error> {
    /// Parse error packet from buffer converting all fields to host byte order
    /// @param[Buffer] Assumptions: \p Buffer is not a nullptr, it's size is greater or equal than \p Len
    /// @param[Len] Assumptions: \p Len is greater than zero
    /// @n If parsing wasn't successful, \p Packet remains in valid but unspecified state
    static ParseReturn<Error> parse(const std::uint8_t *Buffer, std::size_t Len) {
        assert(Buffer != nullptr);
        assert(Len > 0);

        std::uint16_t Type_ = types::ErrorPacket;
        std::uint16_t ErrorCode;
        std::string ErrorMessage;

        std::size_t Step = 0;
        std::size_t BytesRead = 0;
        for (std::size_t Idx = 0; Idx != Len; ++Idx) {
            const auto Byte = Buffer[Idx];
            BytesRead++;

            switch (Step) {
            // Opcode (2 bytes)
            case 0:
                Type_ = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 1:
                Type_ |= std::uint16_t(Byte) << 8;
                Type_ = ntohs(Type_);
                if (Type_ != types::ErrorPacket) {
                    Step = 0;
                    continue;
                }
                Step++;
                break;
            // ErrorCode (2 bytes)
            case 2:
                ErrorCode = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 3:
                ErrorCode |= std::uint16_t(Byte) << 8;
                ErrorCode = ntohs(ErrorCode);
                Step++;
                break;
            // ErrorMessage
            case 4:
                if (Byte == 0u) {
                    return ParseResult<Error>{Error{ErrorCode, ErrorMessage}, BytesRead};
                } else {
                    ErrorMessage.push_back(Byte);
                }
                break;
            default:
                assert(false);
            }
        }
        return {std::nullopt};
    }
};

template <> struct Parser<OptionAcknowledgment> {

    /// Parse error packet from buffer converting all fields to host byte order
    /// @param[Buffer] Assumptions: \p Buffer is not a nullptr, it's size is greater or equal than \p Len
    /// @param[Len] Assumptions: \p Len is greater than zero
    /// @n If parsing wasn't successful, \p Packet remains in valid but unspecified state
    static ParseReturn<OptionAcknowledgment> parse(const std::uint8_t *Buffer, std::size_t Len) {
        assert(Buffer != nullptr);
        assert(Len > 0);

        std::uint16_t Type_;
        // According to the RFC, the order in which options are specified is not significant, so it's fine
        std::unordered_map<std::string, std::string> Options;
        std::string Name;
        std::string Value;

        std::size_t Step = 0;
        std::size_t BytesRead = 0;
        for (std::size_t Idx = 0; Idx != Len; ++Idx) {
            const auto Byte = Buffer[Idx];
            BytesRead++;

            switch (Step) {
            // Opcode (2 bytes)
            case 0:
                Type_ = std::uint16_t(Byte) << 0;
                Step++;
                break;
            case 1:
                Type_ |= std::uint16_t(Byte) << 8;
                Type_ = ntohs(Type_);
                if (Type_ != types::OptionAcknowledgmentPacket) {
                    Step = 0;
                    continue;
                }
                Step++;
                break;
            // Option name
            case 2:
                if (Byte == 0u)
                    Step++;
                else
                    Name.push_back(Byte);
                break;
            // Option value
            case 3:
                if (Byte == 0u) {
                    Options.emplace(std::move(Name), std::move(Value));

                    if (Idx == Len - 1) {
                        return ParseResult<OptionAcknowledgment>{OptionAcknowledgment{std::move(Options)}, BytesRead};
                    }
                    Step--;
                } else {
                    Value.push_back(Byte);
                }
                break;
            default:
                assert(false);
            }
        }
        return {std::nullopt};
    }
};

} // namespace tftp::packets
