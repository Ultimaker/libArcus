// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#ifndef ARCUS_TYPES_H
#define ARCUS_TYPES_H

#include <memory>
#include <string>
#include <cstdint>

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google

namespace Arcus
{
// Convenience typedef so uint can be used.
typedef uint32_t uint;
// Convenience typedef for standard message argument.
typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

/**
 * Socket state.
 */
enum class SocketState
{
    Initial, ///< Created, not running.
    Connecting, ///< Connecting to an address and port.
    Connected, ///< Connected and capable of sending and receiving messages.
    Opening, ///< Opening for incoming connections.
    Listening, ///< Listening for incoming connections.
    Closing, ///< Closing down.
    Closed, ///< Closed, not running.
    Error ///< A fatal error happened that blocks the socket from operating.
};
} // namespace Arcus

#endif // ARCUS_TYPES_H
