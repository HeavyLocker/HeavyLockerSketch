#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash.h"
#include <iostream>

using namespace std;

class CMSketch
{	
private:
	BOBHash * bobhash[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int *counter[MAX_HASH_NUM];
	int w, d;
	int MAX_CNT;
	int counter_index_size;
	uint64_t hash_value;

public:
	CMSketch(int _w, int _d)
	{

		counter_index_size = 20;
		w = _w;
		d = _d;
		
		
		for(int i = 0; i < d; i++)	
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
		}

		//MAX_CNT = MAX_INSERT_PACKAGE;
		MAX_CNT = UINT32_MAX;

		for(int i = 0; i < d; i++)
		{
			bobhash[i] = new BOBHash(i + 1000);
		}
	}
	void Insert(const char * str)
	{
		//printf("the CM bucket is =%d\n", w);
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, KEY_LEN)) % w;
			if((uint32_t)counter[i][index[i]] < MAX_CNT)
			{
				counter[i][index[i]]++;
			}

		}
	}
	double Query(const char *str)
	{
		uint32_t min_value = UINT32_MAX;
		uint32_t temp;
		
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, KEY_LEN)) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	
	}
	void Delete(const char * str)
	{
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, KEY_LEN)) % w;
			counter[i][index[i]] --;
		}
	}
	~CMSketch()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
		}


		for(int i = 0; i < d; i++)
		{
			delete bobhash[i];
		}
	}
};
#endif//_CMSKETCH_H
