/*
	Key Recovery Program - for 56-bit k_0 (k_0[0] to k_0[55])
	For any question, please email to he-l17@mails.tsinghua.edu.cn.
	
	This Program supposes that attackers have known 55 bits of k_1 (k_1[0] to k_1[54]).
	In actual scenes, attackers need to apply the same algorithm to all 2^55 cases.
	For the case matching k_1[0] to k_1[55], the program can correctly recover other key bits within 2^28 complexity with probability almost 1, \
	  or to say, 2^11 for k_0[0] to k_0[55] and 2^17 for remained 17 unrecovered bits.
	
	The algorithm will attempt to recover keynum(internal parameter) randomized keys.
	For each key, program tests 111 groups of e_j and each e_j costs 2^15 data complexity.
	Program can only work when predicting errors occurs within 2 test groups -- if over 3 bits do not match the prediction, \
	  the recovery program is truncatedly failed.
	
	Program can be directly executed without any operational parameters.
	
	Output Format
		One execution will produce one output file named "output.txt".
		In output.txt, the predicting information is recorded for each randomized key in the form below:
			1st line: binary 64-bit k_0
			2nd line: binary 64-bit k_1
			3rd line: recovered 56-bit k_0 (if succeeds)
			4th line: predicting process
		There are 4 cases of "predicting process":
			1. all correct (succeed)
			2. 1 bit misses (succeed)
			3. 2 bits miss (succeed)
			4. over 3 bits miss (fail)
		Finally the program will print the success rate of all randomized key receovery in the end.
*/

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<ctime>
using namespace std;

//int inputnum = 1048576;//2^20
//int halfinputnum = 524288;
int inputnum = 32768;//2^15
int halfinputnum = 16384;
int keynum = 10000;

int jsite[63] = {
	26,	27,	28,	29,	30,	31,	32,	33,	34,
	51,	52,	53,	54,	55,	40,	41,	42,	43,
	44,	45,	46,	47, 48,  1,  2,  3,  4,
	 5,  6,  7,	 8,  9, 58, 59, 60, 61,
	62, 63, 16,  0,	 2,  3,  4, 21, 22,
	 7,  8,  9, 10, 11,	12, 13, 14, 15,
	32, 17, 18, 18, 19, 37, 37, 38, 39
};

double bound[63][2] = {
	-5.205, -5.205,
	-5.165, -5.185,
	-5.075, -5.090,
	-5.095, -5.125,
	-5.415, -5.455,
	-5.400, -5.385,
	-4.535, -4.535,
	-4.445, -4.455,
	-4.385, -4.365,
	-6.005, -6.025,
	-6.025, -6.025,
	-5.895, -5.955,
	-6.005, -5.965,
	-5.915, -5.955,
	-4.995, -5.035,
	-5.195, -5.180,
	-5.270, -5.265,
	-5.290, -5.265,
	-5.225, -5.210,
	-5.145, -5.180,
	-5.155, -5.145,
	-5.205, -5.145,
	-5.155, -5.185,
	-4.905, -4.930,
	-5.005, -5.015,
	-5.155, -5.165,
	-5.545, -5.580,
	-6.030, -6.155,
	-6.125, -6.105,
	-6.060, -6.125,
	-6.120, -6.140,
	-6.105, -6.120,
	-5.205, -5.165,
	-5.175, -5.175,
	-5.120, -5.100,
	-5.215, -5.195,
	-5.190, -5.190,
	-5.155, -5.150,
	-5.710, -5.695, 
	-5.435, -5.445,
	-3.745, -3.765,
	-3.975, -3.965,
	-4.680, -4.740,
	-5.535, -5.525,
	-5.335, -5.270,
	-5.105, -5.130,
	-5.075, -5.115,
	-5.060, -5.030,
	-5.210, -5.175,
	-5.180, -5.185,
	-5.180, -5.165,
	-5.205, -5.175,
	-5.210, -5.210,
	-5.215, -5.200,
	-6.045, -6.085,
	-4.645, -4.655,
	-4.645, -4.635,
	-6.625, -6.655,
	-6.615, -6.595,
	-4.965, -5.105,
	-7.160, -7.145,
	-7.075, -7.030,
	-6.770, -6.765
};

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

unsigned long long simple_ran64() //64-bit randomize
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
unsigned long long simple_ran56_withpadding() //56-bit randomize
{
	//modular different Mersenne Prime to avoid circulation
	unsigned long long u,v,w,x,y,z,pad;
	u = rand()%31;
	v = rand()%31;
	w = rand()%127;
	x = rand()%8191;
	y = rand()%8191;
	z = rand()%8191;
	pad = 7;
	return ((pad<<56)|(u<<51)|(v<<46)|(w<<39)|(x<<26)|(y<<13)|z);
}

void print_longlong_in_binary(unsigned long long a)
{
	for (int j=63;j>=0;j--) cout<<get_pos_i(a,j);
	cout<<endl;
}
void fprint_longlong_in_binary(unsigned long long a,FILE* fout)
{
	for (int j=63;j>=0;j--) fprintf(fout,"%d",get_pos_i(a,j));
	fprintf(fout,"\n");
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

int goalbitlist[64];
int guesslist[64];
void getgoalbitlist()
{
	unsigned long long goal = h[2]^key1;
	for (int i=0;i<64;i++)
	{
		goalbitlist[i] = goal%2;
		goal = goal/2;
	}
}
bool keyoracle() //run behind getgoalbitlist()
{
	for (int i=0;i<=55;i++)
		if (guesslist[i]!=goalbitlist[i])
			return false;
	return true;
}

int predictresult[56][2];
void biastest() //fill predictresult
{
	int count = 0;
	double testbias[56][2];
	for (int inputcount=1;inputcount<=inputnum;inputcount++)
	{
		unsigned long long message = simple_ran56_withpadding();
		unsigned long long message_pie = flip_pos_i(message,0);
		unsigned long long output = siphash_2_1(message);
		unsigned long long output_pie = siphash_2_1(message_pie);
		unsigned long long diffrence = output^output_pie;
		if (get_pos_i(diffrence,jsite[0])==1) count++;
	}
	if (count==halfinputnum) testbias[0][0] = -21;
	else testbias[0][0] = log(abs(count-halfinputnum))/log(2)-log(inputnum)/log(2);
	if (testbias[0][0]<=bound[0][0]) predictresult[0][0] = 1;
	else predictresult[0][0] = 0;
	for (int i=1;i<56;i++)
	{
		for (int j=0;j<2;j++)
		{
			count = 0;
			for (int inputcount=1;inputcount<=inputnum;inputcount++)
			{
				unsigned long long message = simple_ran56_withpadding();
				if (j==0)
					if (get_pos_i(h[3]^key2^message,i-1)!=0) message = flip_pos_i(message,i-1);//v3[i-1] = 0	
				if (j==1)
					if (get_pos_i(h[3]^key2^message,i-1)!=1) message = flip_pos_i(message,i-1);//v3[i-1] = 1
				unsigned long long message_pie = flip_pos_i(message,i);
				unsigned long long output = siphash_2_1(message);
				unsigned long long output_pie = siphash_2_1(message_pie);
				unsigned long long diffrence = output^output_pie;
				if (get_pos_i(diffrence,jsite[i])==1) count++;
			}
			if (count==halfinputnum) testbias[i][j] = -21;
			else testbias[i][j] = log(abs(count-halfinputnum))/log(2)-log(inputnum)/log(2);
			if (j==0)
			{
				if (testbias[i][j]<=bound[i][j]) predictresult[i][j] = 1;
				else predictresult[i][j] = 0;
			}
			if (j==1)
			{
				if (testbias[i][j]<=bound[i][j]) predictresult[i][j] = 0;
				else predictresult[i][j] = 1;
			}
		}
	}
}

int allcorrect = 0;
int onebitmisses = 0;
int twobitmiss = 0;
int overthreebitmiss = 0;
void recover(FILE* fout)
{
	//all correct
	guesslist[0] = predictresult[0][0];
	for (int i=1;i<56;i++) guesslist[i] = predictresult[i][guesslist[i-1]];
	if (keyoracle())
	{
		allcorrect++;
		fprintf(fout,"                ");
		for (int i=55;i>=0;i--) fprintf(fout,"%d",(guesslist[i]+get_pos_i(h[2],i))%2);
		fprintf(fout,"\n");
		fprintf(fout,"all correct\n");
		return;
	}
	//1 bit misses
	for (int wrongsite=0;wrongsite<56;wrongsite++)
	{
		if (wrongsite==0) guesslist[0] = 1-predictresult[0][0];//regarded as wrong guess
		else guesslist[0] = predictresult[0][0];
		for (int i=1;i<56;i++)
			if (wrongsite==i) guesslist[i] = 1-predictresult[i][guesslist[i-1]];//regarded as wrong guess
			else guesslist[i] = predictresult[i][guesslist[i-1]];
		if (keyoracle())
		{
			onebitmisses++;
			fprintf(fout,"                ");
			for (int i=55;i>=0;i--) fprintf(fout,"%d",(guesslist[i]+get_pos_i(h[2],i))%2);
			fprintf(fout,"\n");
			fprintf(fout,"1 bit misses\n");
			return;
		}
	}
	//2 bit miss
	for (int wrongsite1=0;wrongsite1<55;wrongsite1++)
		for (int wrongsite2=wrongsite1+1;wrongsite2<56;wrongsite2++)
		{
			if ((wrongsite1==0)||(wrongsite2==0)) guesslist[0] = 1-predictresult[0][0];//regarded as wrong guess
			else guesslist[0] = predictresult[0][0];
			for (int i=1;i<56;i++)
				if ((wrongsite1==i)||(wrongsite2==i)) guesslist[i] = 1-predictresult[i][guesslist[i-1]];//regarded as wrong guess
				else guesslist[i] = predictresult[i][guesslist[i-1]];
			if (keyoracle())
			{
				twobitmiss++;
				fprintf(fout,"                ");
				for (int i=55;i>=0;i--) fprintf(fout,"%d",(guesslist[i]+get_pos_i(h[2],i))%2);
				fprintf(fout,"\n");
				fprintf(fout,"2 bit miss\n");
				return;
			}
		}
	overthreebitmiss++;
	fprintf(fout,"over 3 bits miss\n");
}

int main()
{
	srand((int)time(0));
	char filename[20] = "output.txt";
	FILE* fout = fopen(filename,"w");
	for (int i=1;i<=keynum;i++)
	{
		key1 = simple_ran64();
		key2 = simple_ran64();
		getgoalbitlist();
		biastest();
		fprint_longlong_in_binary(key1,fout);
		fprint_longlong_in_binary(key2,fout);
		recover(fout);
		fprintf(fout,"\n");
	}
	fprintf(fout,"all correct:%d\n",allcorrect);
	fprintf(fout,"1 bit misses:%d\n",onebitmisses);
	fprintf(fout,"2 bit miss:%d\n",twobitmiss);
	fprintf(fout,"over 3 bits miss:%d\n",overthreebitmiss);
	fclose(fout);
	return 0;
}


