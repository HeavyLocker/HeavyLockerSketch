#ifndef _BBSKETCH_H
#define _BBSKETCH_H

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <stdlib.h>
//#include "BOBHash.h"
#include "BOBHASH64.h"
#include "params.h"


uint32_t depth=5;
double lock_thre=0.5;
uint32_t hash_num=1;

using namespace std;
class MSketch
{
private:
	struct bucket_t {	//one bucket
		uint32_t fingerprint[6];	//four fingerprints  
		uint32_t counter[6];	
		bool lock_bit=0;
	};
	uint64_t bucket_num, h1, h2;	//bucket_num indicates the number of buckets in each array
	bucket_t *bucket[1];		//two arrays
	BOBHash64 * bobhash[5];		//Bob hash function
	double hh_ratio;
	uint32_t total_packet=0;
	uint64_t hash[5]={0};

public:
	MSketch(uint _bucket, double _hh_ratio) {
		hh_ratio = _hh_ratio * lock_thre;
		bucket_num = _bucket;
		for (int i = 0; i < hash_num; i++) {
			bobhash[i] = new BOBHash64(i + 1005);
		}
		for (int i = 0; i < 1; i++) {	//initialize two arrays 
			bucket[i] = new bucket_t[bucket_num];
			memset(bucket[i], 0, sizeof(bucket_t) * bucket_num);
		}
	}
	
	bool plus(bucket_t* b, int j) {		//try to plus entry_j in the bucket
		b->counter[j-1]++;	
		if (j==depth) return true;
		if (b->counter[j-1] > b->counter[j]){
			swap(b->counter[j-1], b->counter[j]);
			swap(b->fingerprint[j-1], b->fingerprint[j]);
		}
		return true;
	}
	
	void Insert(const char *key) {
		total_packet++;
		//maxloop = 1;		
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		for (int i = 0; i < hash_num; i++){
			hash[i] = (bobhash[i]->run(key, KEY_LEN));
		}
	//	h1 = (bobhash[0]->run(key, KEY_LEN)); //% bucket_num;
	//	h2 = (bobhash[1]->run(key, KEY_LEN));
	//	h2 = ((h1>>32)&0xffff);
	//	char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		uint32_t fp = (hash[0]>>(64-FP_LEN));

		for (int i = 0; i < hash_num; i++){
			hash[i]%=bucket_num;
		}
		//h1 = (h1&0xffffffff);
		//h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))); //% bucket_num;		
		//uint hash[2] = {h1%bucket_num, h2%bucket_num};
	//	uint hash[1] = {h1%bucket_num};
		//uint hashh[2] = {h1, h2};
		
		int ii, jj, flag = 0;
		for (int i = 0; i < hash_num; i++) {
			/*if (bucket[i][hash[0]].lock_bit == 1){
				if (bucket[i][hash[i]].counter[0] < (total_packet*hh_ratio)){
					bucket[i][hash[i]].lock_bit=0;
				}
				return;
			}*/
			for(int j = depth; j >= 1; j--) {
				if (bucket[0][hash[i]].fingerprint[j-1] == fp && bucket[0][hash[i]].counter[j-1]!=0) {
					plus(&bucket[0][hash[i]], j);
					/*if (bucket[0][hash[0]].counter[0] >= (total_packet*hh_ratio) && total_packet>=1){
						bucket[0][hash[0]].lock_bit=1;
					}*/
					return;
				}
				else if (bucket[0][hash[i]].fingerprint[j-1] == 0 && bucket[0][hash[i]].fingerprint[j-1] == 0){
					bucket[0][hash[i]].fingerprint[j-1] = fp;
					plus(&bucket[0][hash[i]], j);
					return;
				}
			}
		}
		/*if (bucket[0][hash[0]].lock_bit == 1 && total_packet>=1){
			if (bucket[0][hash[0]].counter[0] < (total_packet*hh_ratio)){
				bucket[0][hash[0]].lock_bit=0;
			}
			//else{
				return;
			//}
		}*/
		int i = 0;
		for (i; i < hash_num; i++){
			if (bucket[0][hash[i]].counter[0]<(total_packet*hh_ratio)){
				bucket[0][hash[i]].counter[0]--;
				if (bucket[0][hash[i]].counter[0]<=0){
					bucket[0][hash[i]].fingerprint[0] = fp;
					bucket[0][hash[i]].counter[0] = 1;
				}
				return;
			}
		}
		return;
		/*if (bucket[0][hash[0]].counter[0]>=(total_packet*hh_ratio)){
			if (bucket[0][hash[1]].counter[0]>=(total_packet*hh_ratio)){
				return;
			}
			i=1;
			//return;
		}*/
		/*else{
			i=0;
		}*/
		
	}

	
	double Query(const char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24))
		for (int i = 0; i < hash_num; i++){
			hash[i] = (bobhash[i]->run(key, KEY_LEN));
		}
	//	h1 = (bobhash[0]->run(key, KEY_LEN)); //% bucket_num;
	//	h2 = (bobhash[1]->run(key, KEY_LEN));
	//	h2 = ((h1>>32)&0xffff);
	//	char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		uint32_t fp = (hash[0]>>(64-FP_LEN));
		for (int i = 0; i < hash_num; i++){
			hash[i]%=bucket_num;
		}
	//	h1 = (h1&0xffffffff);
//		uint hash[1] = {h1%bucket_num};
	//	uint hash[2] = {h1%bucket_num, h2%bucket_num};
		bool flag=0;
		for (int i = 0; i < hash_num; i++) {
			for (int j = depth; j >= 1; j--) {
				if (bucket[0][hash[i]].fingerprint[j-1] == fp)
					return bucket[0][hash[i]].counter[j-1];
				if (bucket[0][hash[i]].fingerprint[j-1] == 0 && bucket[0][hash[i]].counter[j-1] == 0)
					return 0;
			}
		}
		return 0;
	}
	
	//memeory access
/*	int Mem(const char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, KEY_LEN));// % bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1)));// % bucket_num;
		int hash[2] = {h1%bucket_num, h2%bucket_num};
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 4; j++) {
				if (bucket[i][hash[i]].fingerprint[j] == fp)
					return 1;
			}
		}
		return 2;
	}*/

	// the use ratio of the bucket
/*	double Ratio() {
		int used_num = 0;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < bucket_num; j++) {
				if ((bucket[i][j].count12&0xf) != 0) used_num++;
				if ((bucket[i][j].count12&0xf0 != 0)) used_num++;
				if (bucket[i][j].count3 != 0) used_num++;
				if (bucket[i][j].count4 != 0) used_num++;
			}
		}
		return used_num / (bucket_num * 2.0);
	}*/

/*	void minus(bucket_t* b, int j) {
		if (j == 1) {
			b->count12--;
			if (b->count12 & 0xf == 0)
				b->fingerprint[0] = NULL;
		}
		else if (j == 2) {
			b->count12 -= 0x10;
			if (b->count12 & 0xf0 == 0)
				b->fingerprint[1] = NULL;
		}
		else if (j == 3) {
			b->count3--;
			if (b->count3 == 0)
				b->fingerprint[2] = NULL;
		}
		else if (j == 4) {
			b->count4--;
			if (b->count4 == 0)
				b->fingerprint[3] = NULL;
		}
	}*/

	//delete the bucket
/*	void Delete(char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, KEY_LEN)); //% bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1)));// % bucket_num;
		uint hash[2] = {h1%bucket_num, h2%bucket_num};
		for (int i = 0; i < 2; i++) {
			for (int j = 1; j <= 4; j++) {
				if (bucket[i][hash[i]].fingerprint[j-1] == fp) {
					minus(&bucket[i][hash[i]], j);
					return;					
				}
			}
		}
	}*/

	~MSketch() {
		for (int i = 0; i < 1; i++) {
			delete[]bucket[i];
		}
		for (int i = 0; i < 1; i++) {
			delete bobhash[i];
		}
	}
};
#endif//_CCCOUNTER_H
