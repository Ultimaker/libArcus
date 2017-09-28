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

#ifndef ARCUS_ERROR_H
#define ARCUS_ERROR_H

#include "ArcusExport.h"
#include "Types.h"

namespace Arcus
{
    /**
     * Possible error codes.
     */
    namespace ErrorCode
    {
        // Note: Not using enum class due to incompatibility with SIP.
        enum ErrorCode
        {
            UnknownError, ///< An unknown error occurred.
            CreationError, ///< Socket creation failed.
            ConnectFailedError, ///< Connection failed.
            BindFailedError, ///< Bind to IP and port failed.
            AcceptFailedError, ///< Accepting an incoming connection failed.
            SendFailedError, ///< Sending a message failed.
            ReceiveFailedError, ///< Receiving a message failed.
            UnknownMessageTypeError, ///< Received a message with an unknown message type.
            ParseFailedError, ///< Parsing the received message failed.
            ConnectionResetError, ///< The connection was reset by peer.
            MessageRegistrationFailedError, ///< Message registration failed.
            InvalidStateError, ///< Socket is in an invalid state.
            InvalidMessageError, ///< Message being handled is a nullptr or otherwise invalid.
            Debug, //Debug messages
        };
    }

    /**
     * A class representing an error with an error code and an error message.
     */
    class ARCUS_EXPORT Error
    {
    public:
        /**
         * Default constructor.
         */
        Error();
        /**
         * Create an error with an error code and error message.
         */
        Error(ErrorCode::ErrorCode error_code, const std::string& error_message);


        /**
         * Get the error code of this error.
         */
        ErrorCode::ErrorCode getErrorCode() const;
        /**
         * Get the error message.
         */
        std::string getErrorMessage() const;
        /**
         * Is this error considered a fatal error?
         */
        bool isFatalError() const;
        /**
         * Is this error valid?
         */
        bool isValid() const;
        /**
         * The error code as reported by the platform.
         */
        int getNativeErrorCode() const;
        /**
         * Set whether this should be considered a fatal error.
         */
        void setFatalError(bool fatal);
        /**
         * Set the native error code, if any.
         */
        void setNativeErrorCode(int code);
        /**
         * Convert the error to a string that can be printed.
         */
        std::string toString() const;

    private:
        ErrorCode::ErrorCode _error_code;
        std::string _error_message;
        bool _fatal_error;
        int _native_error_code;
    };
}

// Output the error to a stream.
ARCUS_EXPORT std::ostream& operator<<(std::ostream& stream, const Arcus::Error& error);

#endif //ARCUS_ERROR_H
