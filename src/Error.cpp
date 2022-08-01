// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#include "Arcus/Error.h"
#include <iostream>

using namespace Arcus;

Arcus::Error::Error() : _error_code(ErrorCode::UnknownError), _fatal_error(false), _native_error_code(0)
{
}

Arcus::Error::Error(ErrorCode error_code, const std::string& error_message) : _error_code(ErrorCode::UnknownError), _fatal_error(false), _native_error_code(0)
{
    _error_code = error_code;
    _error_message = error_message;
}

ErrorCode Arcus::Error::getErrorCode() const
{
    return _error_code;
}

std::string Arcus::Error::getErrorMessage() const
{
    return _error_message;
}

bool Arcus::Error::isFatalError() const
{
    return _fatal_error;
}

bool Arcus::Error::isValid() const
{
    return _error_code != ErrorCode::UnknownError || ! _error_message.empty();
}

int Arcus::Error::getNativeErrorCode() const
{
    return _native_error_code;
}

void Arcus::Error::setFatalError(bool fatal)
{
    _fatal_error = fatal;
}

void Arcus::Error::setNativeErrorCode(int code)
{
    _native_error_code = code;
}

std::string Arcus::Error::toString() const
{
    static std::string error_start("Arcus Error (");
    static std::string fatal_error_start("Arcus Fatal Error (");
    static std::string native_prefix(", native ");
    static std::string message_separator("): ");

    return (_fatal_error ? fatal_error_start : error_start) + std::to_string(static_cast<int>(_error_code)) + (_native_error_code != 0 ? native_prefix + std::to_string(_native_error_code) : "") + message_separator + _error_message;
}

std::ostream& operator<<(std::ostream& stream, const Arcus::Error& error)
{
    return stream << error.toString();
}
