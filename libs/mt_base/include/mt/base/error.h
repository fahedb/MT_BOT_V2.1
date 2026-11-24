// ---------------------------------------------------------------------------
// Result<T> and Error — explicit failure propagation.
//
// Replaces the pre-refactor habit of logging an error and returning early with
// no indication to the caller (finding E9 in docs/REFACTOR_PLAN.md). A
// function that can fail returns Result<T>; the caller cannot ignore it by
// accident.
//
// std::expected is C++23; this is the minimal C++20 stand-in with the same
// shape, so it can be swapped later.
// ---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace mt {

enum class ErrorCode {
    Unknown = 0,
    InvalidArgument,
    OutOfRange,
    NotFound,
    AlreadyExists,
    FailedPrecondition,
    DataCorrupt,
    IoFailure,
    NotImplemented,
    Unavailable,
};

[[nodiscard]] constexpr std::string_view toString(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Unknown:            return "Unknown";
        case ErrorCode::InvalidArgument:    return "InvalidArgument";
        case ErrorCode::OutOfRange:         return "OutOfRange";
        case ErrorCode::NotFound:           return "NotFound";
        case ErrorCode::AlreadyExists:      return "AlreadyExists";
        case ErrorCode::FailedPrecondition: return "FailedPrecondition";
        case ErrorCode::DataCorrupt:        return "DataCorrupt";
        case ErrorCode::IoFailure:          return "IoFailure";
        case ErrorCode::NotImplemented:     return "NotImplemented";
        case ErrorCode::Unavailable:        return "Unavailable";
    }
    return "Unknown";
}

class Error {
public:
    Error() = default;

    Error(ErrorCode code, std::string message)
        : m_code(code), m_message(std::move(message)) {}

    explicit Error(ErrorCode code) : m_code(code) {}

    [[nodiscard]] ErrorCode code() const noexcept { return m_code; }
    [[nodiscard]] const std::string& message() const noexcept { return m_message; }

    [[nodiscard]] std::string describe() const {
        std::string out(toString(m_code));
        if (!m_message.empty()) {
            out += ": ";
            out += m_message;
        }
        return out;
    }

private:
    ErrorCode m_code = ErrorCode::Unknown;
    std::string m_message;
};

// A value or an error. Checking is enforced at the point of use: value() and
// error() assert, so a mistaken access fails loudly in debug builds rather
// than returning something plausible.
template <typename T>
class [[nodiscard]] Result {
public:
    Result(T value) : m_state(std::move(value)) {}          // NOLINT(google-explicit-constructor)
    Result(Error error) : m_state(std::move(error)) {}       // NOLINT(google-explicit-constructor)

    [[nodiscard]] bool hasValue() const noexcept { return m_state.index() == 0; }
    explicit operator bool() const noexcept { return hasValue(); }

    [[nodiscard]] const T& value() const& {
        assert(hasValue() && "Result::value() on an error");
        return std::get<0>(m_state);
    }

    [[nodiscard]] T&& value() && {
        assert(hasValue() && "Result::value() on an error");
        return std::get<0>(std::move(m_state));
    }

    [[nodiscard]] const Error& error() const& {
        assert(!hasValue() && "Result::error() on a value");
        return std::get<1>(m_state);
    }

    // Reads better than `r ? r.value() : fallback` at call sites.
    [[nodiscard]] T valueOr(T fallback) const& {
        return hasValue() ? std::get<0>(m_state) : std::move(fallback);
    }

private:
    std::variant<T, Error> m_state;
};

// Result<void> equivalent: success or an error, no payload.
class [[nodiscard]] Status {
public:
    Status() = default;
    Status(Error error) : m_error(std::move(error)), m_ok(false) {}  // NOLINT(google-explicit-constructor)

    [[nodiscard]] bool ok() const noexcept { return m_ok; }
    explicit operator bool() const noexcept { return m_ok; }

    [[nodiscard]] const Error& error() const {
        assert(!m_ok && "Status::error() on a success");
        return m_error;
    }

    static Status success() { return {}; }

private:
    Error m_error;
    bool m_ok = true;
};

// Shorthand constructors.
[[nodiscard]] inline Error invalidArgument(std::string message) {
    return {ErrorCode::InvalidArgument, std::move(message)};
}
[[nodiscard]] inline Error outOfRange(std::string message) {
    return {ErrorCode::OutOfRange, std::move(message)};
}
[[nodiscard]] inline Error notFound(std::string message) {
    return {ErrorCode::NotFound, std::move(message)};
}
[[nodiscard]] inline Error failedPrecondition(std::string message) {
    return {ErrorCode::FailedPrecondition, std::move(message)};
}
[[nodiscard]] inline Error dataCorrupt(std::string message) {
    return {ErrorCode::DataCorrupt, std::move(message)};
}
[[nodiscard]] inline Error ioFailure(std::string message) {
    return {ErrorCode::IoFailure, std::move(message)};
}

}  // namespace mt
