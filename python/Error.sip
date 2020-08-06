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

namespace ErrorCode
{
    enum ErrorCode
    {
        UnknownError,
        CreationError,
        ConnectFailedError,
        BindFailedError,
        AcceptFailedError,
        SendFailedError,
        ReceiveFailedError,
        UnknownMessageTypeError,
        ParseFailedError,
        ConnectionResetError,
        MessageRegistrationFailedError,
        InvalidStateError,
        InvalidMessageError,
        Debug,
    };
};

class Error
{
    %TypeHeaderCode
    #include "Error.h"
    %End

public:
    Error();
    Error(ErrorCode::ErrorCode error_code, const std::string& error_message);
    Error(const Error& error);

    ErrorCode::ErrorCode getErrorCode() const;
    std::string getErrorMessage() const;
    bool isFatalError() const;
    bool isValid() const;
    void setFatalError(bool fatal);

    PyObject* __repr__();
    %MethodCode
    return PyUnicode_FromString(sipCpp->toString().c_str());
    %End
};
