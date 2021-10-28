// RingBuffer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include "RingBuffer.h"
#include <windows.h>
#include <process.h>
#include <random>
#include <list>
using namespace std;

enum THRAD_INDEX
{
	MAIN_THREAD = 0,
	WORK_THREAD,
	WORK_THREAD_1,
	WORK_THREAD_2,
	THREAD_MAX
};

enum MSG_TYPE_INDEX
{
	MSG_TYPE_ADD_STR = 0,
	MSG_TYPE_DEL_STR,
	MSG_TYPE_PRINT_LIST,
	MSG_TYPE_QUIT,
};

#define STRING_LENGTH 10

struct MsgHeadInfo
{
	short type = 0;
	short strLength = 0;
};

bool gShutdown = false;
HANDLE gMenualResetEvent = NULL;
HANDLE thread[THREAD_MAX];
RingBuffer gMsgQueue;
list<wstring> gList;

unsigned __stdcall WorkThread(void* arguments)
{
	//int inputRet;
	//char* data = nullptr;

	//while (gShutdown == false)
	//{
	//	//if(gRingBuffer.GetFreeSize() >= gDataSize)
	//	inputRet = gRingBuffer.Enqueue(gOriginalData, gDataSize);
	//	if (inputRet < 0)
	//	{
	//		if (inputRet != RingBuffer::DATA_FULL)
	//			_ASSERT(false);
	//	}
	//}
	//switch()


	return 0;
}

unsigned __stdcall DequeueThread(void* arguments)
{
	/*int dequeueRet;
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
	}*/

	return 0;
}

int main()
{
	unsigned int threadID[THREAD_MAX];
	random_device rd;
	mt19937 mtRand(rd());
	WCHAR originalData[] = { L"1234567890" };
	WCHAR data[STRING_LENGTH] = { 0 };

	//WORD dataSize = lstrlen(originalData);
	uniform_int_distribution<DWORD> lengthRange(1, lstrlen(originalData));  // 범위안의 난수발생
	MsgHeadInfo msgHeadInfo;


	//thread[ENQUEUE_THREAD] = (HANDLE)_beginthreadex(NULL, 0, &EnqueueThread, NULL, 0, &threadID[ENQUEUE_THREAD]);
	//thread[ENQUEUE_THREAD_1] = (HANDLE)_beginthreadex(NULL, 0, &EnqueueThread, NULL, 0, &threadID[ENQUEUE_THREAD_1]);
	//thread[ENQUEUE_THREAD_2] = (HANDLE)_beginthreadex(NULL, 0, &EnqueueThread, NULL, 0, &threadID[ENQUEUE_THREAD_2]);
	//thread[DEQUEUE_THREAD] = (HANDLE)_beginthreadex(NULL, 0, &DequeueThread, NULL, 0, &threadID[DEQUEUE_THREAD]);
	//thread[DEQUEUE_THREAD_1] = (HANDLE)_beginthreadex(NULL, 0, &DequeueThread, NULL, 0, &threadID[DEQUEUE_THREAD_1]);
	//thread[DEQUEUE_THREAD_2] = (HANDLE)_beginthreadex(NULL, 0, &DequeueThread, NULL, 0, &threadID[DEQUEUE_THREAD_2]);

	msgHeadInfo.type = MSG_TYPE_ADD_STR;
	msgHeadInfo.strLength = (short)lengthRange(mtRand);
	
	gMsgQueue.Enqueue((char*)&msgHeadInfo, sizeof(msgHeadInfo));
	if (msgHeadInfo.type == MSG_TYPE_ADD_STR)
	{
		gMsgQueue.Enqueue((char*)originalData, msgHeadInfo.strLength * 2);
	}



	WaitForMultipleObjects(THREAD_MAX, thread, TRUE, INFINITE);

	return 0;
}
