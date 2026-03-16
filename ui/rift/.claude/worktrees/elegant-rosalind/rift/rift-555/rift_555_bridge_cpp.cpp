/*
 * RIFT Stage 555: RIFTBridge C++ Wrapper
 * 
 * Provides C++ template-based interface for the RIFT pipeline.
 * Compatible with modern C++17 and later.
 */

#include "rift_555_bridge.h"
#include "../include/rift_pipeline.h"
#include "../nsigii-codec/nsigii_codec.h"
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <utility>

namespace rift {

/* ============================================================================
 * C++ TOKEN TYPES
 * ============================================================================ */
enum class TokenType {
    Unknown = TOKEN_TYPE_UNKNOWN,
    Int = TOKEN_TYPE_INT,
    Role = TOKEN_TYPE_ROLE,
    Mask = TOKEN_TYPE_MASK,
    Op = TOKEN_TYPE_OP,
    QByte = TOKEN_TYPE_QBYTE,
    QRole = TOKEN_TYPE_QROLE,
    QMatrix = TOKEN_TYPE_QMATRIX,
    Identifier = TOKEN_TYPE_IDENTIFIER,
    Keyword = TOKEN_TYPE_KEYWORD,
    Literal = TOKEN_TYPE_LITERAL,
    Operator = TOKEN_TYPE_OPERATOR,
    Delimiter = TOKEN_TYPE_DELIMITER,
    Eof = TOKEN_TYPE_EOF
};

/* ============================================================================
 * TOKEN TRIPLET TEMPLATE
 * ============================================================================ */
template<typename T, typename V, typename M>
class TokenTriplet {
public:
    TokenType type;
    T value;
    M memory;
    uint32_t validation_bits;
    
    TokenTriplet() : type(TokenType::Unknown), validation_bits(0) {}
    
    bool is_valid() const {
        return (validation_bits & 0x03) == 0x03;
    }
    
    bool is_locked() const {
        return (validation_bits & 0x04) != 0;
    }
};

/* ============================================================================
 * RIFTBRIDGE C++ CLASS
 * ============================================================================ */
class Bridge {
public:
    Bridge(RiftPolar polar = RIFT_POLAR_CPP) 
        : bridge_(riftbridge_create(polar)), owned_(true) {}
    
    ~Bridge() {
        if (owned_ && bridge_) {
            riftbridge_destroy(bridge_);
        }
    }
    
    /* Disable copy */
    Bridge(const Bridge&) = delete;
    Bridge& operator=(const Bridge&) = delete;
    
    /* Enable move */
    Bridge(Bridge&& other) noexcept 
        : bridge_(other.bridge_), owned_(other.owned_) {
        other.bridge_ = nullptr;
        other.owned_ = false;
    }
    
    Bridge& operator=(Bridge&& other) noexcept {
        if (this != &other) {
            if (owned_ && bridge_) {
                riftbridge_destroy(bridge_);
            }
            bridge_ = other.bridge_;
            owned_ = other.owned_;
            other.bridge_ = nullptr;
            other.owned_ = false;
        }
        return *this;
    }
    
    /* Initialization */
    bool initialize() {
        return bridge_ && riftbridge_initialize(bridge_) == 0;
    }
    
    /* Pipeline stages */
    bool tokenize(const std::string& input) {
        return bridge_ && riftbridge_tokenize(bridge_, input.c_str()) == 0;
    }
    
    bool process() {
        return bridge_ && riftbridge_process(bridge_) == 0;
    }
    
    bool build_ast() {
        return bridge_ && riftbridge_build_ast(bridge_) == 0;
    }
    
    bool generate_target() {
        return bridge_ && riftbridge_generate_target(bridge_) == 0;
    }
    
    bool execute_pipeline(const std::string& input) {
        return bridge_ && riftbridge_execute_pipeline(bridge_, input.c_str()) == 0;
    }
    
    /* NSIGII codec */
    std::vector<uint8_t> encode_nsigii(const std::vector<uint8_t>& input) {
        if (!bridge_) return {};
        
        uint8_t* output = nullptr;
        size_t output_len = 0;
        
        if (riftbridge_encode_nsigii(bridge_, input.data(), input.size(), 
                                      &output, &output_len) == 0) {
            std::vector<uint8_t> result(output, output + output_len);
            free(output);
            return result;
        }
        
        return {};
    }
    
    /* Error handling */
    std::string error_message() const {
        return bridge_ ? bridge_->error_message : "Bridge not created";
    }
    
    int error_code() const {
        return bridge_ ? bridge_->error_code : -1;
    }
    
    /* Version */
    static std::string version() {
        return riftbridge_version_string();
    }
    
private:
    RiftBridge* bridge_;
    bool owned_;
};

/* ============================================================================
 * HEX PATTERN MATCHER
 * ============================================================================ */
template<typename AlphaType, typename NumericType>
class HexMatcher {
public:
    using Result = std::pair<AlphaType, NumericType>;
    
    static constexpr const char* pattern = "[A-Z][0-9]$";
    
    Result match(const std::string& input) {
        Result result{};
        
        /* Use C++ regex for pattern matching */
        std::regex hex_regex("([A-Z])([0-9])");
        std::smatch match;
        
        if (std::regex_search(input, match, hex_regex)) {
            if (match.size() > 1) {
                result.first = convert_alpha(match[1].str());
            }
            if (match.size() > 2) {
                result.second = convert_numeric(match[2].str());
            }
        }
        
        return result;
    }
    
private:
    AlphaType convert_alpha(const std::string& str) {
        if constexpr (std::is_same_v<AlphaType, char>) {
            return str.empty() ? '\0' : str[0];
        } else if constexpr (std::is_same_v<AlphaType, std::string>) {
            return str;
        } else {
            return AlphaType{};
        }
    }
    
    NumericType convert_numeric(const std::string& str) {
        if constexpr (std::is_integral_v<NumericType>) {
            return str.empty() ? 0 : std::stoi(str);
        } else if constexpr (std::is_floating_point_v<NumericType>) {
            return str.empty() ? 0.0 : std::stod(str);
        } else {
            return NumericType{};
        }
    }
};

/* ============================================================================
 * TRIDENT CHANNEL WRAPPER
 * ============================================================================ */
class TridentChannel {
public:
    TridentChannel(uint8_t id) : channel_(nsigii_trident_channel_create(id)) {}
    
    ~TridentChannel() {
        if (channel_) {
            nsigii_trident_channel_destroy(channel_);
        }
    }
    
    /* Disable copy, enable move */
    TridentChannel(const TridentChannel&) = delete;
    TridentChannel& operator=(const TridentChannel&) = delete;
    TridentChannel(TridentChannel&&) = default;
    TridentChannel& operator=(TridentChannel&&) = default;
    
    std::string loopback_address() const {
        return channel_ ? channel_->loopback_addr : "";
    }
    
    uint8_t state() const {
        return channel_ ? channel_->state : 0;
    }
    
private:
    NsigiiTridentChannel* channel_;
};

/* ============================================================================
 * NSIGII CODEC WRAPPER
 * ============================================================================ */
class NSIGIICodec {
public:
    NSIGIICodec(int width, int height) 
        : codec_(nsigii_codec_create(width, height)) {}
    
    ~NSIGIICodec() {
        if (codec_) {
            nsigii_codec_destroy(codec_);
        }
    }
    
    /* Disable copy, enable move */
    NSIGIICodec(const NSIGIICodec&) = delete;
    NSIGIICodec& operator=(const NSIGIICodec&) = delete;
    NSIGIICodec(NSIGIICodec&&) = default;
    NSIGIICodec& operator=(NSIGIICodec&&) = default;
    
    static std::string version() {
        return nsigii_version_string();
    }
    
private:
    NsigiiCodec* codec_;
};

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
inline std::string polar_to_string(RiftPolar polar) {
    return rift_polar_to_string(polar);
}

inline std::string discriminant_state_to_string(NsigiiDiscriminantState state) {
    return nsigii_discriminant_state_to_string(state);
}

} /* namespace rift */

/* ============================================================================
 * EXTERN C INTERFACE FOR C# P/INVOKE
 * ============================================================================ */
extern "C" {

/* Platform-specific export macro */
#if defined(_WIN32) || defined(_WIN64)
    #define RIFT_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define RIFT_EXPORT __attribute__((visibility("default")))
#else
    #define RIFT_EXPORT
#endif

/* C# compatible exports */
RIFT_EXPORT RiftBridge* riftbridge_create_cs() {
    return riftbridge_create(RIFT_POLAR_CS);
}

RIFT_EXPORT int riftbridge_execute_pipeline_cs(RiftBridge* bridge, const char* input) {
    if (!bridge) return -1;
    return riftbridge_execute_pipeline(bridge, input);
}

RIFT_EXPORT const char* riftbridge_get_error_cs(RiftBridge* bridge) {
    if (!bridge) return "Null bridge";
    return bridge->error_message;
}

RIFT_EXPORT void riftbridge_destroy_cs(RiftBridge* bridge) {
    riftbridge_destroy(bridge);
}

} /* extern "C" */
