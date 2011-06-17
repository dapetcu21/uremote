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

#include "URCommonPrivate.h"

#ifdef _WIN32
int URRefcount = 0;
void URSetBlocking(ursock_t sock, bool on)
{
	u_long iMode = 1;
	if (on)
		iMode = 0;
	else 
		iMode = 1;
	ioctlsocket(sock, FIONBIO, &iMode);
}

void URSetup()
{
	if (!URRefcount)
	{
		WORD sockVersion;
		WSADATA wsaData;
		sockVersion = MAKEWORD(2, 2);
		WSAStartup(sockVersion, &wsaData);
	}
	URRefcount++;
}
void URCleanup()
{
	URRefcount--;
	if (!URRefcount)
		WSACleanup();
}

std::string URStringForError(int err)
{
	LPSTR pBuffer = NULL;
	FormatMessageA(	FORMAT_MESSAGE_ALLOCATE_BUFFER,
					NULL, 
					err,
					0,
					(LPSTR)&pBuffer, 
					0,
					NULL );
	return std::string((char*)pBuffer);
}

#else
void URSetBlocking(ursock_t sock, bool on)
{
	if (on)
		fcntl(sock, F_SETFL, 0);
	else
		fcntl(sock, F_SETFL, O_NONBLOCK);
}
void URSetup() {};
void URCleanup() {};
std::string URStringForError(int err)
{
	std::string str(gai_strerror(err));
	return str;
}
#endif
