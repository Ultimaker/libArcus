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

#ifndef ARCUS_MESSAGE_TYPE_STORE_H
#define ARCUS_MESSAGE_TYPE_STORE_H

#include <memory>

#include "Types.h"

namespace Arcus
{
    class MessageTypeStore
    {
    public:
        MessageTypeStore();
        ~MessageTypeStore();

        bool hasType(uint type_id) const;
        bool hasType(const std::string& type_name) const;

        MessagePtr createMessage(uint type_id) const;
        MessagePtr createMessage(const std::string& type_name) const;

        uint getMessageTypeId(const MessagePtr& message);

        bool registerMessageType(const google::protobuf::Message* message_type);
        bool registerAllMessageTypes(const std::string& file_name);

        void dumpMessageTypes();

    private:
        class Private;
        const std::unique_ptr<Private> d;
    };
}

#endif //ARCUS_MESSAGE_TYPE_STORE_H
