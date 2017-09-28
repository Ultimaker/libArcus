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

bool Arcus::PythonMessage::__hasattr__(const std::string& field_name) const
{
    auto field = _descriptor->FindFieldByName(field_name);
    return bool(field);
}

PyObject* Arcus::PythonMessage::__getattr__(const std::string& field_name) const
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
        {
            std::string data = _reflection->GetString(*_message, field);
            return PyBytes_FromStringAndSize(data.c_str(), data.size());
        }
        case FieldDescriptor::TYPE_STRING:
            return PyUnicode_FromString(_reflection->GetString(*_message, field).c_str());
        case FieldDescriptor::TYPE_ENUM:
            return PyLong_FromLong(_reflection->GetEnumValue(*_message, field));
        default:
            PyErr_SetString(PyExc_ValueError, "Could not handle value of field");
            return nullptr;
    }
}

void Arcus::PythonMessage::__setattr__(const std::string& field_name, PyObject* value)
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
        {
            Py_buffer buffer;
            PyObject_GetBuffer(value, &buffer, PyBUF_SIMPLE);

            std::string str(reinterpret_cast<char*>(buffer.buf), buffer.len);
            _reflection->SetString(_message, field, str);
            break;
        }
        case FieldDescriptor::TYPE_STRING:
            _reflection->SetString(_message, field, PyUnicode_AsUTF8(value));
            break;
        case FieldDescriptor::TYPE_ENUM:
        {
            if(PyUnicode_Check(value))
            {
                auto enum_value = _descriptor->FindEnumValueByName(PyUnicode_AsUTF8(value));
                _reflection->SetEnum(_message, field, enum_value);
            }
            else
            {
                _reflection->SetEnumValue(_message, field, PyLong_AsLong(value));
            }
            break;
        }
        default:
            PyErr_SetString(PyExc_ValueError, "Could not handle value of field");
            break;
    }
}

PythonMessage* Arcus::PythonMessage::addRepeatedMessage(const std::string& field_name)
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

int PythonMessage::repeatedMessageCount(const std::string& field_name) const
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return -1;
    }

    return _reflection->FieldSize(*_message, field);
}

PythonMessage* Arcus::PythonMessage::getMessage(const std::string& field_name)
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return nullptr;
    }
    return new PythonMessage(_reflection->MutableMessage(_message, field));
}

PythonMessage* Arcus::PythonMessage::getRepeatedMessage(const std::string& field_name, int index)
{
    auto field = _descriptor->FindFieldByName(field_name);
    if(!field)
    {
        PyErr_SetString(PyExc_AttributeError, field_name.c_str());
        return nullptr;
    }

    if(index < 0 || index > _reflection->FieldSize(*_message, field))
    {
        PyErr_SetString(PyExc_IndexError, field_name.c_str());
        return nullptr;
    }

    return new PythonMessage(_reflection->MutableRepeatedMessage(_message, field, index));
}

int Arcus::PythonMessage::getEnumValue(const std::string& enum_value) const
{
    auto field = _descriptor->FindEnumValueByName(enum_value);
    if(!field)
    {
        return -1;
    }

    return field->number();
}
