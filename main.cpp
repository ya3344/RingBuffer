// RingBuffer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "RingBuffer.h"
#include <windows.h>

using namespace std;

int main()
{
	RingBuffer ringBuffer;
	int inputRand, outputRand;
	char originalData[] = { "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345\n" };
	char* data = originalData;
	char* outputData = new char[strlen(originalData) + 1];
	int leftSize = strlen(originalData);
	int inputRet, outputRet, peekRet;

	srand(10);

	while (true)
	{
		if (leftSize <= 0)
		{
			data = originalData;
			leftSize = strlen(originalData) + 1;
		}

		inputRand = rand() & leftSize + 1;
		inputRet = ringBuffer.Enqueue(data, leftSize);
		data += inputRet;

		outputRand = rand() & leftSize + 1;

		ZeroMemory(outputData, strlen(originalData));
		//peekRet = ringBuffer.Peek(outputData, ringBuffer.GetUseSize());
		//ringBuffer.MoveReadPos(peekRet);
		outputRet = ringBuffer.Dequeue(outputData, ringBuffer.GetUseSize());
		leftSize -= outputRet;
		if (outputRet > 0)
			printf("%s", outputData);
	}
}
