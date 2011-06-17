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

#ifndef URSERVER_H
#define URSERVER_H

#include "URCommon.h"
#include "URField.h"

#ifdef _WIN32
#include <windows.h>
#define THREADHANDLE HANDLE
#define MUTEXHANDLE HANDLE
#define WORKERTHREAD_DECLARATION DWORD WINAPI worker_thread(void * arg)
#else
#include <pthread.h>
#define THREADHANDLE pthread_t
#define MUTEXHANDLE pthread_mutex_t
#define WORKERTHREAD_DECLARATION void * worker_thread(void * arg)
#endif

class URServer 
{
private:
	bool running;
	std::string error_message,_port;
	
	uint8_t sig;
	bool slen;
	bool detached;
	volatile bool shouldRun;
	ursock_t socks[32]; //32 network interfaces is more than enough
	int nri;
	
	THREADHANDLE thread;
	MUTEXHANDLE mutex,startmutex;
	
	friend WORKERTHREAD_DECLARATION;
	
	#ifdef _WIN32
	WSAEVENT intr;
	WSAEVENT events[32];
	#else
	int fdpipes[2];
	#endif
	
public:
	URServer();
	virtual ~URServer();
	
	std::string errorMessage() { return error_message; }
	
	//not thread safe.. you should stop the server before changing these
	uint8_t signature() { return sig; }; 
	void setSignature(uint8_t signature) { sig = signature; }
	bool shortLength() { return slen; }
	void setShortLength(bool l) { slen = l; }
	
	//thread safe
	void setPort(std::string port); //this automatically restarts the server if it is already running
	std::string port() { return _port; }
	bool isRunning() { return running; } 
	void setRunning(bool val) throw(std::string); 
	void start() throw(std::string) { setRunning(true); }
	void stop() { setRunning(false); }
	void restart() throw(std::string) { setRunning(false); setRunning(true); }
	
	void waitForPacket(); //just a select() call
	void processPendingPackets(); //call this as often as you can.. thread-safe.. or use this:
	void detachThread(); // while (true) { waitForPacket(); processPendingPackets(); }
	void signalWaitingForPacket(); //interrupt waitForPacket()
	void waitForWorkerThread(); //join with the worker thead - pretty much useless externally
	
	virtual bool acceptsSignature(uint8_t signature); //defaults to signature==sig
	virtual uint8_t * processDescription(uint8_t * data, size_t len, size_t & descr_len); //return a new-allocated array of tags or NULL if there is no description and set descr_len to the length of the description. defaults to NULL
	virtual size_t lengthForTag(int tag); //return 0 if the length is specified in the first 1 (or 2) bytes
	virtual void recievedPacket(uint8_t signature, const URField * const * fields, int nrfields); //this is called whenever a packet arrives
	
private:
	void processPacket(uint8_t * data, size_t len);
};

#endif
