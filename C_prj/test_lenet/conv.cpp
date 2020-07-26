#include "conv.h"
#include "math.h"
float _tanh(float x) {

	float exp2x = expf(2 * x) + 1;
	return (exp2x - 2) / (exp2x);
}

/////////////////////////////////////////////////////////////////////��һ�����͵ڶ���ػ�
static float IN1[32][32] = { 0 };
static float W1[6][1][5][5] = { 0 };
static float Bias1[6] = { 0 };
static float OUT1[6][28][28] = { 0 };

static float W2_P[24] = { 0 };
static float Bias2_P[6] = { 0 };
static float OUT2[6][14][14] = { 0 };

void conv_1_pool2(float* In_DRAM, float* W_DRAM, float* Out_DRAM, float* Bias_DRAM,float* W2_P_DRAM, float* Bias2_P_DRAM)
{
	memcpy((void*)Bias1, (const void*)(Bias_DRAM), sizeof(float) * 6);
	memcpy((void*)IN1, (const void*)(In_DRAM), sizeof(float) * 32*32);
	memcpy((void*)W1, (const void*)(W_DRAM ), sizeof(float) * 6*5*5);

	memset(OUT1,0,sizeof(OUT1));

	for (int kr = 0; kr < 5; kr++)
	{
		for (int kc = 0; kc < 5; kc++)
		{
			for (int r = 0; r < 28; r++)
			{
				for (int c = 0; c < 28; c++)
				{
					for (int cho = 0; cho < 6; cho++)
					{
						OUT1[cho][r][c] += IN1[r + kr][c + kc] * W1[cho][0][kr][kc];
					}
				}
			}
		}
	}
	for (int r = 0; r < 28; r++)
	{
		for (int c = 0; c < 28; c++)
		{
			for (int cho = 0; cho < 6; cho++)
			{
				OUT1[cho][r][c] += Bias1[cho];
				OUT1[cho][r][c] = _tanh(OUT1[cho][r][c]);

			}
		}
	}
	/////////////////////////�ػ�
	memcpy((void*)Bias2_P, (const void*)(Bias2_P_DRAM), sizeof(float) * 6);
	memcpy((void*)W2_P, (const void*)(W2_P_DRAM), sizeof(float) * 24);

	for (int r = 0; r < 14; r++)
	{
		for (int c = 0; c < 14; c++)
		{
			for (int cho = 0; cho < 6; cho++)
			{
				OUT2[cho][r][c] = (OUT1[cho][r<<1 + 0][c<<1 + 0]
							+ OUT1[cho][r << 1 + 0][c << 1 + 1]
							+ OUT1[cho][r << 1 + 1][c << 1 + 0]
							+ OUT1[cho][r << 1 + 1][c << 1 + 1] )/4;

				OUT2[cho][r][c] *= W2_P[cho] ;
				OUT2[cho][r][c] += Bias2_P[cho];
				OUT2[cho][r][c] = tanf(OUT2[cho][r][c]);
			}
		}
	}
	memcpy((void*)(Out_DRAM), (const void*)OUT2, sizeof(float) * 6*14*14);
}


/////////////////////////////////////////////////////////////////////���������͵��Ĳ�ػ�
static float IN3[6][14][14] = { 0 };
static float W3[16][6][5][5] = { 0 };
static float Bias3[16] = { 0 };
static float OUT3[16][10][10] = { 0 };

static float W4_P[16*4] = { 0 };
static float Bias4_P[16] = { 0 };
static float OUT4[16][5][5] = { 0 };

void conv_3_pool4(float* In_DRAM, float* W_DRAM, float* Out_DRAM, float* Bias_DRAM, float* W2_P_DRAM, float* Bias2_P_DRAM)
{
	memcpy((void*)Bias3, (const void*)(Bias_DRAM), sizeof(float) * 16);
	memcpy((void*)IN3, (const void*)(In_DRAM), sizeof(float) * 6*14 * 14);
	memcpy((void*)W3, (const void*)(W_DRAM), sizeof(float)*16 * 6 * 5 * 5);

	memset(OUT3, 0, sizeof(OUT3));

	for (int kr = 0; kr < 5; kr++)
	{
		for (int kc = 0; kc < 5; kc++)
		{
			for (int r = 0; r < 10; r++)
			{
				for (int c = 0; c < 10; c++)
				{
					for (int chi = 0; chi < 6; chi++)
					{
						for (int cho = 0; cho < 16; cho++)
						{
							OUT3[cho][r][c] += IN3[chi][r + kr][c + kc] * W3[cho][chi][kr][kc];
						}
					}

				}
			}
		}
	}
	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < 10; c++)
		{
			for (int cho = 0; cho < 16; cho++)
			{
				OUT3[cho][r][c] += Bias3[cho];
				OUT3[cho][r][c] = _tanh(OUT3[cho][r][c]);
			}
		}
	}
	/////////////////////////�ػ�
	memcpy((void*)Bias4_P, (const void*)(Bias2_P_DRAM), sizeof(float) * 16);
	memcpy((void*)W4_P, (const void*)(W2_P_DRAM), sizeof(float) * 16*4);

	for (int r = 0; r < 5; r++)
	{
		for (int c = 0; c < 5; c++)
		{
			for (int cho = 0; cho < 6; cho++)
			{
				OUT4[cho][r][c] = (OUT3[cho][r << 1 + 0][c << 1 + 0]
					+ OUT3[cho][r << 1 + 0][c << 1 + 1]
					+ OUT3[cho][r << 1 + 1][c << 1 + 0]
					+ OUT3[cho][r << 1 + 1][c << 1 + 1]) / 4;

				OUT4[cho][r][c] *= W4_P[cho];
				OUT4[cho][r][c] += Bias4_P[cho];
				OUT4[cho][r][c] = tanf(OUT4[cho][r][c]);
			}
		}
	}
	memcpy((void*)(Out_DRAM), (const void*)OUT4, sizeof(float) * 16 * 5 * 5);
}
/////////////////////////////////////////////////////////////////////�����
static float IN5[16][5][5] = { 0 };
static float W5[120][16][5][5] = { 0 };
static float Bias5[120] = { 0 };
static float OUT5[120] = { 0 };

void conv_5(float* In_DRAM, float* W_DRAM, float* Out_DRAM, float* Bias_DRAM )
{
	memcpy((void*)Bias5, (const void*)(Bias_DRAM), sizeof(float) * 120);
	memcpy((void*)IN5, (const void*)(In_DRAM), sizeof(float) * 16 * 5 * 5);
	memcpy((void*)W5, (const void*)(W_DRAM), sizeof(float) * 120 * 16 * 5 * 5);

	memset(OUT5, 0, sizeof(OUT5));

	for (int kr = 0; kr < 5; kr++)
	{
		for (int kc = 0; kc < 5; kc++)
		{
			for (int chi = 0; chi < 16; chi++)
			{
				for (int cho = 0; cho < 120; cho++)
				{
					OUT5[cho]  += IN5[chi][kr][kc] * W5[cho][chi][kr][kc];
				}
			}
		}
	}

	for (int cho = 0; cho < 120; cho++)
	{
		{
			OUT5[cho]  += Bias5[cho];
			OUT5[cho]  = _tanh(OUT5[cho] );
		}
	}
	
	memcpy((void*)(Out_DRAM), (const void*)OUT5, sizeof(float) * 120);
}
