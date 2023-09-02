/*
 * records.h
 *
 *  Created on: 29 черв. 2023 р.
 *      Author: 2andn
 */

#ifndef INC_RECORDS_H_
#define INC_RECORDS_H_
#include <cstdint>

#define ELEMENTS_SIZE (6 * 24)

class record {
private:
	float *elements;
	uint16_t indexWrite;
	uint16_t indexRead;
	uint16_t writeSize;
	uint8_t elementsLen;
public:
	float dataAnalyz(uint8_t len);
	void put(float element);
	float get();
	uint16_t getSizeWrites();
	record(float *buffer, uint8_t len);
};

#endif /* INC_RECORDS_H_ */
