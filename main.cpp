#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <vector>

#include "CMSketch.h"
//#include "CUSketch.h"
#include "ASketch.h"
//#include "PCUSketch.h"
#include "ElasticSketch.h"
//#include "NitroSketch.h"
#include "MVSketch.h"
#include "./UnivMon/Univmon.h"
#include "MSketch.h"

using namespace std;


char * filename_stream = "../../data/";


char insert[40000000 + 1000000 / 5][105];
char query[40000000 + 1000000 / 5][105];


unordered_map<string, int> unmp;

#define testcycles 1
//#define hh 0.00005
#define hc 0.0005

double hh = 0.0001;

unordered_map<string, int> metrics;

int main(int argc, char** argv)
{
    double memory = 0.1;
    if(argc >= 2){
        filename_stream = argv[1];
    }
    if (argc >= 3){
    	memory = stod(argv[2]);
    }
    if (argc >= 4){
        hh = stod(argv[3]);
    }
    if (argc >= 5){
        depth = stoi(argv[4]);
    }
    if (argc >= 6){
        lock_thre = stod(argv[5]);
    }
    if (argc >= 7){
        hash_num = stoi(argv[6]);
    }
    for (int i=7; i<argc; i++){
        metrics[string(argv[i])]=1;
    }
    

    unmp.clear();
    int val;



    //const double memory = 0.4;// MB
    int memory_ = memory * 1000;//KB
    int word_size = 64;


    int w = memory_ * 1024 * 8.0 / COUNTER_SIZE;	//how many counter;
    int w_p = memory * 1024 * 1024 * 8.0 / (word_size * 2);
    int m1 = memory * 1024 * 1024 * 1.0/4 / 8 / 8;
    int m2 = memory * 1024 * 1024 * 3.0/4 / 2 / 1;
    int m2_mv = memory * 1024 * 1024 / 8 / 4;

    //printf("\n******************************************************************************\n");
    //printf("Evaluation starts!\n\n");

    CMSketch *cmsketch;
    //CUSketch *cusketch;
    ASketch *asketch;
    //PCUSketch *pcusketch;
    //Nitrosketch *nitrosketch;
    Elasticsketch *elasticsketch;
    MVsketch *mvsketch;
    UnivMon *univmon;
    MSketch *msketch;

    char _temp[200], temp2[200];
    int t = 0;

    int package_num = 0;


    FILE *file_stream = fopen(filename_stream, "r");

    //while(fgets(insert[package_num], 105, file_stream) != NULL)
    while (fread(insert[package_num], 1, KEY_LEN, file_stream)==KEY_LEN)
    {
        unmp[string(insert[package_num], KEY_LEN)]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_stream);

    //printf("memory = %dKB\n", memory_);
    //printf("dataset name: %s\n", filename_stream);
    //printf("total stream size = %d\n", package_num);
    //printf("distinct item number = %d\n", unmp.size());
 
    int max_freq = 0;
    unordered_map<string, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        //strcpy(query[i], it->first.c_str());
        memcpy(query[i], (it->first).c_str(), KEY_LEN);

        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    //printf("max_freq = %d\n", max_freq);
    
    //printf("*************************************\n");



/********************************insert*********************************/

    timespec time1, time2;
    long long resns;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; i++)
        {
            cmsketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cm = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of CM (insert): %.6lf Mips\n", throughput_cm);
   


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        asketch = new ASketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; i++)
        {
            asketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of A (insert): %.6lf Mips\n", throughput_a);


	

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < testcycles; t++)
	{
		elasticsketch = new Elasticsketch(m1, m2);
		for (int i = 0; i < package_num; i++)
		{
			elasticsketch->Insert(insert[i]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_elastic = (double)1000.0 * testcycles * package_num / resns;
	//printf("throughput of Elastic (insert): %.6lf Mips\n", throughput_elastic);

    
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        mvsketch = new MVsketch(m2_mv);
        for(int i = 0; i < package_num; i++)
        {
            mvsketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_mvsketch = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of MVsketch (insert): %.6lf Mips\n", throughput_mvsketch);	


	clock_gettime(CLOCK_MONOTONIC, &time1);
    for (int t = 0; t < testcycles; t++)
    {
            univmon = new UnivMon(memory * 1024 *1024);
	    for (int i = 0; i < package_num; i++)
        {
            univmon->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_univmon = (double)1000.0 * testcycles * package_num / resns;



	clock_gettime(CLOCK_MONOTONIC, &time1);
    for (int t = 0; t < testcycles; t++)
    {
            msketch = new MSketch(memory * 1024 *1024/(6 * depth), hh);
	    for (int i = 0; i < package_num; i++)
        {
            msketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_msketch = (double)1000.0 * testcycles * package_num / resns;







/********************************************************************************************/


    //avoid the over-optimize of the compiler! 
    double sum = 0;

    if(sum == (1 << 30))
        return 0;

    char temp[105];

    double re_cm = 0.0, re_cu = 0.0,  re_a = 0.0,  re_univmon = 0.0, re_pcsketch = 0.0, re_cccounter = 0.0, re_elastic=0.0, re_nitro=0.0, re_mvsketch=0.0, re_msketch=0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0,  re_a_sum = 0.0,  re_univmon_sum = 0.0, re_cccounter_sum = 0.0, re_elastic_sum=0.0, re_nitro_sum=0.0, re_mvsketch_sum=0.0, re_msketch_sum=0.0;
    
    double ae_cm = 0.0, ae_cu = 0.0,  ae_a = 0.0,  ae_univmon = 0.0, ae_cccounter = 0.0, ae_elastic=0.0, ae_nitro=0.0, ae_mvsketch=0.0, ae_msketch=0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0,  ae_a_sum = 0.0,  ae_univmon_sum = 0.0, ae_cccounter_sum = 0.0, ae_elastic_sum=0.0, ae_nitro_sum=0.0, ae_mvsketch_sum=0.0, ae_msketch_sum=0.0;

    double val_cm = 0.0, val_cu = 0.0,  val_a = 0.0,  val_univmon = 0.0, val_cccounter = 0.0, val_elastic=0.0, val_nitro=0.0, val_mvsketch=0.0, val_msketch=0.0;
    double mem_cc = 0.0, mem_cc_sum = 0.0;

    double rc_cm = 0.0, rc_cu = 0.0,  rc_a = 0.0,  rc_univmon = 0.0, rc_cccounter = 0.0, rc_elastic=0.0, rc_nitro=0.0, rc_mvsketch=0.0, rc_msketch=0.0;
    double pr_cm = 0.0, pr_cu = 0.0,  pr_a = 0.0,  pr_univmon = 0.0, pr_cccounter = 0.0, pr_elastic=0.0, pr_nitro=0.0, pr_mvsketch=0.0, pr_msketch=0.0;
    double f1_cm = 0.0, f1_cu = 0.0,  f1_a = 0.0,  f1_univmon = 0.0, f1_cccounter = 0.0, f1_elastic=0.0, f1_nitro=0.0, fl_mvsketch=0.0, f1_msketch=0.0;
    double tp_cm = 0.0, tp_cu = 0.0,  tp_a = 0.0,  tp_univmon = 0.0, tp_cccounter = 0.0, tp_elastic=0.0, tp_nitro=0.0, tp_mvsketch=0.0, tp_msketch=0.0;
    double fp_cm = 0.0, fp_cu = 0.0,  fp_a = 0.0,  fp_univmon = 0.0, fp_cccounter = 0.0, fp_elastic=0.0, fp_nitro=0.0, fp_mvsketch=0.0, fp_msketch=0.0;
    double tn_cm = 0.0, tn_cu = 0.0,  tn_a = 0.0,  tn_univmon = 0.0, tn_cccounter = 0.0, tn_elastic=0.0, tn_nitro=0.0, tn_mvsketch=0.0, tn_msketch=0.0;
    double fn_cm = 0.0, fn_cu = 0.0,  fn_a = 0.0,  fn_univmon = 0.0, fn_cccounter = 0.0, fn_elastic=0.0, fn_nitro=0.0, fn_mvsketch=0.0, fn_msketch=0.0;

    int threshold = package_num * hh;
    int hh_num = 0;

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        //strcpy(temp, (it->first).c_str());
        memcpy(temp, (it->first).c_str(), KEY_LEN);
        val = it->second;
        
	bool f1_true = 0;
	bool f2_cm = 0, f2_cu = 0,  f2_a = 0,  f2_univmon = 0, f2_cccounter = 0, f2_elastic=0, f2_nitro=0, f2_mvsketch=0, f2_msketch=0;
 
	if (val >= threshold) {
		f1_true = 1;
		hh_num++;
	}

        val_cm = cmsketch->Query(temp);  
        val_a = asketch->Query(temp);
	    val_elastic = elasticsketch->Query(temp);
	    val_mvsketch = mvsketch->Query(temp);
        val_univmon = univmon->Query(temp);
	    val_msketch = msketch->Query(temp);
	
	if (val_cm >= threshold) f2_cm = 1;
	if (val_a >= threshold) f2_a = 1;
	if (val_elastic >= threshold) f2_elastic = 1;
	if (val_mvsketch >= threshold) f2_mvsketch = 1;
    if (val_univmon >= threshold) f2_univmon = 1;
	if (val_msketch >= threshold) f2_msketch = 1;

	if (f1_true) {
        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_a = fabs(val_a - val) / (val * 1.0);
	    re_elastic = fabs(val_elastic - val) / (val * 1.0);
	    re_mvsketch = fabs(val_mvsketch - val) / (val * 1.0);
        re_univmon = fabs(val_univmon - val) / (val * 1.0);
	    re_msketch = fabs(val_msketch - val) / (val * 1.0);

        ae_cm = fabs(val_cm - val);      
        ae_a = fabs(val_a - val);          
	    ae_elastic = fabs(val_elastic - val);
	    ae_mvsketch = fabs(val_mvsketch - val);
        ae_univmon = fabs(val_univmon - val); 
	    ae_msketch = fabs(val_msketch - val);


        re_cm_sum += re_cm;       
        re_a_sum += re_a;               
	    re_elastic_sum += re_elastic;
	    re_mvsketch_sum += re_mvsketch;
        re_univmon_sum += re_univmon; 
	    re_msketch_sum += re_msketch;

        ae_cm_sum += ae_cm;    
        ae_a_sum += ae_a;  
	    ae_elastic_sum += ae_elastic;
	    ae_mvsketch_sum += ae_mvsketch;
        ae_univmon_sum += ae_univmon; 
	    ae_msketch_sum += ae_msketch;
	}

	if (f1_true && f2_cm) tp_cm++;
	else if (f1_true && !f2_cm) fn_cm++;
	else if (!f1_true && f2_cm) fp_cm++;
	else tn_cm++;

	if (f1_true && f2_a) tp_a++;
	else if (f1_true && !f2_a) fn_a++;
	else if (!f1_true && f2_a) fp_a++;
	else tn_a++;

	if (f1_true && f2_elastic) tp_elastic++;
	else if (f1_true && !f2_elastic) fn_elastic++;
	else if (!f1_true && f2_elastic) fp_elastic++;
	else tn_elastic++;

	if (f1_true && f2_mvsketch) tp_mvsketch++;
	else if (f1_true && !f2_mvsketch) fn_mvsketch++;
	else if (!f1_true && f2_mvsketch) fp_mvsketch++;
	else tn_mvsketch++;

    if (f1_true && f2_univmon) tp_univmon++;
	else if (f1_true && !f2_univmon) fn_univmon++;
	else if (!f1_true && f2_univmon) fp_univmon++;
	else tn_univmon++;

	if (f1_true && f2_msketch) tp_msketch++;
	else if (f1_true && !f2_msketch) fn_msketch++;
	else if (!f1_true && f2_msketch) fp_msketch++;
	else tn_msketch++;
   }

    	double b = hh_num * 1.0;
	    //printf("Heavy Hitter threshold = %d\n",threshold);
	    //printf("Heavy Hitter numbers = %d\n", hh_num);

    if (metrics["T"]){
        printf("CM\n%lf\n", throughput_cm);
	    printf("AS\n%lf\n", throughput_a);
	    printf("ES\n%lf\n", throughput_elastic);
	    printf("MV\n%lf\n", throughput_mvsketch);
        printf("UnivMon\n%lf\n", throughput_univmon);
        printf("HL\n%lf\n", throughput_msketch);
	    //printf("HL-%d\n%lf\n", memory_, throughput_msketch);
    }

    	//printf("\n*************** Heavy hitter detection: ****************\n\n");
    if (metrics["AAE"]){
 	    //printf("*************** AAE ****************\n");
    	printf("CM\n%lf\n", ae_cm_sum / b);
	    printf("AS\n%lf\n", ae_a_sum / b);
	    printf("ES\n%lf\n", ae_elastic_sum / b);
	    printf("MV\n%lf\n", ae_mvsketch_sum / b);
        printf("UnivMon\n%lf\n", ae_univmon_sum / b); 
	    printf("HL\n%lf\n", ae_msketch_sum / b);
    }
    if (metrics["ARE"]){
    	//printf("******************* ARE ******************\n");

    	printf("CM\n%lf\n", re_cm_sum / b);
	    printf("AS\n%lf\n", re_a_sum / b);
	    printf("ES\n%lf\n", re_elastic_sum / b); 
	    printf("MV\n%lf\n", re_mvsketch_sum / b);
        printf("UnivMon\n%lf\n", re_univmon_sum / b);
        printf("HL\n%lf\n", re_msketch_sum / b);
	    //printf("HL-%d\n%lf\n", memory_, re_msketch_sum / b);
    }
    if (metrics["Recall"]){
	    //printf("****************** Recall *******************\n");

    	printf("CM\n%lf\n", tp_cm / (tp_cm + fn_cm));
	    printf("AS\n%lf\n", tp_a / (tp_a + fn_a));
        printf("ES\n%lf\n", tp_elastic / (tp_elastic + fn_elastic)); 
	    printf("MV\n%lf\n", tp_mvsketch / (tp_mvsketch + fn_mvsketch));
        printf("UnivMon\n%lf\n", tp_univmon / (tp_univmon + fn_univmon));
	    printf("HL\n%lf\n", tp_msketch / (tp_msketch + fn_msketch));
    }
    if (metrics["Precision"]){
	    //printf("******************* Precision ******************\n");

	    printf("CM\n%lf\n", tp_cm / (tp_cm + fp_cm));
	    printf("AS\n%lf\n", tp_a / (tp_a + fp_a));
	    printf("ES\n%lf\n", tp_elastic / (tp_elastic + fp_elastic));    	
	    printf("MV\n%lf\n", tp_mvsketch / (tp_mvsketch + fp_mvsketch));
        printf("UnivMon\n%lf\n", tp_univmon / (tp_univmon + fp_univmon));
	    printf("HL\n%lf\n", tp_msketch / (tp_msketch + fp_msketch)); 
    }
    if (metrics["F1"]){
	    //printf("****************** F1 score *******************\n");

	    printf("CM\n%lf\n", 2 * tp_cm / (2 * tp_cm + fp_cm + fn_cm));
	    printf("AS\n%lf\n", 2 * tp_a / (2 * tp_a + fp_a + fn_a));
	    printf("ES\n%lf\n", 2 * tp_elastic / (2 * tp_elastic + fp_elastic + fn_elastic)); 
	    printf("MV\n%lf\n", 2 * tp_mvsketch / (2 * tp_mvsketch + fp_mvsketch + fn_mvsketch));
        printf("UnivMon\n%lf\n", 2 * tp_univmon / (2 * tp_univmon + fp_univmon + fn_univmon));
        printf("HL\n%lf\n", 2 * tp_msketch / (2 * tp_msketch + fp_msketch + fn_msketch));
	    //printf("HL-%d\n%lf\n", memory_, 2 * tp_msketch / (2 * tp_msketch + fp_msketch + fn_msketch));
    }
    //	printf("******************************************************************************\n");
    //	printf("Evaluation Ends!\n\n");

	    return 0;
}
