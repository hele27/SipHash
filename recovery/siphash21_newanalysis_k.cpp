/*
	Key Classification Program - Analysis
		(For Data Generation Program, see siphash21_newcondtest_k.cpp)
	For any question, please email to he-l17@mails.tsinghua.edu.cn.
	
	This program will load the result of Data Generation Program, \
	  find out the best boundline to recover v2[k], and calculate the expected success rate.
	Methods to choose the boundline and calculate the success rate are given in article "Cryptanalysis of Reduced-Round SipHash".
	Since the data complexity is only 2^20, output bias 2^-9 is required according to 4-sigma-principle, \
	  which means boundlines less than -9 will be filtered. 
	
	Program must be executed with 2 operational parameters k(operational parameter) and n(operational parameter), \
	  with file "sip21test_k_n.txt" existing under directory "./data".
	i.e.:
		./siphash21_newanalysis_k 07 0
		./siphash21_newanalysis_k 43 1
	
	Output Format
		Program only produces standard outputs.
		The first line appears after loading successfully.
		Next 64 lines show best boundlines and corresponding results using different output bits 0~63.
		Finally, program gives the best choice as well as proportions p1 and p2 or q1 and q2 (same meaning as in article), \
		  followed with corresponding success rate of recovering v2[k].
		*If there are multiple choices leading to success rate 1, program will print all those choices.
		*If one line of output bit j shows "bound:0.000 p:0.000", \
		  that's because the biases under all keys are less than 2^-9, filtering all data instead of causing program mistakes.  
	i.e.:
		load ./data/sip21test_32_0.txt finished
		site:0 bound:-8.925 p:0.500
		site:1 bound:-8.985 p:0.500
		site:2 bound:0.000 p:0.000
		...
		site:10 bound:-6.170 p:1.000
		... 
		site:58 bound:-5.205 p:1.000
		...
		site:63 bound:0.000 p:0.000
		
		best choices:
		site:10 midbound:-6.170 leftbound:-6.330 rightbound:-6.010
		p1:0.000 p2:1.000 P_v2[k]:1.000
		site:58 midbound:-5.205 leftbound:-5.310 rightbound:-5.100
		p1:0.000 p2:1.000 P_v2[k]:1.000
*/

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
using namespace std;

const int keynum = 2*4096;

struct testdata
{
	double bias;
	int flag;
};
testdata data[64][keynum];

void initdata()
{
	for (int i=0;i<64;i++)
	{
		for (int j=0;j<keynum;j++)
		{
			data[i][j].bias = 0.0;
			data[i][j].flag = -1;
		}
	}
}

void sortdata(int k)
{
	for (int i=0;i<keynum-1;i++)
		for (int j=i+1;j<keynum;j++)
			if (data[k][i].bias>data[k][j].bias)
			{
				testdata temp = data[k][i];
				data[k][i] = data[k][j];
				data[k][j] = temp;
			}
}

void loaddata(char* name)
{
	initdata();
	ifstream fin;
	fin.open(name);
	for (int i=0;i<keynum;i++)
	{
		string key1,key2;
		int site,flag;
		double bias;
		fin>>key1;
		fin>>key2;
		flag = i/(keynum/2);
		for (int j=0;j<64;j++)
		{
			fin>>site;
			fin>>bias;
			data[j][i].bias = bias;
			data[j][i].flag = flag;
		}
	}
	fin.close();
	cout<<"load "<<name<<" finished\n";
}

double myabs(double a)
{
	if (a>0) return a;
	else return -a;
}

int chartoint(char* s)
{
	int ans = 0;
	ans = ans+(s[0]-'0');
	if (s[1]!='\0') ans = 10*ans+(s[1]-'0');
	return ans;
}

int main(int argc, char* argv[])
{
	//filename
	char filename[40] = "./data/sip21test_";
	strcat(filename,argv[1]);
	strcat(filename,"_");
	strcat(filename,argv[2]);
	strcat(filename,".txt");
	int n = chartoint(argv[2]);
	loaddata(filename);
	//analyze
	int maxj = -1;
	double maxp = 0;
	double maxleftbound = 0.0;
	double maxrightbound = 0.0;
	int bestjlist[64];
	double bestboundleftlist[64];
	double bestboundrightlist[64];
	for (int i=0;i<64;i++)
	{
		bestjlist[i] = 0;
		bestboundleftlist[i] = 0;
		bestboundrightlist[i] = 0;
	}
	int bestlist_i = -1;
	for (int j=0;j<64;j++)
	{
		sortdata(j);
		double maxp_ofj = 0;
		double maxboundleft_ofj = 0.0;
		double maxboundright_ofj = 0.0;
		int p0 = 0;
		int q0 = 0;
		for (int i=0;i<keynum;i++)
		{
			if (data[j][i].flag==0) p0++;
			if (data[j][i].flag==1) q0++;
			if (data[j][i].bias<-9) continue;
			if ((i<keynum-1)&&(data[j][i+1].bias==data[j][i].bias)) continue;
			double pp0 = p0*1.0/(keynum/2);
			double qq0 = q0*1.0/(keynum/2);
			double p = (1+myabs(pp0-qq0))/2;
			if (p>maxp_ofj)
			{
				maxp_ofj = p;
				maxboundleft_ofj = data[j][i].bias;
				if (i==keynum-1) maxboundright_ofj = data[j][i].bias+0.01;
				else maxboundright_ofj = data[j][i+1].bias;
			}
		}
		printf("site:%d bound:%.3f p:%.3f\n",j,(maxboundleft_ofj+maxboundright_ofj)/2,maxp_ofj);
		if (myabs(1-maxp_ofj)<1e-6)
		{
			bestlist_i++;
			bestjlist[bestlist_i] = j;
			bestboundleftlist[bestlist_i] = maxboundleft_ofj;
			bestboundrightlist[bestlist_i] = maxboundright_ofj;
		}
		if (maxp_ofj>maxp)
		{
			maxj = j;
			maxp = maxp_ofj;
			maxleftbound = maxboundleft_ofj;
			maxrightbound = maxboundright_ofj;
		}
	}
	printf("\n");
	printf("best choices:\n");
	if (bestlist_i!=-1)
	{
		for (int j=0;j<=bestlist_i;j++)
		{
			printf("site:%d midbound:%.3f leftbound:%.3f rightbound:%.3f\n",bestjlist[j],(bestboundleftlist[j]+bestboundrightlist[j])/2,bestboundleftlist[j],bestboundrightlist[j]);
			int maxp0 = 0;
			int maxq0 = 0;
			for (int i=0;i<keynum;i++)
			{
				if (data[bestjlist[j]][i].bias<(bestboundleftlist[j]+bestboundrightlist[j])/2)
				{
					if (data[bestjlist[j]][i].flag==0) maxp0++;
					if (data[bestjlist[j]][i].flag==1) maxq0++;
				}
			}
			double maxpp0 = maxp0*1.0/(keynum/2);
			double maxqq0 = maxq0*1.0/(keynum/2);
			if (n==0) printf("p1:%.3f p2:%.3f p_v2[k]:%.3f\n",maxpp0,maxqq0,(1+myabs(maxpp0-maxqq0))/2);
			if (n==1) printf("q1:%.3f q2:%.3f p_v2[k]:%.3f\n",maxpp0,maxqq0,(1+myabs(maxpp0-maxqq0))/2);
		}
	}
	else
	{
		printf("site:%d midbound:%.3f leftbound:%.3f rightbound:%.3f\n",maxj,(maxleftbound+maxrightbound)/2,maxleftbound,maxrightbound);
		int maxp0 = 0;
		int maxq0 = 0;
		for (int i=0;i<keynum;i++)
		{
			if (data[maxj][i].bias<(maxleftbound+maxrightbound)/2)
			{
				if (data[maxj][i].flag==0) maxp0++;
				if (data[maxj][i].flag==1) maxq0++;
			}
		}
		double maxpp0 = maxp0*1.0/(keynum/2);
		double maxqq0 = maxq0*1.0/(keynum/2);
		if (n==0) printf("p1:%.3f p2:%.3f p_v2[k]:%.3f\n",maxpp0,maxqq0,maxp);
		if (n==1) printf("q1:%.3f q2:%.3f p_v2[k]:%.3f\n",maxpp0,maxqq0,maxp);
	}
	return 0;
}
