/*
 * records.cpp
 *
 *  Created on: 29 черв. 2023 р.
 *      Author: 2andn
 */

#include <cstdint>
#include "records.h"

float record::dataAnalyz(uint8_t len)
{
	float res = 0;
	uint8_t index, prewIndex;
	if (len > elementsLen || len > writeSize) {
		return -9999;
	}
	for (uint8_t x = 1; x < len; x++) {
		if (len <= indexWrite) {
			index = (indexWrite - (len-1)) + x;
		} else {
			index = (indexWrite + elementsLen - (len-1)) + x;
			if(index >= elementsLen) {
				index-=elementsLen;
			}
		}
		if (index == 0) {
			prewIndex = elementsLen - 1;
		} else {
			prewIndex = index - 1;
		}
		res += (elements[index] - elements[prewIndex]);
	}
	return res;
}

void record::put(float val)
{
	if (++indexWrite == elementsLen) {
		indexWrite = 0;
	}
	elements[indexWrite] = val;
	if(writeSize < elementsLen)writeSize++;
}

float record::get()
{
	if(writeSize > 0) {
		writeSize--;
		if(++indexRead == elementsLen) {
			indexRead = 0;
		}
	} else {
		return -9999;
	}
	return elements[indexRead];
}

uint16_t record::getSizeWrites()
{
	return writeSize;
}

record::record(float *buffer, uint8_t len)
{
	elements = buffer;
	indexWrite = 0;
	indexRead = 0;
	writeSize = 0;
	elementsLen = len;
}
