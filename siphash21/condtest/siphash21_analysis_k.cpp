/*
	Key Classification Program - Analysis
		(For Data Generation Program, see siphash21_condtest_k.cpp)
	For any question, please email to he-l17@mails.tsinghua.edu.cn.
	
	This program will load the result of Data Generation Program, \
	  find out the best boundline to recover v2[k], and calculate the expected success rate.
	Methods to choose the boundline and calculate the success rate are given in article "Cryptanalysis of Reduced-Round SipHash".
	Since the data complexity is only 2^20, output bias 2^-9 is required according to 4-sigma-principle, \
	  which means boundlines less than -9 will be filtered. 
	
	Program must be executed with 2 operational parameters k(operational parameter) and n(operational parameter), \
	  with file "sip21test_k_n.txt" existing under directory "./data".
	i.e.:
		./siphash21_analysis_k 07 0
		./siphash21_analysis_k 43 1
	
	Output Format
		Program only produces standard outputs.
		The first line appears after loading successfully.
		Next 64 lines show best boundlines and corresponding results using different output bits 0~63.
		Finally, program gives the best choice as well as proportions p1,q1,p2,q2 (same meaning as in article), \
		  followed with corresponding success rate of recovering v2[k] and v3[k-1]. 
		*If one line of output bit j shows "bound:0.000 p:0.000", \
		  that's because the biases under all keys are less than 2^-9, filtering all data instead of causing program mistakes.  
	i.e.:
		load ./data/sip21test_32_0.txt finished
		site:0 bound:0.000 p:0.000
		site:1 bound:-8.985 p:0.500
		...
		site:58 bound:-5.485 p:0.908
		...
		site:63 bound:-8.955 p:0.500
		
		best choice:
		site:58 bound:-5.485
		p1:0.000 q1:0.156 p2:0.966 q2:0.165 P_v2[k]:0.908 p_v3[k-1]:0.726
*/

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
using namespace std;

const int keynum = 4*4096;

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
		int site,flag,group;
		fin>>key1;
		fin>>key2;
		flag = i/(keynum/4);
		for (int j=0;j<64;j++)
		{
			fin>>site;
			fin>>data[j][i].bias;
			data[j][i].flag = flag;
		}
	}
	fin.close();
	cout<<"load "<<name<<" finished\n";
}

double mymax(double a,double b)
{
	if (a>b) return a;
	else return b;
}

int main(int argc, char* argv[])
{
	//filename
	char filename[40] = "./data/sip21test_";
	strcat(filename,argv[1]);
	strcat(filename,"_");
	strcat(filename,argv[2]);
	strcat(filename,".txt");
	loaddata(filename);
	//analyze
	int maxj = -1;
	double maxp = 0;
	double maxbound = 0.0;
	for (int j=0;j<64;j++)
	{
		sortdata(j);
		double maxp_ofj = 0;
		double maxbound_ofj = 0.0;
		int p0 = 0;
		int q0 = 0;
		int p1 = 0;
		int q1 = 0;
		for (int i=0;i<keynum;i++)
		{
			if (data[j][i].flag==0) p0++;
			if (data[j][i].flag==1) q0++;
			if (data[j][i].flag==2) p1++;
			if (data[j][i].flag==3) q1++;
			if (data[j][i].bias<-9) continue;
			if ((i<keynum-1)&&(data[j][i+1].bias==data[j][i].bias)) continue;
			double pp0 = p0*1.0/(keynum/4);
			double qq0 = q0*1.0/(keynum/4);
			double pp1 = p1*1.0/(keynum/4);
			double qq1 = q1*1.0/(keynum/4);
			double p = (mymax(pp0*qq0,pp1*qq1)+mymax(pp0+qq0-2*pp0*qq0,pp1+qq1-2*pp1*qq1)+mymax((1-pp0)*(1-qq0),(1-pp1)*(1-qq1)))/2;
			if (p>maxp_ofj)
			{
				maxp_ofj = p;
				maxbound_ofj = data[j][i].bias+0.005;
			}
		}
		printf("site:%d bound:%.3f p:%.3f\n",j,maxbound_ofj,maxp_ofj);
		if (maxp_ofj>maxp)
		{
			maxj = j;
			maxp = maxp_ofj;
			maxbound = maxbound_ofj;
		}
	}
	printf("\n");
	printf("best choice:\n");
	printf("site:%d bound:%.3f\n",maxj,maxbound);
	int maxp0 = 0;
	int maxq0 = 0;
	int maxp1 = 0;
	int maxq1 = 0;
	for (int i=0;i<keynum;i++)
	{
		if (data[maxj][i].bias<maxbound)
		{
			if (data[maxj][i].flag==0) maxp0++;
			if (data[maxj][i].flag==1) maxq0++;
			if (data[maxj][i].flag==2) maxp1++;
			if (data[maxj][i].flag==3) maxq1++;
		}
	}
	double maxpp0 = maxp0*1.0/(keynum/4);
	double maxqq0 = maxq0*1.0/(keynum/4);
	double maxpp1 = maxp1*1.0/(keynum/4);
	double maxqq1 = maxq1*1.0/(keynum/4);
	printf("p1:%.3f q1:%.3f p2:%.3f q2:%.3f p_v2[k]:%.3f ",maxpp0,maxqq0,maxpp1,maxqq1,maxp);
	double max1 = mymax(maxpp0,maxqq0)/(maxpp0+maxqq0);
	double max2 = mymax(1-maxpp0,1-maxqq0)/(2-maxpp0-maxqq0);
	double max3 = mymax(maxpp1,maxqq1)/(maxpp1+maxqq1);
	double max4 = mymax(1-maxpp1,1-maxqq1)/(2-maxpp1-maxqq1);
	double p0_v3 = (1-maxpp0-maxqq0+2*maxpp0*maxqq0)*0.5+(maxpp0+maxqq0-2*maxpp0*maxqq0)*mymax(max1,max2);
	double p1_v3 = (1-maxpp1-maxqq1+2*maxpp1*maxqq1)*0.5+(maxpp1+maxqq1-2*maxpp1*maxqq1)*mymax(max3,max4);
	double p_v3 = (p0_v3+p1_v3)/2;
	printf("p_v3[k-1]:%.3f\n",p_v3);
	return 0;
}
