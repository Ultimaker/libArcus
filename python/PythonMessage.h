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

#ifndef ARCUS_PYTHON_MESSAGE_H
#define ARCUS_PYTHON_MESSAGE_H

#include <Python.h>
#include "Types.h"

namespace google
{
    namespace protobuf
    {
        class Descriptor;
        class Reflection;
    }
}

namespace Arcus
{
    /**
     * A simple wrapper around a Protobuf message so it can be used from Python.
     *
     * This class wraps a Protobuf message and makes it possible to get and set
     * values from the message. Message properties are exposed as Python properties
     * so can be set using things like `message.data = b"something"` from Python.
     *
     * Repeated messages are supported, using addRepeatedMessage, repeatedMessageCount
     * and getRepeatedMessage. A repeated message is returned as a PythonMessage object
     * so exposes the same API as the top level message.
     */
    class PythonMessage
    {
    public:
        PythonMessage(google::protobuf::Message* message);
        PythonMessage(const MessagePtr& message);
        virtual ~PythonMessage();

        /**
         * Get the message type name of this message.
         */
        std::string getTypeName() const;

        /**
         * Python property interface.
         */
        bool __hasattr__(const std::string& field_name) const;
        PyObject* __getattr__(const std::string& field_name) const;
        void __setattr__(const std::string& name, PyObject* value);

        /**
         * Add an instance of a Repeated Message to a specific field.
         *
         * \param field_name The name of the field to add a message to.
         *
         * \return A pointer to an instance of PythonMessage wrapping the new Message in the field.
         */
        PythonMessage* addRepeatedMessage(const std::string& field_name);

        /**
         * Get the number of messages in a repeated message field.
         */
        int repeatedMessageCount(const std::string& field_name) const;

        /**
         * Get a specific instance of a message in a repeated message field.
         *
         * \param field_name The name of a repeated message field to get an instance from.
         * \param index The index of the item to get in the repeated field.
         *
         * \return A pointer to an instance of PythonMessage wrapping the specified repeated message.
         */
        PythonMessage* getRepeatedMessage(const std::string& field_name, int index);

        /**
         * Get a specific instance of a message in a message field.
         *
         * \param field_name The name of a repeated message field to get an instance from.
         *
         * \return A pointer to an instance of PythonMessage wrapping the specified repeated message.
         */
        PythonMessage* getMessage(const std::string& field_name);

        /**
         * Get the value of a certain enumeration.
         *
         * \param enum_value The fully-qualified name of an Enum value.
         *
         * \return The integer value of the specified enum.
         */
        int getEnumValue(const std::string& enum_value) const;

        /**
         * Internal.
         */
        MessagePtr getSharedMessage() const;

    private:
        MessagePtr _shared_message;
        google::protobuf::Message* _message;
        const google::protobuf::Reflection* _reflection;
        const google::protobuf::Descriptor* _descriptor;
    };
}

#endif //ARCUS_MESSAGE_PTR_H
