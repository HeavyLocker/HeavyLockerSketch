#ifndef _elasticsketch_H
#define _elasticsketch_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BaseSketch.h"

#include "params.h"
#include "ssummary.h"
#include "BOBHASH64.h"
#define BN 4    //the depth of heavy part
#define lambd 8 //the param of vote method
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class Elasticsketch : public sketch::BaseSketch
{
private:
	struct heavy { string FP;unsigned int pvote,Flag; } HK[MAX_MEM+10][BN];
	struct light { unsigned int C; } LK[MAX_MEM+10];
	BOBHash64 * bobhash;
	BOBHash64 * bobhash_;
	BOBHash64 * bobhash_test[4];
	int M1, M2;

public:
	Elasticsketch(int M1, int M2) :M1(M1), M2(M2){ 
		bobhash = new BOBHash64(995);
		bobhash_ = new BOBHash64(1010);
		for (int i=0; i<4; i++)
			bobhash_test[i] = new BOBHash64(i+1000); 
    }

	void clear()
	{
		for (int i = 0; i < M1 + 5; i++)
			for (int j = 0; j < BN; j++)
				HK[i][j].FP = HK[i][j].pvote = HK[i][j].Flag = 0;

		for (int j = 0; j < M2 + 5; j++)
			LK[j].C = 0;
			
	}

	void Insert(const string str)
	{
		unsigned int H1 = bobhash->run(str.c_str(), KEY_LEN) % M1;
		unsigned int temphash;
		temphash = bobhash_->run(str.c_str(), KEY_LEN) % M2;
		unsigned int min_size = MAX_INSERT;
		int min_pos = -1;
		int flag = 0;
		for (int i = 0; i < BN-1; i++) {
			if (HK[H1][i].FP == str) {
				HK[H1][i].pvote++;
				flag = 1;
				break;
			}
			else if (HK[H1][i].pvote == 0) {
				HK[H1][i].pvote = 1;
				HK[H1][i].FP = str;
				flag = 1;
				break;
			}
			if (min_size > HK[H1][i].pvote) {
				min_pos = i;
				min_size = HK[H1][i].pvote;
			}
		}
		if (!flag) {
			HK[H1][BN-1].pvote++;
			if (HK[H1][min_pos].pvote*lambd<=HK[H1][BN-1].pvote) {
				if(HK[H1][min_pos].Flag==0){
					if(min_size>255)
						min_size=255;
					LK[temphash].C=min_size;
				}else{
					LK[temphash].C+=min_size;
					if(LK[temphash].C>255)
						LK[temphash].C=255;
				}
				HK[H1][min_pos].FP = str;
				HK[H1][min_pos].Flag = 1;
				HK[H1][min_pos].pvote = 1;
			}else{
				LK[temphash].C++;
			}
		}	
	}
	
	void work(int n)
	{
		for(int i=0;i<M1;i++){
			for(int j=0;j<BN;j++){
				mergename[n][j][i]=HK[i][j].FP;
				mergeresult1[n][j][i]=HK[i][j].pvote;
				mergeresult2[n][j][i]=HK[i][j].Flag;
			}
		}
		for(int i=0;i<M2;i++){
			mergeresult3[n][0][i]=LK[i].C;
		}
	}
	int merge(int thresh,int opt=0){
		cout<<node_num<<endl;
		for(unordered_map<string,int>::iterator it=allflowname.begin();it!=allflowname.end();it++){
			it->second=0;
    	}
		//merge the heavy part
		for(int i=0;i<M1;i++){
			int cnt=0;
			unordered_map<string,int> temp;
			for(int z=0;z<node_num;z++){
				for(int j=0;j<BN-1;j++){
					if(temp.find(mergename[z][j][i]) != temp.end()){
						temp[mergename[z][j][i]]+=mergeresult1[z][j][i];
					}else{
						temp[mergename[z][j][i]]=mergeresult1[z][j][i];
					} 	
					allflowname[mergename[z][j][i]]|=mergeresult2[z][j][i];
				}
			}
			for(unordered_map<string,int>::iterator it=temp.begin();it!=temp.end();it++){
				q[cnt].x=it->first;
				q[cnt].y=it->second;
				cnt++;
			}
			sort(q,q+cnt,cmp);
			for(int j=0;j<BN;j++){
				HK[i][j].FP=q[j].x;
				HK[i][j].pvote=q[j].y;
				HK[i][j].Flag=allflowname[q[j].x];
			}
		}
		//merge the light part
		if(opt==false){
			for(int i=0;i<M2;i++){
				int maxv=-1;
				for(int z=0;z<node_num;z++){
					maxv=max(maxv,mergeresult3[z][0][i]);
				}
				LK[i].C=maxv;
			}
		}else{
			for(int i=0;i<M2;i++){
				int maxv=0;
				for(int z=0;z<node_num;z++){
					maxv+=mergeresult3[z][0][i];
				}
				LK[i].C=maxv;
			}		
		}

		for(unordered_map<string,int>::iterator it=allflowname.begin();it!=allflowname.end();it++){
			string str=it->first;
			unsigned int H1 = bobhash->run(str.c_str(), KEY_LEN) % M1;
			unsigned int temphash = bobhash_->run(str.c_str(), KEY_LEN) % M2;
			unsigned int maxv;
			int flag=0;
			maxv=0;
			for (int i = 0; i < BN; i++) {
				if (HK[H1][i].FP == str) {
					maxv=HK[H1][i].pvote;
					if (HK[H1][i].Flag == 1) {
						flag=1;
						maxv += LK[temphash].C;
					}
					break;
				}
			}
			
			if (maxv == 0) {
				maxv = LK[temphash].C;
			}
			it->second=maxv;
		}

		int CNT=0;
		for(unordered_map<string,int>::iterator it=allflowname.begin();it!=allflowname.end();it++){
			q[CNT].x = it->first; q[CNT].y = it->second; CNT++;
    	} 
		sort(q, q + CNT, cmp);
		int bigflow;
		for(bigflow=0;q[bigflow].y>thresh&&bigflow<CNT;bigflow++);
		return bigflow;
	}
	
	std::pair<std::string, int> Query_top(int k)
	{
		return make_pair(q[k].x, q[k].y);
	}

	int Query(string str)
	{
		if(allflowname.find(str) != allflowname.end()){
			return allflowname[str];
		}else{
			return 0;
		}
	}

	std::string get_name() {
		return "Elasticsketch";
	}
};
#endif
