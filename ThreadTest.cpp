// RingBuffer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "RingBuffer.h"
#include <windows.h>
#include <process.h>
using namespace std;

enum THRAD_INDEX
{
	ENQUEUE_THREAD = 0,
	//ENQUEUE_THREAD_1,
	//ENQUEUE_THREAD_2,
	DEQUEUE_THREAD,
	DEQUEUE_THREAD_1,
	DEQUEUE_THREAD_2,
	THREAD_MAX
};

bool gShutdown = false;
HANDLE gMenualResetEvent = NULL;
HANDLE thread[THREAD_MAX];
RingBuffer gRingBuffer;
char gOriginalData[] = { "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345\n" };
long gLeftSize = 0;
WORD gDataSize = strlen(gOriginalData) + 1;

unsigned __stdcall EnqueueThread(void* arguments)
{
	int inputRet;
	char* data = nullptr;
	
	while (gShutdown == false)
	{
		//if(gRingBuffer.GetFreeSize() >= gDataSize)
		inputRet = gRingBuffer.Enqueue(gOriginalData, gDataSize);
		if (inputRet < 0)
		{
			if (inputRet != RingBuffer::DATA_FULL)
				_ASSERT(false);
		}	
	}

	return 0;
}

unsigned __stdcall DequeueThread(void* arguments)
{
	int dequeueRet;
	char* outputData = new char[strlen(gOriginalData) + 1];
	int size;
	while (gShutdown == false)
	{
		size = gRingBuffer.GetUseSize() < gDataSize ? gRingBuffer.GetUseSize() : gDataSize;
		dequeueRet = gRingBuffer.LockDequeue(outputData, size);
		if (dequeueRet > 0)
			printf("%s", outputData);

		if (dequeueRet == RingBuffer::USE_COUNT_UNDER_FLOW)
			_ASSERT(false);
	}

	return 0;
}

int main()
{
	unsigned int threadID[THREAD_MAX];

	thread[ENQUEUE_THREAD] = (HANDLE)_beginthreadex(NULL, 0, &EnqueueThread, NULL, 0, &threadID[ENQUEUE_THREAD]);
	//thread[ENQUEUE_THREAD_1] = (HANDLE)_beginthreadex(NULL, 0, &EnqueueThread, NULL, 0, &threadID[ENQUEUE_THREAD_1]);
	//thread[ENQUEUE_THREAD_2] = (HANDLE)_beginthreadex(NULL, 0, &EnqueueThread, NULL, 0, &threadID[ENQUEUE_THREAD_2]);
	thread[DEQUEUE_THREAD] = (HANDLE)_beginthreadex(NULL, 0, &DequeueThread, NULL, 0, &threadID[DEQUEUE_THREAD]);
	thread[DEQUEUE_THREAD_1] = (HANDLE)_beginthreadex(NULL, 0, &DequeueThread, NULL, 0, &threadID[DEQUEUE_THREAD_1]);
	thread[DEQUEUE_THREAD_2] = (HANDLE)_beginthreadex(NULL, 0, &DequeueThread, NULL, 0, &threadID[DEQUEUE_THREAD_2]);

	//if (thread[ENQUEUE_THREAD] == NULL)
	//{
	//	printf("thread[ENQUEUE_THREAD] == NULL[GetLastError:%d]", GetLastError());
	//	return 0;
	//}
	//if (thread[DEQUEUE_THREAD] == NULL)
	//{
	//	printf("thread[DEQUEUE_THREAD] == NULL[GetLastError:%d]", GetLastError());
	//	return 0;
	//}
	
	WaitForMultipleObjects(THREAD_MAX, thread, TRUE, INFINITE);

	return 0;
}
