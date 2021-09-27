// RingBuffer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "RingBuffer.h"

int main()
{
	RingBuffer ringBuffer;
	int inputRand, outputRand;
	char originalData[] = { "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345" };
	char* data = originalData;
	char* outputData = new char[strlen(originalData) + 1];
	int leftSize = strlen(originalData);
	int inputRet, outputRet, peekRet;

	srand(10);

	while (true)
	{
		if (leftSize == 0)
		{
			data = originalData;
			leftSize = strlen(originalData);
		}

		inputRand = rand() & leftSize + 1;
		inputRet = ringBuffer.Enqueue(data, inputRand);
		data += inputRet;

		outputRand = rand() & leftSize + 1;

		ZeroMemory(outputData, strlen(originalData));
		peekRet = ringBuffer.Peek(outputData, outputRand);
		leftSize -= inputRet;

		outputRet = ringBuffer.Dequeue(outputData, outputRand);

		if (outputRet > 0)
			printf("%s", outputData);
	}
}
