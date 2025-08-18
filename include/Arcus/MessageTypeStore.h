// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#ifndef ARCUS_MESSAGE_TYPE_STORE_H
#define ARCUS_MESSAGE_TYPE_STORE_H

#include <memory>
#include <cstdint>

#include "Arcus/Types.h"

namespace Arcus
{
/**
 * A class to manage the different types of messages that are available.
 */
class MessageTypeStore
{
public:
    MessageTypeStore();
    ~MessageTypeStore();

    /**
     * Check if a certain message type was registered.
     *
     * \param type_id The ID of the type to check for.
     *
     * \return true if the message type was registered, false if not.
     */
    bool hasType(uint32_t type_id) const;
    /**
     * Check if a certain message type was registered.
     *
     * \param type_name The name of the type to check for.
     *
     * \return true if the message type was registered, false if not.
     */
    bool hasType(const std::string& type_name) const;

    /**
     * Create a Message instance of a certain type.
     *
     * \param type_id The type ID of the message type to create an instance of.
     *
     * \return A new instance of a Message or an invalid pointer if type_id was an invalid type.
     */
    MessagePtr createMessage(uint32_t type_id) const;
    /**
     * Create a Message instance of a certain type.
     *
     * \param type_name The name of the message type to create an instance of.
     *
     * \return A new instance of a Message or an invalid pointer if type_id was an invalid type.
     */
    MessagePtr createMessage(const std::string& type_name) const;

    /**
     * Get the type ID of a message.
     *
     * \param message The message to get the type ID of.
     *
     * \return The type id of the message.
     */
    uint32_t getMessageTypeId(const MessagePtr& message);

    std::string getErrorMessages() const;

    /**
     * Register a message type.
     *
     * \param message_type An instance of a message that will be used as factory to create new messages.
     *
     * \return true if registration was successful, false if not.
     */
    bool registerMessageType(const google::protobuf::Message* message_type);
    /**
     * Register all message types from a Protobuf protocol description file.
     *
     * \param file_name The absolute path to a Protobuf proto file.
     *
     * \return true if registration was successful, false if not.
     */
    bool registerAllMessageTypes(const std::string& file_name);

    /**
     * Dump all message type IDs and type names to stdout.
     */
    void dumpMessageTypes();

private:
    class Private;
    const std::unique_ptr<Private> d;
};
} // namespace Arcus

#endif // ARCUS_MESSAGE_TYPE_STORE_H
