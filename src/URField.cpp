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

#include "URField.h"
#include "URCommonPrivate.h"


#ifdef __BIG_ENDIAN__
	#define htonll(x) (x)
#else
	#ifdef __GNUC__
		#define htonll __builtin_bswap64
	#elif _MSC_VER
		#define htonll _byteswap_uint64
	#else
		inline uint64_t htonll(uint64_t x) //this is really crappy
		{
			return htonl(x>>32) | ((uint64_t)htonl(x&0xFFFFFFFF)<<32);
		}
	#endif
#endif

#define ntohll htonll

URField::URField(): dt(NULL), len(0), manageMem(false), _tag(0x00), knwnLength(false) {}

URField::~URField()
{
	if (dt && manageMem)
		delete[] dt;
}

void URField::setData(const uint8_t * data, size_t length)
{
	if (dt && manageMem)
		delete[] dt;
	dt = data;
	len = length;
}

void URField::setInt8(uint8_t nr)
{
	uint8_t * d = new uint8_t[1];
	*d = nr;
	setData((uint8_t*)d,sizeof(uint8_t));
}

void URField::setInt16(uint16_t nr)
{
	uint16_t * d = new uint16_t[1];
	*d = htons(nr);
	setData((uint8_t*)d,sizeof(uint16_t));
}

void URField::setInt32(uint32_t nr)
{
	uint32_t * d = new uint32_t[1];
	*d = htonl(nr);
	setData((uint8_t*)d,sizeof(uint32_t));
}

void URField::setInt64(uint64_t nr)
{
	uint64_t * d = new uint64_t[1];
	*d = htonll(nr);
	setData((uint8_t*)d,sizeof(uint64_t));
}

void URField::setInt8s(const uint8_t * ints, int count)
{
	uint8_t * d = new uint8_t[count];
	memcpy(d,ints,count);
	setData((uint8_t*)d,sizeof(uint8_t)*count);
}

void URField::setInt16s(const uint16_t * ints, int count)
{
	uint16_t * d = new uint16_t[count];
	for (int i=0; i<count; i++)
		d[i] = htons(ints[i]);
	setData((uint8_t*)d,sizeof(uint16_t)*count);
}
void URField::setInt32s(const uint32_t * ints, int count)
{
	uint32_t * d = new uint32_t[count];
	for (int i=0; i<count; i++)
		d[i] = htonl(ints[i]);
	setData((uint8_t*)d,sizeof(uint32_t)*count);
}
void URField::setInt64s(const uint64_t * ints, int count)
{
	uint64_t * d = new uint64_t[count];
	for (int i=0; i<count; i++)
		d[i] = htonll(ints[i]);
	setData((uint8_t*)d,sizeof(uint64_t)*count);
}

void URField::setString(std::string str)
{
	size_t n = str.length();
	uint8_t * s = new uint8_t[n];
	memcpy(s,str.c_str(),n);
	setData(s,sizeof(char)*n);
}

uint16_t URField::asInt16() const 
{
	if (len<2) return 0;
	return ntohs(((uint16_t*)dt)[0]);
}

uint32_t URField::asInt32() const 
{
	if (len<4) return 0;
	return ntohl(((uint32_t*)dt)[0]);
}

uint64_t URField::asInt64() const 
{
	if (len<8) return 0;
	return ntohll(((uint64_t*)dt)[0]);
}

uint8_t * URField::asInt8s() const
{
	uint8_t * buf = new uint8_t[len];
	memcpy(buf,dt,len);
	return buf;
}

uint16_t * URField::asInt16s() const
{
	int n = len/sizeof(uint16_t);
	uint16_t * buf = new uint16_t[n];
	for (int i=0; i<n; i++)
		buf[i] = htons(((uint16_t*)dt)[i]);
	return buf;
}

uint32_t * URField::asInt32s() const
{
	int n = len/sizeof(uint32_t);
	uint32_t * buf = new uint32_t[n];
	for (int i=0; i<n; i++)
		buf[i] = htonl(((uint32_t*)dt)[i]);
	return buf;
}

uint64_t * URField::asInt64s() const
{
	int n = len/sizeof(uint64_t);
	uint64_t * buf = new uint64_t[n];
	for (int i=0; i<n; i++)
		buf[i] = ntohll(((uint64_t*)dt)[i]);
	return buf;
}

std::string URField::asString() const
{
	return std::string((char*)dt,len);
}