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

class PythonMessage
{
    %TypeHeaderCode
    #include "PythonMessage.h"
    %End

public:
    virtual ~PythonMessage();

    std::string getTypeName() const;

    bool __hasattr__(const std::string&) const /AllowNone/;
    PyObject* __getattr__(const std::string&) const /AllowNone, HoldGIL/;
    void __setattr__(const std::string&, PyObject*) /AllowNone/;
    %MethodCode
    sipCpp->__setattr__(*a0, a1);
    %End

    void __delattr__(const std::string&);
    %MethodCode
    PyErr_SetString(PyExc_NotImplementedError, "__delattr__ not supported on messages.");
    return 0;
    %End

    PythonMessage* addRepeatedMessage(const std::string& field_name) /TransferBack/;
    int repeatedMessageCount(const std::string& field_name) const;
    PythonMessage* getRepeatedMessage(const std::string& field_name, int index) /TransferBack/;
    PythonMessage* getMessage(const std::string& field_name) /TransferBack/;

    int getEnumValue(const std::string& enum_value) const;

private:
    PythonMessage();
};
