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

#ifndef URCLIENT_H
#define URCLIENT_H

#include "URCommon.h"
#include "URField.h"

class URClient
{
private:
	ursock_t sockf;
	bool running;
	std::string host,_port,error_message;
	
	const URField ** tags;
	int nrt;
	int tm;
	uint8_t signature;
public:
	URClient();
	~URClient();
	
	bool isRunning() { return running; };
	void setRunning(bool val) throw(std::string); 
	void start() throw(std::string) { setRunning(true); };
	void stop(){ setRunning(false); };
	void restart() { setRunning(false); setRunning(true); }
	std::string hostname() {return host;};
	std::string port() {return _port;};
	void setHostname (const std::string & hn) throw(std::string); 
	void setPort(const std::string & prt) throw(std::string); 
	void setDestination(const std::string & hn,const std::string & prt) throw(std::string); 
	std::string errorMessage() { return error_message; }
	
	enum flags
	{
		TagDescription = 1,
		ShortLength = 2 //it's in fact longer than the default length, but... you know.. short.. 16 bit
	};
	
	void sendPacket(uint8_t signature, const URField * const * fields, int numfields, int flags, void * description, size_t descr_size) throw(std::string); //use this ooor...
	void sendPacket(uint8_t signature, const URField * const * fields, int numfields, int flags) throw(std::string) {
		 sendPacket(signature,fields,numfields,flags,NULL,0); }
	//use these:
	void beginPacket(uint8_t signature);
	void addField(const URField & field);
	void endPacket(int flags, void * description, size_t descr_size) throw(std::string);
	void endPacket(int flags) throw(std::string) {
		 endPacket(flags,NULL,0); }
};

#endif URCLIENT_H