// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#ifndef ARCUS_ERROR_H
#define ARCUS_ERROR_H

#include "Arcus/Types.h"

namespace Arcus
{
/**
 * Possible error codes.
 */
enum class ErrorCode
{
    UnknownError, ///< An unknown error occurred.
    CreationError, ///< Socket creation failed.
    ConnectFailedError, ///< Connection failed.
    BindFailedError, ///< Bind to IP and port failed.
    AcceptFailedError, ///< Accepting an incoming connection failed.
    SendFailedError, ///< Sending a message failed.
    MessageTooBigError, ///< Sending a message failed because it was too big.
    ReceiveFailedError, ///< Receiving a message failed.
    UnknownMessageTypeError, ///< Received a message with an unknown message type.
    ParseFailedError, ///< Parsing the received message failed.
    ConnectionResetError, ///< The connection was reset by peer.
    MessageRegistrationFailedError, ///< Message registration failed.
    InvalidStateError, ///< Socket is in an invalid state.
    InvalidMessageError, ///< Message being handled is a nullptr or otherwise invalid.
    Debug, // Debug messages
};

/**
 * A class representing an error with an error code and an error message.
 */
class Error
{
public:
    /**
     * Default constructor.
     */
    Error();
    /**
     * Create an error with an error code and error message.
     */
    Error(ErrorCode error_code, const std::string& error_message);


    /**
     * Get the error code of this error.
     */
    ErrorCode getErrorCode() const;
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
    ErrorCode _error_code;
    std::string _error_message;
    bool _fatal_error;
    int _native_error_code;
};
} // namespace Arcus

// Output the error to a stream.
std::ostream& operator<<(std::ostream& stream, const Arcus::Error& error);

#endif // ARCUS_ERROR_H
