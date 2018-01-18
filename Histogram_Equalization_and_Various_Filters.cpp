#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <algorithm>
#include <string>

using namespace std;

#define BASESIZE 256
#define BASESIZE_HEIGHT 256
#define BASESIZE_WIDTH 256
#define PI 3.14
#define NFILTERSIZE 3

void FileRead(FILE* Input, unsigned char** In, string filename);
void FileWrite(FILE* Output, unsigned char** Out, string filename);

void getHistogramEqualization(unsigned char** In, unsigned char** Ori_histogram, unsigned char** Out_histogram, unsigned char** Out_Result, int nHeight_in, int nWidth_in);
void DisplayHistogram(int *Hist, unsigned char** Out);

void AverageFilter(unsigned char** In, unsigned char** Out, int nHeight, int nWidth, int nFilterSize);
void SmoothFilter(unsigned char** In, unsigned char** Out, int nHeight, int nWidth);
void MedianFilter(unsigned char **In, unsigned char **Out, int nHeight, int nWidth, int nFilterSize);
void SharpenFilter(unsigned char** In, unsigned char** Out, int nHeight, int nWidth);

unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nInitVal);
void MemFree2D(unsigned char **Mem, int nHeight);
bool isInsideBoundary(int nHeight, int nWidth, double h, double w);
unsigned char** Padding(unsigned char** In, int nHeight, int nWidth, int nFilterSize);

int main()
{
	cout << "파일을 생성중입니다..." << endl;

	FILE *Input, *Output;
	unsigned char** ch_in_gray = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	fopen_s(&Input, "input.raw", "rb");
	if (Input == NULL) {
		cout << "Input File open failed";
		return 0;
	}
	for (int h = 0; h < BASESIZE_HEIGHT; h++)
		fread(ch_in_gray[h], sizeof(unsigned char), BASESIZE_WIDTH, Input);

	unsigned char** Ori_histogram = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Out_histogram = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Out_Result = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);

	getHistogramEqualization(ch_in_gray, Ori_histogram, Out_histogram, Out_Result, BASESIZE_HEIGHT, BASESIZE_WIDTH);

	fopen_s(&Output, "Ori_histogram.raw", "wb");
	if (Output == NULL) {
		cout << "Output File open failed";
		return 0;
	}
	for (int i = 0; i < BASESIZE_HEIGHT; i++)
		fwrite(Ori_histogram[i], sizeof(unsigned char), BASESIZE_WIDTH, Output);
	
	FileWrite(Output, Out_histogram, "Out_histogram.raw");
	FileWrite(Output, Out_Result, "Out_Result.raw");
/************************************************************************************/
	MemFree2D(ch_in_gray, BASESIZE_HEIGHT);
	ch_in_gray = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Out_3by3 = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Out_7by7 = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Smooth_Result = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Sharpen_Result = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);

	FileRead(Input, ch_in_gray, "lena256.raw");
	AverageFilter(ch_in_gray, Out_3by3, BASESIZE_HEIGHT, BASESIZE_WIDTH, 3);
	FileWrite(Output, Out_3by3, "Out_3by3.raw");

	AverageFilter(ch_in_gray, Out_7by7, BASESIZE_HEIGHT, BASESIZE_WIDTH, 7);
	FileWrite(Output, Out_7by7, "Out_7by7.raw");

	SmoothFilter(ch_in_gray, Smooth_Result, BASESIZE_HEIGHT, BASESIZE_WIDTH);
	FileWrite(Output, Smooth_Result, "Smooth_Result.raw");

	SharpenFilter(Out_3by3, Sharpen_Result, BASESIZE_HEIGHT, BASESIZE_WIDTH);
	FileWrite(Output, Sharpen_Result, "Sharpen_Result.raw");
/************************************************************************************/
	unsigned char** Median_n5 = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Median_n10 = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Median_n25 = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);

	MemFree2D(ch_in_gray, BASESIZE_HEIGHT);
	ch_in_gray = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);
	unsigned char** Median_Result = MemAlloc2D(BASESIZE_HEIGHT, BASESIZE_WIDTH, 0);

	FileRead(Input, ch_in_gray, "lena256_n5.raw");
	MedianFilter(ch_in_gray, Median_n5, BASESIZE_HEIGHT, BASESIZE_WIDTH,3);
	FileWrite(Output, Median_n5, "Median_n5.raw");

	FileRead(Input, ch_in_gray, "lena256_n10.raw");
	MedianFilter(ch_in_gray, Median_n10, BASESIZE_HEIGHT, BASESIZE_WIDTH, 3);
	FileWrite(Output, Median_n10, "Median_n10.raw");

	FileRead(Input, ch_in_gray, "lena256_n25.raw");
	MedianFilter(ch_in_gray, Median_n25, BASESIZE_HEIGHT, BASESIZE_WIDTH, 3);
	FileWrite(Output, Median_n25, "Median_n25.raw");
/************************************************************************************/
	fclose(Input);
	fclose(Output);

	MemFree2D(ch_in_gray, BASESIZE_HEIGHT);

	MemFree2D(Ori_histogram, BASESIZE_HEIGHT);
	MemFree2D(Out_histogram, BASESIZE_HEIGHT);
	MemFree2D(Out_Result, BASESIZE_HEIGHT);

	MemFree2D(Out_3by3, BASESIZE_HEIGHT);
	MemFree2D(Out_7by7, BASESIZE_HEIGHT);
	MemFree2D(Smooth_Result, BASESIZE_HEIGHT);
	MemFree2D(Sharpen_Result, BASESIZE_HEIGHT);

	MemFree2D(Median_n5, BASESIZE_HEIGHT);
	MemFree2D(Median_n10, BASESIZE_HEIGHT);
	MemFree2D(Median_n25, BASESIZE_HEIGHT);

	cout << "완료하였습니다..." << endl;
	return 0;
}

void FileRead(FILE* Input, unsigned char** In, string filename)
{
	const char* file_name = filename.c_str();

	fopen_s(&Input, file_name, "rb");
	if (Input == NULL) {
		cout << "Input File open failed";
		return;
	}
	for (int h = 0; h < BASESIZE_HEIGHT; h++)
		fread(In[h], sizeof(unsigned char), BASESIZE_WIDTH, Input);

}

void FileWrite(FILE* Output, unsigned char** Out, string filename)
{
	const char* file_name = filename.c_str();

	fopen_s(&Output, file_name, "wb");
	if (Output == NULL) {
		cout << "Output File open failed";
		return;
	}
	for (int i = 0; i < BASESIZE_HEIGHT; i++)
		fwrite(Out[i], sizeof(unsigned char), BASESIZE_WIDTH, Output);

}

void SharpenFilter(unsigned char** In, unsigned char** Out, int nHeight, int nWidth)
{
	int Mask[3][3] = {
		{ 0,-1,0 },
		{ -1,5,-1 },
		{ 0,-1,0 }
	};
	int nTemp = 0, nPadSize = (int)(3 / 2);
	unsigned char** In_Pad = Padding(In, nHeight, nWidth, 3);

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			nTemp = 0;
			for (int n = 0; n < 3; n++)
			{
				for (int m = 0; m < 3; m++)
				{
					nTemp += In_Pad[h + n][w + m] * Mask[n][m];
				}
			}
			if (nTemp > 255)nTemp = 255;
			else if (nTemp < 0)nTemp = 0;

			Out[h][w] = static_cast<unsigned char>(nTemp);
		}
	}
	MemFree2D(In_Pad, nHeight + 2 * nPadSize);
}

void MedianFilter(unsigned char **In, unsigned char **Out, int nHeight, int nWidth, int nFilterSize)
{
	int nPadSize = (int)(nFilterSize / 2);
	int med = (int)(nFilterSize*nFilterSize / 2);
	unsigned char** In_Pad = Padding(In, nHeight, nWidth, nFilterSize);
	unsigned char* sorting = new unsigned char[nFilterSize*nFilterSize];
	
	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			int sort_num = 0;
			for (int n = 0; n < nFilterSize; n++)
			{
				for (int m = 0; m< nFilterSize; m++)
				{
					sorting[sort_num] = In_Pad[h + n][w + m];
					sort_num++;
				}
			}

			sort(sorting, sorting + nFilterSize*nFilterSize);
			Out[h][w] = sorting[med];
		}
	}

	MemFree2D(In_Pad, nHeight + 2 * nPadSize);
}

void SmoothFilter(unsigned char** In, unsigned char** Out, int nHeight, int nWidth)
{
	int nTemp = 0, nPadSize = (int)(3 / 2);
	unsigned char** In_Pad = Padding(In, nHeight, nWidth, 3);

	double Mask[3][3] = {
		{ 1, 2, 1 },
		{ 2 , 4 , 2 },
		{ 1, 2 , 1 }
	};

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			nTemp = 0;
			for (int n = 0; n < 3; n++)
			{
				for (int m = 0; m < 3; m++)
				{
					nTemp += In_Pad[h + n][w + m] * Mask[n][m];
				}
			}
			
			Out[h][w] = static_cast<unsigned char>(nTemp / 16);
		}
	}

	MemFree2D(In_Pad, nHeight + 2 * nPadSize);
}

void AverageFilter(unsigned char** In, unsigned char** Out, int nHeight, int nWidth, int nFilterSize)
{
	int nTemp = 0, nPadSize = (int)(nFilterSize / 2);
	unsigned char** In_Pad = Padding(In, nHeight, nWidth, nFilterSize);

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			nTemp = 0;
			for (int n = 0; n < nFilterSize; n++)
			{
				for (int m = 0; m < nFilterSize; m++)
				{
					nTemp += In_Pad[h + n][w + m];
				}
			}
			
			Out[h][w] = static_cast<unsigned char>(nTemp / (nFilterSize*nFilterSize));
		}
	}

	MemFree2D(In_Pad, nHeight + 2 * nPadSize);
}

void getHistogramEqualization(unsigned char** In, unsigned char** Ori_histogram, unsigned char** Out_histogram, unsigned char** Out_Result, int nHeight_in, int nWidth_in)
{
	int *Hist = new int[256];
	memset(Hist, 0, sizeof(int) * 256);

	//GetHistogram
	for (int h = 0; h < nHeight_in; h++)
	{
		for (int w = 0; w < nWidth_in; w++)
		{
			Hist[In[h][w]]++;
		}
	}

	//Histogram Equalization + Result
	double Hist_CDF[256] = { 0.0 };
	int sum = 0;
	for (int i = 0; i < 256; i++)
	{
		sum += Hist[i];
		Hist_CDF[i] = (double)sum / (nHeight_in *nWidth_in);
	}
	for (int h = 0; h < nHeight_in; h++)
	{
		for (int w = 0; w < nWidth_in; w++)
		{
			Out_Result[h][w] = Hist_CDF[In[h][w]] * 255;
		}
	}

	//Result Histogram
	int *Hist_Eq = new int[256];
	memset(Hist_Eq, 0, sizeof(int) * 256);

	for (int h = 0; h < nHeight_in; h++)
	{
		for (int w = 0; w < nWidth_in; w++)
		{
			Hist_Eq[Out_Result[h][w]]++;
		}
	}

	DisplayHistogram(Hist, Ori_histogram);
	DisplayHistogram(Hist_Eq, Out_histogram);
}

void DisplayHistogram(int *Hist, unsigned char** Out)
{
	//Normalization
	int nMax = 0;

	for (int n = 0; n < 256; n++)
	{
		if (nMax < Hist[n])
		{
			nMax = Hist[n];
		}
	}

	double dNormalizeFactor = 255.0 / nMax;

	for (int w = 0; w < 256; w++)
	{
		int nNormalizedValue = (int)Hist[w] * dNormalizeFactor;
		for (int h = 255; h > 255 - nNormalizedValue; h--)
		{
			Out[h][w] = 255;
		}
	}
}

unsigned char** MemAlloc2D(int nHeight, int nWidth, unsigned char nInitVal)
{
	unsigned char** rtn = new unsigned char*[nHeight];
	for (int n = 0; n < nHeight; n++)
	{
		rtn[n] = new unsigned char[nWidth];
		memset(rtn[n], nInitVal, sizeof(unsigned char) * nWidth);
	}
	return rtn;
}

void MemFree2D(unsigned char **Mem, int nHeight)
{
	for (int n = 0; n < nHeight; n++)
	{
		delete[] Mem[n];
	}
	delete[] Mem;
}

bool isInsideBoundary(int nHeight, int nWidth, double h, double w)
{
	if (h >= 0 && w >= 0 && h < nHeight && w < nWidth)
	{
		return true;
	}
	else return false;
}

unsigned char** Padding(unsigned char** In, int nHeight, int nWidth, int nFilterSize)
{
	int nPadSize = (int)(nFilterSize / 2);
	unsigned char** Pad = MemAlloc2D(nHeight + 2 * nPadSize, nWidth + 2 * nPadSize, 0);

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			Pad[h + nPadSize][w + nPadSize] = In[h][w];
		}
	}
	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nWidth; w++)
		{
			Pad[h][w + nPadSize] = In[0][w];
			Pad[h + (nHeight - 1) + nPadSize][w + nPadSize] = In[nHeight - 1][w];
		}
	}

	for (int h = 0; h < nHeight; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h + nPadSize][w] = In[h][0];
			Pad[h + nPadSize][w + (nWidth - 1) + nPadSize] = In[h][nWidth - 1];
		}
	}

	for (int h = 0; h < nPadSize; h++)
	{
		for (int w = 0; w < nPadSize; w++)
		{
			Pad[h][w] = In[0][0];
			Pad[h + (nHeight - 1) + nPadSize][w] = In[nHeight - 1][0];
			Pad[h][w + (nWidth - 1) + nPadSize] = In[0][nWidth - 1];
			Pad[h + (nHeight - 1) + nPadSize][w + (nWidth - 1) + nPadSize] = In[nHeight - 1][nWidth - 1];
		}
	}

	return Pad;
}