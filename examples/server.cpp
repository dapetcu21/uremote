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

#include "URServer.h"
#include <cstdio>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

class CustomServer : public URServer
{
	virtual void recievedPacket(uint8_t signature, const URField * const * fields, int nrfields)
	{
		printf("%2x: ",signature);
		for (int i=0; i<nrfields; i++)
		{
			switch (fields[i]->tag())
			{
				case 0x01:
				printf("%8x ",fields[i]->asInt32());
				break;
				case 0x02:
				printf("%s",fields[i]->asString().c_str());
			}
		}
		printf("\n");
	}
};

int main(int argc, char * argv[])
{
	CustomServer srv;
	srv.setSignature(0xAF);
	if (argc<2)
		srv.setPort("4432"); //and yes, 4432 is my favorite number, the problem is I use it as a port number in anything network-related I write :D
	else
		srv.setPort(argv[1]);
	srv.start();
	srv.detachThread();
#ifdef _WIN32
	Sleep(5000);
#else
	sleep(5);
#endif
	srv.stop();
	return 0;
}
