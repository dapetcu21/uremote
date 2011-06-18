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

#include "URClient.h"
#include <cstdio>

int main(int argc, char * argv[])
{
	URClient client;
	if (argc<2)
		client.setHostname("localhost");
	else
		client.setHostname(argv[1]);
	if (argc<3)
		client.setPort("4432");
	else
		client.setPort(argv[2]);
	try { client.start(); }
	catch (std::string ex)
		{printf("error: %s\n",ex.c_str());};
	URField f1, f2;
	client.beginPacket(0xAF);
	f1.setTag(0x01);
	f1.setInt32(0xa1a2a3a4);
	client.addField(f1);
	f2.setTag(0x02);
	f2.setString("hello world");
	client.addField(f2);
	try {client.endPacket(0,NULL,0); }
	catch (std::string ex)
		{printf("error: %s\n",ex.c_str());};
	client.stop();
	return 0;
}
