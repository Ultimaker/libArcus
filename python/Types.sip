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

%MappedType Arcus::MessagePtr
{
%TypeHeaderCode
#include <memory>
#include <google/protobuf/message.h>
%End

%ConvertFromTypeCode
    // Convert a Protobuf message to a Python object
    //PyObject* object;
    Py_RETURN_NOTIMPLEMENTED;
%End

%ConvertToTypeCode
    // Convert a Python object to a Protobuf message
//     if(PyObject_HasAttr(sipPy, "
    //Py_RETURN_NOTIMPLEMENTED;
    return 0;
%End

};

%UnitCode
#include "Types.h"
%End

namespace Arcus
{
    namespace SocketState
    {
        enum State
        {
            Initial, ///< Created, not running.
            Connecting, ///< Connecting to an address and port.
            Connected, ///< Connected and capable of sending and receiving messages.
            Opening, ///< Opening for incoming connections.
            Listening, ///< Listening for incoming connections.
            Closing, ///< Closing down.
            Closed, ///< Closed, not running.
            Error ///< An error happened.
        };
    };
};
