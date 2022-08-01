// Copyright (c) 2022 Ultimaker B.V.
// libArcus is released under the terms of the LGPLv3 or higher.

#include "Arcus/MessageTypeStore.h"

#include <iostream>
#include <sstream>
#include <unordered_map>

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

using namespace Arcus;

/**
 * Taken from libstdc++, this implements hashing a string to an int.
 *
 * Since we rely on the hashing method for type ID generation and the implementation
 * of std::hash differs between compilers, we need to make sure we use the same
 * implementation everywhere.
 */
uint32_t hash(const std::string& input)
{
    const char* data = input.c_str();
    uint32_t length = input.size();
    uint32_t result = static_cast<uint32_t>(2166136261UL);
    for (; length; --length)
    {
        result ^= static_cast<uint32_t>(*data++);
        result *= static_cast<uint32_t>(16777619UL);
    }
    return result;
}

class ErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
    ErrorCollector() : _error_count(0)
    {
    }

    void AddError(const std::string& filename, int line, int column, const std::string& message) override
    {
        _stream << "[" << filename << " (" << line << "," << column << ")] " << message << std::endl;
        _error_count++;
    }

    std::string getAllErrors()
    {
        return _stream.str();
    }

    int getErrorCount()
    {
        return _error_count;
    }

private:
    std::stringstream _stream;
    int _error_count;
};

class MessageTypeStore::Private
{
public:
    std::unordered_map<uint, const google::protobuf::Message*> message_types;
    std::unordered_map<const google::protobuf::Descriptor*, uint> message_type_mapping;

    std::shared_ptr<ErrorCollector> error_collector;
    std::shared_ptr<google::protobuf::compiler::DiskSourceTree> source_tree;
    std::shared_ptr<google::protobuf::compiler::Importer> importer;
    std::shared_ptr<google::protobuf::DynamicMessageFactory> message_factory;
};

Arcus::MessageTypeStore::MessageTypeStore() : d(new Private)
{
}

Arcus::MessageTypeStore::~MessageTypeStore()
{
}

bool Arcus::MessageTypeStore::hasType(uint32_t type_id) const
{
    if (d->message_types.find(type_id) != d->message_types.end())
    {
        return true;
    }

    return false;
}

bool Arcus::MessageTypeStore::hasType(const std::string& type_name) const
{
    uint32_t type_id = hash(type_name);
    return hasType(type_id);
}

MessagePtr Arcus::MessageTypeStore::createMessage(uint32_t type_id) const
{
    if (! hasType(type_id))
    {
        return MessagePtr();
    }

    return MessagePtr(d->message_types[type_id]->New());
}

MessagePtr Arcus::MessageTypeStore::createMessage(const std::string& type_name) const
{
    uint32_t type_id = hash(type_name);
    return createMessage(type_id);
}

uint32_t Arcus::MessageTypeStore::getMessageTypeId(const MessagePtr& message)
{
    return hash(message->GetTypeName());
}

std::string Arcus::MessageTypeStore::getErrorMessages() const
{
    return d->error_collector->getAllErrors();
}

bool Arcus::MessageTypeStore::registerMessageType(const google::protobuf::Message* message_type)
{
    uint32_t type_id = hash(message_type->GetTypeName());

    if (hasType(type_id))
    {
        return false;
    }

#ifdef ARCUS_DEBUG
    std::cerr << "'Manually' adding message type: " << message_type->GetTypeName() << " = " << type_id << std::endl;
#endif // ARCUS_DEBUG

    d->message_types[type_id] = message_type;
    d->message_type_mapping[message_type->GetDescriptor()] = type_id;

    return true;
}

bool Arcus::MessageTypeStore::registerAllMessageTypes(const std::string& file_name)
{
#ifdef ARCUS_DEBUG
    std::cerr << "Reading message prototypes from: " << file_name << std::endl;
#endif // ARCUS_DEBUG

    if (! d->importer)
    {
        d->error_collector = std::make_shared<ErrorCollector>();
        d->source_tree = std::make_shared<google::protobuf::compiler::DiskSourceTree>();
#ifndef _WIN32
        d->source_tree->MapPath("/", "/");
#else
        // Because of silly DiskSourceTree, we need to make sure absolute paths to
        // the protocol file are properly mapped.
        for (auto letter : std::string("abcdefghijklmnopqrstuvwxyz"))
        {
            std::string lc(1, letter);
            std::string uc(1, toupper(letter));
            d->source_tree->MapPath(lc + ":/", lc + ":\\");
            d->source_tree->MapPath(uc + ":/", uc + ":\\");
        }
#endif
        d->importer = std::make_shared<google::protobuf::compiler::Importer>(d->source_tree.get(), d->error_collector.get());
    }

    auto descriptor = d->importer->Import(file_name);
    if (d->error_collector->getErrorCount() > 0)
    {
#ifdef ARCUS_DEBUG
        std::cerr << d->error_collector->getAllErrors() << std::endl;
#endif // ARCUS_DEBUG
        return false;
    }

    if (! d->message_factory)
    {
        d->message_factory = std::make_shared<google::protobuf::DynamicMessageFactory>();
    }

    for (int i = 0; i < descriptor->message_type_count(); ++i)
    {
        auto message_type_descriptor = descriptor->message_type(i);
        auto message_type = d->message_factory->GetPrototype(message_type_descriptor);
        uint32_t type_id = hash(message_type->GetTypeName());
#ifdef ARCUS_DEBUG
        std::cerr << message_type->GetTypeName() << ": " << type_id << std::endl;
#endif // ARCUS_DEBUG

        d->message_types[type_id] = message_type;
        d->message_type_mapping[message_type_descriptor] = type_id;
    }

    return true;
}

void Arcus::MessageTypeStore::dumpMessageTypes()
{
    for (auto type : d->message_types)
    {
        std::cout << "Type ID: " << type.first << " Type Name: " << type.second->GetTypeName() << std::endl;
    }
}
