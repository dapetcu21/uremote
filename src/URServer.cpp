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
#include "URCommonPrivate.h"

#ifdef _WIN32
#define mutex_lock(x) WaitForSingleObject(x,INFINITE)
#define mutex_unlock(x) ReleaseMutex(x)
#define mutex_init(x) x = CreateMutex(NULL,FALSE,NULL)
#define mutex_destroy(x) CloseHandle(x)
#else 
#define mutex_lock(x) pthread_mutex_lock(&x)
#define mutex_unlock(x) pthread_mutex_unlock(&x)
#define mutex_init(x)  do { pthread_mutexattr_t atr; pthread_mutexattr_init(&atr); pthread_mutexattr_settype(&atr, PTHREAD_MUTEX_RECURSIVE); pthread_mutex_init(&x,&atr); pthread_mutexattr_destroy(&atr); } while(false)
#define mutex_destroy(x) pthread_mutex_destroy(&x)
#endif

URServer::URServer() : running(false), error_message(""), _port("45432"), sig(0x00), slen(false), detached(false), shouldRun(false), nri(0)
{
	mutex_init(mutex);
	mutex_init(startmutex);
#ifdef _WIN32
	intr = WSACreateEvent();
#else
	pipe(fdpipes);
	URSetBlocking(fdpipes[0],false);
#endif
}

URServer::~URServer()
{
	stop();
	mutex_destroy(mutex);
	mutex_destroy(startmutex);
#ifdef _WIN32
	WSACloseEvent(intr);
#else
	close(fdpipes[0]);
	close(fdpipes[1]);
#endif
}

void URServer::setRunning(bool val) throw(std::string)
{
	mutex_lock(startmutex);
	if (val == running) return;
	running = val;
	if (val)
	{
		struct addrinfo hints, *servinfo, *p;
	    int rv;

		URSetup();

	    memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
	    hints.ai_socktype = SOCK_DGRAM;
	    hints.ai_flags = AI_PASSIVE;

	    if ((rv = getaddrinfo(NULL, _port.c_str(), &hints, &servinfo)) != 0) {
			error_message = URStringForError(rv);
			running=false;
			URCleanup();
			throw error_message;
	    }

		nri = 0;

	    for(p = servinfo; p != NULL; p = p->ai_next) {
			ursock_t sockfd;
	        if (((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)),sockfd) == BAD_SOCK) {
				continue;
	        }

	        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	            close(sockfd);
				continue;
	        }

			URSetBlocking(sockfd,false);
#ifdef _WIN32
			WSAEVENT evt = WSACreateEvent();
			WSAEventSelect(sockfd,evt,FD_READ);
			events[nri] = evt;
#endif
			socks[nri++] = sockfd;
	    }

		if (!nri) {
			error_message = "no interface to bind to";
			running=false;
			URCleanup();
			throw error_message;
	    }

#ifdef _WIN32
		events[nri] = intr;
#endif

	    freeaddrinfo(servinfo);
	} else {
		if (detached)
		{
			shouldRun = false;
			signalWaitingForPacket();
			waitForWorkerThread();
			detached = false;
			
#ifdef _WIN32
			for (int i=0; i<nri; i++)
				WSACloseEvent(events[i]);
#endif

			for (int i=0; i<nri; i++)
				close(socks[i]);
		}
	}
	mutex_unlock(startmutex);
}

void URServer::setPort(std::string port)
{
	bool resume = running;
	if (resume)
		stop();
	_port = port;
	if (resume)
		start();
}

#define fail { for (int j=0; j<i; j++) delete fields[j]; free(fields); return; }

void URServer::processPacket(uint8_t * data, size_t len)
{
	if (len<1) return;
	if (!acceptsSignature(data[0])) return;
	size_t p = 1;
	len--;
	size_t consumed = 0;
	uint8_t * tags  = processDescription(data+p,len,consumed);
	if (tags)
		{ p+=consumed; len-=consumed; }
	int i=0;
	int nr = 64;
	URField ** fields = (URField**)malloc(sizeof(URField*)*nr);
	while((len>0) && (tags?tags[i]:1))
	{
		uint8_t tag;
		if (tags)
			tag = tags[i];
		else
		{
			if (len<1) fail;
			tag = data[p];
			p++; len--;
		}
		size_t size = lengthForTag(tag);
		if (!size)
		{
			if (slen) 
			{
				if (len<2) fail;
				size = ntohs(*((uint16_t*)(data+p)));
				p+=2; len-=2;
			} else {
				if (len<1) fail;
				size = data[p];
				p++; len--;
			}
		}
		if (len<size) fail;
		if (i>=nr)
		{
			nr = nr<<1;
			fields = (URField**)realloc(fields,sizeof(URField*)*nr);
		}
		fields[i] = new URField;
		fields[i]->setTag(tag);
		fields[i]->setData(data+p,size);
		p+=size; len-=size;
		i++;
	}
	recievedPacket(data[0], fields, i);
	fail;
}

#define BUFLEN 4096

void URServer::processPendingPackets()
{
	if (!running) return;
	mutex_lock(mutex);
	int numbytes;
    size_t addr_len;
    struct sockaddr_storage their_addr;
	addr_len = sizeof their_addr;
	uint8_t data[BUFLEN];
	for (int i=0; i<nri; i++)
	{
		while (1)
		{
    		if ((numbytes = recvfrom(socks[i], (datap)data, BUFLEN , 0, (struct sockaddr *)&their_addr, (socklen_t*)&addr_len)) == -1) 
			{
				//if(ERRNO!=EWOULDBLOCK && ERRNO!=EAGAIN) 
				//	;
				break;
			} else
				processPacket(data,numbytes);
		}
	}
	mutex_unlock(mutex);
}

WORKERTHREAD_DECLARATION
{
	URServer * ths = (URServer*)arg;
	do
	{
		ths->processPendingPackets();
		ths->waitForPacket();
	} while(ths->shouldRun);
	return 0;
}


void URServer::detachThread()
{
	mutex_lock(startmutex);
	if (detached || !running) 
	{
		mutex_unlock(startmutex);	
		return;
	}
	detached = true;
	shouldRun = true;
	#ifdef _WIN32
	CreateThread( NULL, 0, worker_thread, this, 0, NULL); 
	#else
	pthread_create(&thread,NULL,worker_thread,this);
	#endif
	mutex_unlock(startmutex);
}

void URServer::signalWaitingForPacket()
{
	#ifdef _WIN32
	WSASetEvent(intr);
	#else
	uint8_t data = 1;
	write(fdpipes[1],&data,sizeof(uint8_t));
	#endif
}


void URServer::waitForPacket()
{
	if (!running) return;
	#ifdef _WIN32
	WSAResetEvent(intr);
	WSAWaitForMultipleEvents(nri+1,events,FALSE,WSA_INFINITE,FALSE);
	WSAResetEvent(intr);
	#else
	fd_set set;
	FD_ZERO(&set);
	int max = fdpipes[0];
	FD_SET(fdpipes[0],&set);
	for (int i=0; i<nri; i++)
	{
		if (socks[i]>max)
			max = socks[i];
		FD_SET(socks[i],&set);
	}
	select(max+1,&set,NULL,NULL,NULL);
	uint8_t buf[10];
	while (read(fdpipes[0],buf,10)>0) {};
	#endif
}


void URServer::waitForWorkerThread()
{
	mutex_lock(startmutex);
	if (detached) 
	{
		#ifdef _WIN32
		WaitForSingleObject(thread,INFINITE);
		#else
		pthread_join(thread,NULL);
		#endif
	}
	mutex_unlock(startmutex);
}

//stuff to override
#define UNUSED_ARGUMENT(x) (void)x

bool URServer::acceptsSignature(uint8_t signature)
{
	return  signature == sig;
}

uint8_t * URServer::processDescription(uint8_t * data, size_t len, size_t & descr_len)
{
	UNUSED_ARGUMENT(data);
	UNUSED_ARGUMENT(len);
	UNUSED_ARGUMENT(descr_len);
	return NULL;
}

size_t URServer::lengthForTag(int tag)
{
	UNUSED_ARGUMENT(tag);
	return 0;
}

void URServer::recievedPacket(uint8_t signature, const URField * const * fields, int nrfields)
{
	UNUSED_ARGUMENT(signature);
	UNUSED_ARGUMENT(fields);
	UNUSED_ARGUMENT(nrfields);
}