/*
 * This file is part of libArcus
 *
 * Copyright (C) 2016 Ultimaker b.v. <a.hiemstra@ultimaker.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Error.h"

using namespace Arcus;

Arcus::Error::Error()
    : _error_code(ErrorCode::UnknownError), _fatal_error(false)
{
}

Arcus::Error::Error(ErrorCode::ErrorCode error_code, const std::string& error_message)
    : _error_code(ErrorCode::UnknownError), _fatal_error(false)
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
    return _error_code != ErrorCode::UnknownError && !_error_message.empty();
}

void Arcus::Error::setFatalError(bool fatal)
{
    _fatal_error = fatal;
}

std::ostream & operator<<(std::ostream& stream, const Arcus::Error& error)
{
    static std::string error_start("Arcus Error (");
    static std::string fatal_error_start("Arcus Fatal Error (");
    static std::string message_separator("): ");

    stream << (error.isFatalError() ? fatal_error_start : error_start) << std::to_string(static_cast<int>(error.getErrorCode())) << message_separator << error.getErrorMessage();
    return stream;
}
