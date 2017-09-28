/*
 * This file is part of libArcus
 *
 * Copyright (C) 2016 Ultimaker b.v. <a.hiemstra@ultimaker.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License v3.0 as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License v3.0 for more details.
 * You should have received a copy of the GNU Lesser General Public License v3.0
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Error.h"
#include <iostream>

using namespace Arcus;

Arcus::Error::Error()
    : _error_code(ErrorCode::UnknownError), _fatal_error(false), _native_error_code(0)
{
}

Arcus::Error::Error(ErrorCode::ErrorCode error_code, const std::string& error_message)
    : _error_code(ErrorCode::UnknownError), _fatal_error(false), _native_error_code(0)
{
    _error_code = error_code;
    _error_message = error_message;
}

ErrorCode::ErrorCode Arcus::Error::getErrorCode() const
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
    return _error_code != ErrorCode::UnknownError || !_error_message.empty();
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

std::ostream & operator<<(std::ostream& stream, const Arcus::Error& error)
{
    return stream << error.toString();
}
