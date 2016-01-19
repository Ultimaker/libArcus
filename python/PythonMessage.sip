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

class PythonMessage
{
    %TypeHeaderCode
    #include "PythonMessage.h"
    %End

public:
    std::string getTypeName() const;

    bool __hasattr__(const std::string&) /AllowNone/;
    %MethodCode
    if(sipCpp->hasField(*a0))
    {
        Py_RETURN_TRUE;
    }
    else
    {
        Py_RETURN_FALSE;
    }
    %End

    SIP_PYOBJECT __getattr__(const std::string&) /AllowNone/;
    %MethodCode
    return sipCpp->getField(*a0);
    %End

    void __setattr__(const std::string&, SIP_PYOBJECT) /AllowNone/;
    %MethodCode
    sipCpp->setField(*a0, a1);
    %End

    void __delattr__(const std::string&);
    %MethodCode
    return 0;
    %End

    PythonMessage* addRepeatedField(const std::string& field_name);
    int repeatedFieldCount(const std::string& field_name);

private:
    PythonMessage();
    virtual ~PythonMessage();
};
