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

#ifndef URCOMMON_H
#define URCOMMON_H

#include <string>

#ifdef _MSC_VER //stupid microsoft
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	typedef SOCKET ursock_t;
#else
	typedef int ursock_t;
#endif

#endif URCOMMON_H