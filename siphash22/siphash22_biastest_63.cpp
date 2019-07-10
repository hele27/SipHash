/*
	Distinguishing Program - Bias Test for Input Differential Bit 63
	For any question, please email to he-l17@mails.tsinghua.edu.cn.
	
	This program randomly chooses inputnum(internal parameter) pairs of input message differing on bit 63, \
	  or say input1^input2 = 0x1000000000000000, and calculates corresponding biases of output bit 57.
		(To reduce running time, here we make use of the conclusion of our teammates that \
		  under input diffential bit k, the ouput bit with the most significant bias locates on bit k+58 mod 64)
	All input messages are restricted into one 64-bit block.
	The authors suggest setting inputnum = 2^36, which is enough to check whether the output bias is greater than 2^-17.
	Test results provided by the authors with 256 keys are obtained under parameter inputnum = 2^40. 
	
	Program can be executed directly without any operational parameters.
	One execution only returns the result of one key in "sip22test_63.txt", yet costing several hours on an ordinary PC.
	It is suggested that prople who would like to recover the experiment have enough parallel computing resources.
	
	
	Output Format
		Each key contains 3 lines of information, \
		  with 2 lines of key information (64-bit per line) and 1 line of bias information (output bit 57).
		The line of bias information contains 2 number, the former is the output bit (57) and the latter is the output bias (exponential part).
	i.e.:
		1234567890abcdef
		fedcba0987654321
		57 -15.67
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
long long inputnum = 68719476736;//2^36
long long halfinputnum = 34359738368;
long long roundnum = 64;
long long roundinputnum = 1073741824;//2^30
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
unsigned long long siphash_2_2(unsigned long long m)
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
	//2-round finalization
	v[2] = v[2]^ff;
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
	//2-round finalization
	return (v[0]^v[1]^v[2]^v[3]);
} 
//siphash

int main()
{
	srand((int)time(0)); 
	long long counter_57 = 0;//output differential counter
	int k = 63;
	char filename[20] = "sip22test_63.txt";
	FILE* fout = fopen(filename,"w");
	key1 = simple_ran64();
	key2 = simple_ran64();
	for (int roundcount=1;roundcount<=roundnum;roundcount++)
	{
		for (int inputcount=1;inputcount<=roundinputnum;inputcount++)
		{
			unsigned long long message = simple_ran64();
			unsigned long long message_pie = flip_pos_i(message,k);
			unsigned long long output = siphash_2_2(message);
			unsigned long long output_pie = siphash_2_2(message_pie);
			unsigned long long diffrence = output^output_pie;
			if (get_pos_i(diffrence,57)==0) counter_57++;
		}
	}
	fprint_longlong_in_hex(key1,fout);
	fprint_longlong_in_hex(key2,fout);
	fprintf(fout,"57 %.2f\n",log(abs(counter_57-halfinputnum))/log(2)-log(inputnum)/log(2));
	fclose(fout);
	return 0;
}
