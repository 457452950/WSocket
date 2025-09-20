#pragma once
#ifndef WSOCKET__ERROR_H
#define WSOCKET__ERROR_H

#include <system_error>


namespace wsocket {

enum Error : int {
    Success            = 0,
    SysFrameError      = 1,
    ErrorReasonTooLong = 2,
    UnexpectedError    = 3,
    KeepAliveTimeout   = 4,
};

class ErrorCategory : public std::error_category {
public:
    const char *name() const noexcept override { return "WSocket::ErrorCategory"; }

    std::string message(int ev) const override {
        switch(static_cast<Error>(ev)) {
        case Success:
            return "Success";
        case SysFrameError:
            return "SysFrameError";
        case ErrorReasonTooLong:
            return "ErrorReasonTooLong";
        case UnexpectedError:
            return "UnexpectedError";
        case KeepAliveTimeout:
            return "KeepAliveTimeout";
        }

        return "Unknown error";
    }

    static const ErrorCategory &instance() {
        static ErrorCategory instance;
        return instance;
    }
};

inline std::error_code make_error_code(wsocket::Error e) {
    return {static_cast<int>(e), wsocket::ErrorCategory::instance()};
}

inline std::error_condition make_error_condition(wsocket::Error e) {
    return {static_cast<int>(e), wsocket::ErrorCategory::instance()};
}

} // namespace wsocket

template <>
struct std::is_error_code_enum<wsocket::Error> : true_type {}; // namespace std


#endif // WSOCKET__ERROR_H
