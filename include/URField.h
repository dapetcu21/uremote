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

#ifndef URFIELD_H
#define URFIELD_H

#include "URCommon.h"

class URField 
{
private:
	const uint8_t * dt;
	size_t len;
	bool manageMem;
	uint8_t _tag;
	bool knwnLength;
public:
	URField();
	~URField();
	
	bool knownLength() const { return knwnLength; };
	void setKnownLength(bool val) { knwnLength = val; };
	void setTag(uint8_t tg) { _tag = tg; }
	uint8_t tag() const { return _tag; }
	bool managesItsMemory() const { return manageMem; } //if URField manages your memory, any array of data you set with setData will be freed1 with delete[] when not needed anymore
	void setManagesItsMemory(bool val) { manageMem = val; }
	size_t length() const { return len; }
	const uint8_t * data() const { return dt; }
	void setData(const uint8_t * data, size_t length);
	
	
	void setInt8(uint8_t nr);
	void setInt16(uint16_t nr); 
	void setInt32(uint32_t nr);
	void setInt64(uint64_t nr);
	
	//all this stuff first converts to network byte order
	void setInt8s(const uint8_t * ints, int count); //as opposed to setData, this copies the ints and manages their memory
	void setInt16s(const uint16_t * ints, int count);
	void setInt32s(const uint32_t * ints, int count);
	void setInt64s(const uint64_t * ints, int count);
	
	void setString(std::string str);
	
	uint8_t asInt8() const { if (len<1) return 0; return dt[0]; };
	uint16_t asInt16() const ;
	uint32_t asInt32() const ;
	uint64_t asInt64() const ;
	
	//these methods allocate memory with new and copy dt into it after evaluating endianness, so you should free it with delete[]
	uint8_t * asInt8s() const;
	uint16_t * asInt16s() const;
	uint32_t * asInt32s() const;
	uint64_t * asInt64s() const;
	
	std::string asString() const;
};

#endif