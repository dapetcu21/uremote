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
#include "URCommonPrivate.h"

URClient::URClient() : running(false), host(""), _port(""), tags(NULL), nrt(0), tm(0), signature(0)
{
}

URClient::~URClient()
{
	stop();
}

void URClient::setRunning(bool val) throw(std::string)
{
	if (val==running) return;
	running = val;
	if (val)
	{
		struct addrinfo hints, *servinfo, *p;
	    int rv;
		
		URSetup();
		error_message = "";

	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_DGRAM;

	    if ((rv = getaddrinfo(host.c_str(), _port.c_str(), &hints, &servinfo)) != 0) {
			error_message = URStringForError(rv);
			running=false;
			URCleanup();
			throw error_message;
	    }

	    for(p = servinfo; p != NULL; p = p->ai_next) {
	        if ((sockf = socket(p->ai_family, p->ai_socktype,
								 p->ai_protocol)) == BAD_SOCK) {
				continue;
	        }
	        break;
	    }

	    if (p == NULL) {
			error_message = "can't open any of the sockets getaddrinfo() returned";
			running=false;
			URCleanup();
			throw error_message;
	    }
	
		int broadcast = 1;
		if (setsockopt(sockf, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
		{
			error_message = URStringForError(ERRNO);
			running=false;
			close(sockf); 
			URCleanup();
			throw error_message;
		}

		if (connect(sockf,p->ai_addr,p->ai_addrlen)==-1)
		{
			error_message = URStringForError(ERRNO);
			running=false;
			close(sockf); 
			URCleanup();
			throw error_message;
		}

	    freeaddrinfo(servinfo);
		URSetBlocking(sockf,false);
	} else 
	{
		close(sockf);
		URCleanup();
	}
}

void URClient::setHostname(const std::string & hn) throw(std::string)
{
	if (hn==host) return;
	bool resume = running;
	if (resume)
		stop();
	host = hn;
	if (resume)
		start();
}

void URClient::setPort(const std::string & prt) throw(std::string)
{
	if (_port==prt) return;
	bool resume = running;
	if (resume)
		stop();
	_port = prt;
	if (resume)
		start();
}

void URClient::setDestination(const std::string & hn,const std::string & prt) throw(std::string)
{
	if (_port==prt && hn==host) return;
	bool resume = running;
	if (resume)
		stop();
	_port = prt;
	host = hn;
	if (resume)
		start();
}

void URClient::sendPacket(uint8_t signature, const URField * const * tags, int numtags, int flags, void * description, size_t descr_size) throw(std::string)
{
	if (!running) return;
	size_t size = 1;
	size_t lensize = 1;
	if (flags & ShortLength)
		lensize = 2;
	for (int i=0; i<numtags; i++)
		size+=tags[i]->length()+(tags[i]->knownLength()?0:lensize);
	if (flags & TagDescription)
		size+=descr_size;
	else
		size+=numtags;
	uint8_t * data = new uint8_t[size];
	data[0] = signature;
	size_t p = 1;
	if (flags & TagDescription)
	{
		memcpy(data+p,description,descr_size);
		p+=descr_size;
	}
	for (int i=0; i<numtags; i++)
	{
		size_t l = tags[i]->length();
		if (!(flags & TagDescription))
			data[p++] = tags[i]->tag();
		if (!(tags[i]->knownLength()))
		{
			if (flags & ShortLength)
				(*((uint16_t*)(data+p))) = htons((uint16_t)l);
			else
				data[p] = (uint8_t)l;
			p+=lensize;
		}
		memcpy(data+p,tags[i]->data(),l);
		p+=l;
	}
	if (send(sockf,(datap)data,p,0)==-1)
	{
		delete[] data;
		error_message = URStringForError(ERRNO);
		throw error_message;
	}
	delete[] data;
}

void URClient::beginPacket(uint8_t sig)
{
	signature = sig;
	tm = 32;
	tags = (const URField **)malloc(sizeof(const URField * )*tm);
	nrt = 0;
}

void URClient::addField(const URField & tag)
{
	if (nrt>=tm)
	{
		tm = (tm<<1);
		tags = (const URField **)realloc(tags,sizeof(const URField *)*tm);
	}
	tags[nrt] = &tag;
	nrt++;
}

void URClient::endPacket(int flags, void * description, size_t descr_size) throw(std::string)
{
	sendPacket(signature,tags,nrt,flags,description,descr_size);
	free(tags);
	tags = NULL;
	tm = nrt = 0;
}