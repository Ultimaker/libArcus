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

#include <iostream>

#include "TestServer.h"

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " [address] [port]" << std::endl;
        return 1;
    }

    std::string address = argv[1];
    int port = atoi(argv[2]);

    TestServer server(address, port);
    return server.run() ? 0 : 1;
}
