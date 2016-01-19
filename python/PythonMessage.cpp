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

#include "PythonMessage.h"

#include <Python.h>

#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>

using namespace Arcus;
using namespace google::protobuf;

PythonMessage::PythonMessage(google::protobuf::Message* message)
{
    _message = message;
    _reflection = message->GetReflection();
    _descriptor = message->GetDescriptor();
}

Arcus::PythonMessage::PythonMessage(const MessagePtr& message)
{
    _shared_message = message;
    _message = message.get();
    _reflection = message->GetReflection();
    _descriptor = message->GetDescriptor();
}

PythonMessage::~PythonMessage()
{
}

std::string Arcus::PythonMessage::getTypeName() const
{
    return  _message->GetTypeName();
}

MessagePtr Arcus::PythonMessage::getSharedMessage() const
{
    return _shared_message;
}

bool Arcus::PythonMessage::hasField(const std::string& field_name)
{
    auto field = _descriptor->FindFieldByName(field_name);
    return bool(field);
}

PyObject* Arcus::PythonMessage::getField(const std::string& field_name)
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return nullptr;
    }

    switch(field->type())
    {
        case FieldDescriptor::TYPE_FLOAT:
            return PyFloat_FromDouble(_reflection->GetFloat(*_message, field));
        case FieldDescriptor::TYPE_DOUBLE:
            return PyFloat_FromDouble(_reflection->GetDouble(*_message, field));
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_FIXED32:
        case FieldDescriptor::TYPE_SINT32:
        case FieldDescriptor::TYPE_SFIXED32:
            return PyLong_FromLong(_reflection->GetInt32(*_message, field));
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_FIXED64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            return PyLong_FromLongLong(_reflection->GetInt64(*_message, field));
        case FieldDescriptor::TYPE_UINT32:
            return PyLong_FromUnsignedLong(_reflection->GetUInt32(*_message, field));
        case FieldDescriptor::TYPE_UINT64:
            return PyLong_FromUnsignedLongLong(_reflection->GetUInt64(*_message, field));
        case FieldDescriptor::TYPE_BOOL:
            if(_reflection->GetBool(*_message, field))
            {
                Py_RETURN_TRUE;
            }
            else
            {
                Py_RETURN_FALSE;
            }
        case FieldDescriptor::TYPE_BYTES:
            return PyBytes_FromString(_reflection->GetString(*_message, field).c_str());
        case FieldDescriptor::TYPE_STRING:
            return PyUnicode_FromString(_reflection->GetString(*_message, field).c_str());
        default:
            PyErr_SetString(PyExc_ValueError, "Could not handle value of field");
            return nullptr;
    }
}

void Arcus::PythonMessage::setField(const std::string& field_name, PyObject* value)
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return;
    }

    switch(field->type())
    {
        case FieldDescriptor::TYPE_FLOAT:
            _reflection->SetFloat(_message, field, PyFloat_AsDouble(value));
            break;
        case FieldDescriptor::TYPE_DOUBLE:
            _reflection->SetDouble(_message, field, PyFloat_AsDouble(value));
            break;
        case FieldDescriptor::TYPE_INT32:
        case FieldDescriptor::TYPE_SFIXED32:
        case FieldDescriptor::TYPE_FIXED32:
        case FieldDescriptor::TYPE_SINT32:
            _reflection->SetInt32(_message, field, PyLong_AsLong(value));
            break;
        case FieldDescriptor::TYPE_INT64:
        case FieldDescriptor::TYPE_FIXED64:
        case FieldDescriptor::TYPE_SINT64:
        case FieldDescriptor::TYPE_SFIXED64:
            _reflection->SetInt64(_message, field, PyLong_AsLongLong(value));
            break;
        case FieldDescriptor::TYPE_UINT32:
            _reflection->SetUInt32(_message, field, PyLong_AsUnsignedLong(value));
            break;
        case FieldDescriptor::TYPE_UINT64:
            _reflection->SetUInt64(_message, field, PyLong_AsUnsignedLongLong(value));
            break;
        case FieldDescriptor::TYPE_BOOL:
            if(value == Py_True)
            {
                _reflection->SetBool(_message, field, true);
            }
            else
            {
                _reflection->SetBool(_message, field, false);
            }
            break;
        case FieldDescriptor::TYPE_BYTES:
            _reflection->SetString(_message, field, std::string(PyBytes_AsString(value)));
            break;
        case FieldDescriptor::TYPE_STRING:
            _reflection->SetString(_message, field, PyUnicode_AsUTF8(value));
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Could not handle value of field");
            break;
    }
}

PythonMessage* Arcus::PythonMessage::addRepeatedField(const std::string& field_name)
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return nullptr;
    }

    Message* message = _reflection->AddMessage(_message, field);
    return new PythonMessage(message);
}

int PythonMessage::repeatedFieldCount(const std::string& field_name)
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return -1;
    }

    return _reflection->FieldSize(*_message, field);
}
