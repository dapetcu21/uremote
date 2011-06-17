/*	This file is part of URemote.

    Foobar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with URemote.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef URCOMMONPRIVATE_H
#define URCOMMONPRIVATE_H

#include "URCommon.h"

#ifdef _WIN32

#include <windows.h>

#define ERRNO WSAGetLastError()
#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#ifndef EAGAIN
#define EAGAIN WSAEWOULDBLOCK
#endif
#define BAD_SOCK INVALID_SOCKET
#define close(x) closesocket(x)
#define datap char* 

#ifdef _MSC_VER
#pragma comment (lib, "Ws2_32.lib")
#endif

#else
 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define BAD_SOCK -1
#define ERRNO errno
#define datap void*

#endif

void URSetBlocking(ursock_t sock, bool off);
void URSetup();
void URCleanup();
std::string URStringForError(int err);

#endif 