/*
	Bias Test Program - Data Generation
		(For Analysis Program, see siphash21_analysis.cpp)
	For any question, please email to he-l17@mails.tsinghua.edu.cn.
	
	This program will generate a group of data, with keynum(internal parameter) randomized keys.
	For each key, program randomly chooses inputnum(internal parameter) pairs of input message differing on bit k(operational parameter), \
	  or say input1^input2 = 1<<k, and calculates corresponding biases of all 64 output bits.
	All input messages are restricted into one 64-bit block.
	Those internal parameters can be directly modified in the first page, \
	  but the authors still suggest the original version inputnum = 1048576 and keynum = 4096.
	
	Program must be executed with 1 operational parameter k(operational parameter) where k:00~63 indicates the input diffenrential bit.
	i.e.:
		./siphash21_biastest 07
		./siphash21_biastest 43
	
	Output Format
		One execution will produce one output file named "sip21test_k.txt", where k can discriminate different execution.
		Each output file records the information of keynum keys.
		Each key contains 66 lines of information, \
		  with 2 lines of key information (64-bit per line) and 64 lines of bias information (one output bit per line).
		Each line of bias information contains 2 number, the former is the output bit and the latter is the output bias (exponential part).
	i.e.:
		1234567890abcdef
		fedcba0987654321
		00 -12.34
		01 -15.67
		02 -6.73
		...
		63 -9.83
	
	The entire experiment needs 64 times of executions, while one execution costs several hours on an ordinary PC.
	It is suggested that prople who would like to recover the experiment have enough parallel computing resources.
*/

#include<iostream>
#include<fstream>
#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<ctime>

using namespace std;

//internal parameters
long long inputnum = 1048576;//2^20
long long halfinputnum = 524288;
int keynum = 4096;
//internal parameters

unsigned long long leftrotate(unsigned long long a,int length)  //64-bit left-rotate 
{
	return ((a<<length)+(a>>(64-length))); 
}
unsigned long long rightrotate(unsigned long long a,int length)  //64-bit right-rotate
{
	return ((a<<(64-length))+(a>>length));
}
int get_pos_i(unsigned long long a,int i)  //64-bit get-bit
{
	unsigned long long yi = 0x1;
	return ((a&(yi<<i))>>i);
}
unsigned long long flip_pos_i(unsigned long long a,int i)  //64-bit flip-bit
{
	unsigned long long yi = 0x1;
	return (a^(yi<<i));
}

unsigned long long simple_ran64()  //64-bit randomize
{
	//modular different Mersenne Prime to avoid circulation
	unsigned long long u,v,w,x,y,z;
	u = rand()%31;
	v = rand()%127;
	w = rand()%8191;
	x = rand()%8191;
	y = rand()%8191;
	z = rand()%8191;
	return ((u<<59)|(v<<52)|(w<<39)|(x<<26)|(y<<13)|z);
}

void print_longlong_in_binary(unsigned long long a)
{
	for (int j=63;j>=0;j--) cout<<get_pos_i(a,j);
	cout<<endl;
}
void print_longlong_in_hex(unsigned long long a)
{
	unsigned int left32 = (unsigned int)(a>>32);
	unsigned int right32 = (unsigned int)((a<<32)>>32);
	printf("%08x%08x\n",left32,right32);
}
void fprint_longlong_in_hex(unsigned long long a,FILE* fout)
{ 
	unsigned int left32 = (unsigned int)(a>>32);
	unsigned int right32 = (unsigned int)((a<<32)>>32);
	fprintf(fout,"%08x%08x\n",left32,right32);
}

//siphash
unsigned long long key1,key2;
unsigned long long ff = 0x00000000000000ff;
unsigned long long h[4] = {0x736f6d6570736575,0x646f72616e646f6d,0x6c7967656e657261,0x7465646279746573};
unsigned long long v[4];
unsigned long long siphash_2_1(unsigned long long m)
{
	//init
	v[0] = h[0]^key1;
	v[1] = h[1]^key2;
	v[2] = h[2]^key1;
	v[3] = h[3]^key2;
	//init
	//2-round compression
	v[3] = v[3]^m;
	for (int i=1;i<=2;i++)
	{
		v[0] = v[0]+v[1];
		v[2] = v[2]+v[3];
		v[1] = leftrotate(v[1],13);
		v[3] = leftrotate(v[3],16);
		v[1] = v[0]^v[1];
		v[3] = v[2]^v[3];
		v[0] = leftrotate(v[0],32);
		//halfround
		v[2] = v[1]+v[2];
		v[0] = v[0]+v[3];
		v[1] = leftrotate(v[1],17);
		v[3] = leftrotate(v[3],21);
		v[1] = v[1]^v[2];
		v[3] = v[0]^v[3];
		v[2] = leftrotate(v[2],32);
	}
	v[0] = v[0]^m;
	//2-round compression
	//1-round finalization
	v[2] = v[2]^ff;
	for (int i=1;i<=1;i++)
	{
		v[0] = v[0]+v[1];
		v[2] = v[2]+v[3];
		v[1] = leftrotate(v[1],13);
		v[3] = leftrotate(v[3],16);
		v[1] = v[0]^v[1];
		v[3] = v[2]^v[3];
		v[0] = leftrotate(v[0],32);
		//halfround
		v[2] = v[1]+v[2];
		v[0] = v[0]+v[3];
		v[1] = leftrotate(v[1],17);
		v[3] = leftrotate(v[3],21);
		v[1] = v[1]^v[2];
		v[3] = v[0]^v[3];
		v[2] = leftrotate(v[2],32);
	}
	//1-round finalization
	return (v[0]^v[1]^v[2]^v[3]);
} 
//siphash

char knum[10];
void myitoa(int k)
{
	for (int i=0;i<9;i++) knum[i] = '\0';
	knum[0] = '0'+k/10;
	knum[1] = '0'+k%10;
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
	srand((int)time(0));
	long long counter[64];//output differential counter
	//load k
	int k = chartoint(argv[1]);
	//filename
	char filename[20] = "sip21test_";
	strcat(filename,argv[1]);
	strcat(filename,".txt");
	FILE* fout = fopen(filename,"w");
	//test for the k-th input differential bit
	for (int keycount=0;keycount<keynum;keycount++)
	{
		//init
		for (int j=0;j<64;j++) counter[j] = 0;
		key1 = simple_ran64();
		key2 = simple_ran64();
		//test
		for (int inputcount=1;inputcount<=inputnum;inputcount++)
		{
			unsigned long long message = simple_ran64();
			unsigned long long message_pie = flip_pos_i(message,k);
			unsigned long long output = siphash_2_1(message);
			unsigned long long output_pie = siphash_2_1(message_pie);
			unsigned long long diffrence = output^output_pie;
			for (int j=0;j<64;j++)
				if (get_pos_i(diffrence,j)==1) counter[j]++;
		}
		//output
		fprint_longlong_in_hex(key1,fout);
		fprint_longlong_in_hex(key2,fout);
		for (int j=0;j<64;j++)
		{
			if (j>9) fprintf(fout,"%d ",j);
			else fprintf(fout,"0%d ",j);
			if (counter[j]==halfinputnum) fprintf(fout,"-21.00\n");
			else fprintf(fout,"%.2f\n",log(abs(counter[j]-halfinputnum))/log(2)-log(inputnum)/log(2));
		}
		fprintf(fout,"\n");
	}
	fclose(fout);
	return 0;
} 
