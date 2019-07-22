/*
	Bias Test Program - Analysis
		(For Data Generation Program, see siphash21_biastest.cpp)
	For any question, please email to he-l17@mails.tsinghua.edu.cn.
	
	This program will load the result of Data Generation Program, \
	  find out the output bits with the greatest imbalances and give relevant information.
	Information includes (of a single output bit):
		Times of being the greatest among 4096 keys
		The average bias (exponential part)
		The biggest and smallest biases among 4096 keys
	
	Program must be executed with 1 operational parameter k(operational parameter) \
	  with file "sip21test_k.txt" existing under directory "./data".
	i.e.:
		./siphash21_analysis 07
		./siphash21_analysis 43
	
	Output Format
		Program only produces standard outputs.
		The first line appears after loading successfully.
		Only those bits that have been the greatest will appear in the outputs, \
		  with 1, 2 or more lines according to k. 
	i.e.:
		load ./data/sip21test_07.txt finished
		j:17 times:1071 average:-4.42 max:-3.37 min:-5.89
		j:33 times:3025 average:-4.26 max:-3.28 min:-5.68
*/

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
using namespace std;

const int keynum = 4096;

double data[64][keynum];

void initdata()
{
	for (int i=0;i<64;i++)
		for (int j=0;j<keynum;j++)
			data[i][j] = 0.0;
}

void loaddata(char* name)
{
	initdata();
	ifstream fin;
	fin.open(name);
	for (int i=0;i<keynum;i++)
	{
		string key1,key2;
		int site,flag,group;
		fin>>key1;
		fin>>key2;
		for (int j=0;j<64;j++)
		{
			fin>>site;
			fin>>data[j][i];
		}
	}
	fin.close();
	cout<<"load "<<name<<" finished\n";
}

int main(int argc, char* argv[])
{
	//filename
	char filename[40] = "./data/sip21test_";
	strcat(filename,argv[1]);
	strcat(filename,".txt");
	loaddata(filename);
	//analyze
	int maxtime[64];
	for (int j=0;j<64;j++) maxtime[j] = 0;
	for (int i=0;i<keynum;i++)
	{
		double maxbias_key = -100.0;
		int maxsite_key = -1;
		for (int j=0;j<64;j++)
			if (data[j][i]>maxbias_key)
			{
				maxbias_key = data[j][i];
				maxsite_key = j;
			}
		maxtime[maxsite_key]++;
	}
	for (int j=0;j<64;j++)
	{
		if (maxtime[j]==0) continue;
		double sum = 0.0;
		double maxbias_j = -100.0;
		double minbias_j = 0.0;
		for (int i=0;i<keynum;i++)
		{
			sum = sum+data[j][i];
			if (data[j][i]>maxbias_j) maxbias_j = data[j][i];
			if (data[j][i]<minbias_j) minbias_j = data[j][i];
		}
		printf("j:%d times:%d average:%.2f max:%.2f min:%.2f\n",j,maxtime[j],sum/keynum,maxbias_j,minbias_j);
	}
	return 0;
}
