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

// Convert a python str object to a std::string.
%MappedType std::string
{
%TypeHeaderCode
#include <string>
%End

%ConvertFromTypeCode
    // convert an std::string to a Python (unicode) string
    PyObject* newstring;
    newstring = PyUnicode_DecodeUTF8(sipCpp->c_str(), sipCpp->length(), NULL);
    if(newstring == NULL) {
        PyErr_Clear();
        newstring = PyBytes_FromString(sipCpp->c_str());
    }
    return newstring;
%End

%ConvertToTypeCode
    // Allow a Python string (or a unicode string) whenever a string is
    // expected.
    // If argument is a Unicode string, just decode it to UTF-8
    // If argument is a Python string, assume it's UTF-8
    if (sipIsErr == NULL)
        return (PyBytes_Check(sipPy) || PyUnicode_Check(sipPy));

    if (sipPy == Py_None)
    {
        *sipCppPtr = new std::string;
        return 1;
    }

    if (PyUnicode_Check(sipPy))
    {
        PyObject* s = PyUnicode_AsEncodedString(sipPy, "UTF-8", "");
        *sipCppPtr = new std::string(PyBytes_AS_STRING(s));
        Py_DECREF(s);
        return 1;
    }

    if (PyBytes_Check(sipPy))
    {
        *sipCppPtr = new std::string(PyBytes_AS_STRING(sipPy));
        return 1;
    }
    return 0;
%End
};

// Convert a MessagePtr (aka std::shared_ptr<google::protobuf::Message>) to a PythonMessage*
%MappedType MessagePtr
{
%TypeHeaderCode
#include <memory>
#include "PythonMessage.h"
%End

%ConvertFromTypeCode
    // Convert a Protobuf message to a Python object
    if(!(*sipCpp))
    {
        PyErr_SetString(PyExc_ValueError, "Unknown message type");
        return NULL;
    }

    const sipTypeDef* message_type = sipFindType("PythonMessage");

    PythonMessage* message = new PythonMessage(*sipCpp);

    sipTransferObj = Py_None;

    PyObject* msg = sipConvertFromType(message, message_type, sipTransferObj);
    if(!msg)
    {
        delete message;
        return NULL;
    }

    return msg;
%End

%ConvertToTypeCode
    // Convert a Python object to a Protobuf message
    const sipTypeDef* message_type = sipFindType("PythonMessage");

    if(sipIsErr == NULL)
    {
        return sipCanConvertToType(sipPy, message_type, SIP_NOT_NONE);
    }

    if(sipCanConvertToType(sipPy, message_type, SIP_NOT_NONE))
    {
        int iserr = 0;
        int state = 0;

        PythonMessage* message = reinterpret_cast<PythonMessage*>(sipConvertToType(sipPy, message_type, NULL, 0, &state, &iserr));

        if(iserr != 0)
        {
            PyErr_SetString(PyExc_ValueError, "Could not convert to Message");
            return 0;
        }

        MessagePtr msg = message->getSharedMessage();
        *sipCppPtr = new MessagePtr(msg);

        sipReleaseType(message, message_type, state);
    }

    return sipGetState(sipTransferObj);
%End

};

%UnitCode
#include "Types.h"
%End

namespace SocketState
{
    enum SocketState
    {
        Initial,
        Connecting,
        Connected,
        Opening,
        Listening,
        Closing,
        Closed,
        Error
    };
};
