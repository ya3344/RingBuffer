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

#pragma comment (lib, "winmm")

enum THRAD_INDEX
{
	WORK_THREAD = 0,
	WORK_THREAD_1,
	WORK_THREAD_2,
	WORK_THREAD_3,
	//WORK_THREAD_4,
	//WORK_THREAD_5,
	THREAD_MAX
};

enum MSG_TYPE_INDEX
{
	MSG_TYPE_ADD_STR = 0,
	MSG_TYPE_DEL_STR,
	MSG_TYPE_PRINT_LIST,
	MSG_TYPE_QUIT,
};

enum SYSTEM_INDEX
{
	MSG_MAX_SIZE = 30,
	STRING_MAX_SIZE = 10,
	MAIN_WAIT_TIME = 1,
};

struct MsgHeadInfo
{
	short type = 0;
	short strLength = 0;
};

bool gShutdown = false;
HANDLE gMainAutoEvent = NULL;
HANDLE gWorkMenualEvent = NULL;
HANDLE thread[THREAD_MAX];
RingBuffer gMsgQueue;
list<wstring> gList;
SRWLOCK gSRW_Lock;
long gTPS_Conut = 0;

unsigned __stdcall WorkThread(void* arguments)
{
	MsgHeadInfo msgHeadInfo;
	char outputData[sizeof(MsgHeadInfo)] = { 0 };
	WCHAR stringData[STRING_MAX_SIZE + 1] = { 0 };
	int dequeueRet = 0;

	while (true)
	{
		WaitForSingleObject(gWorkMenualEvent, INFINITE);

		//printf("Start threadID:%d\n", GetCurrentThreadId());
		AcquireSRWLockExclusive(&gSRW_Lock);

		dequeueRet = gMsgQueue.Dequeue(outputData, sizeof(msgHeadInfo));
		if (dequeueRet <= 0)
		{
			//printf("dequeueRet 0 threadID:%d\n", GetCurrentThreadId());
			ResetEvent(gWorkMenualEvent);
			ReleaseSRWLockExclusive(&gSRW_Lock);
			continue;
		}

		msgHeadInfo = *(MsgHeadInfo*)outputData;
		/*printf("outputData threadID:%d type:%d length:%d use Size:%d\n", GetCurrentThreadId(), msgHeadInfo.type,
			msgHeadInfo.strLength, gMsgQueue.GetUseSize());*/

		if (msgHeadInfo.type == MSG_TYPE_ADD_STR)
		{
			memset(stringData, 0, sizeof(stringData));
			dequeueRet = gMsgQueue.Dequeue((char*)stringData, msgHeadInfo.strLength);
		}
	/*	else if (msgHeadInfo.type == MSG_TYPE_QUIT)
		{
			wprintf(L"MSG_TYPE_QUIT\n");
			SetEvent(gWorkMenualEvent);
			ReleaseSRWLockExclusive(&gSRW_Lock);
			return 0;
		}*/
			

		ReleaseSRWLockExclusive(&gSRW_Lock);
		
		switch (msgHeadInfo.type)
		{
		case MSG_TYPE_ADD_STR:
			{
				//memset(stringData, 0, sizeof(stringData));
				//dequeueRet = gMsgQueue.LockDequeue((char*)stringData, msgHeadInfo.strLength);
				//wprintf(L"stringData threadID:%d stringData:%s\n", GetCurrentThreadId(), stringData);

				/*if (dequeueRet <= 0)
				{
					_ASSERT(false);
				}*/
				AcquireSRWLockExclusive(&gSRW_Lock);
				gList.emplace_back(stringData);
				ReleaseSRWLockExclusive(&gSRW_Lock);
			}
			break;
		case MSG_TYPE_DEL_STR:
			{
				if (gList.size() > 0)
				{
					AcquireSRWLockExclusive(&gSRW_Lock);
					gList.pop_front();
					ReleaseSRWLockExclusive(&gSRW_Lock);
				}
			}
			break;
		case MSG_TYPE_PRINT_LIST:
			{
				AcquireSRWLockShared(&gSRW_Lock);
				if (gList.size() <= 0)	// List Size가 0보다
				{
					ReleaseSRWLockShared(&gSRW_Lock);	
					ResetEvent(gWorkMenualEvent);
					continue;
				}
				
				wprintf(L"List:");
				//AcquireSRWLockExclusive(&gSRW_Lock);
				for (wstring string : gList)
				{
					wprintf(L" [%s]", string.c_str());
				}
				wprintf(L" List Size:%d Thread ID:%d\n", (int)gList.size(), GetCurrentThreadId());
				ReleaseSRWLockShared(&gSRW_Lock);
				//ReleaseSRWLockExclusive(&gSRW_Lock);
			}
			break;
		case MSG_TYPE_QUIT:
			{
				SetEvent(gWorkMenualEvent);
				wprintf(L"MSG_TYPE_QUIT\n");
				return 0;
			}
			break;
		default:
			_ASSERT(false);
			return 0;
		}
		InterlockedIncrement(&gTPS_Conut); // tps 카운트
		//ReleaseSRWLockExclusive(&gSRW_Lock);

		ResetEvent(gWorkMenualEvent);
	}

	return 0;
}

int main()
{
	unsigned int threadID[THREAD_MAX];
	random_device rd;
	mt19937 mtRand(rd());
	WCHAR originalData[] = { L"1234567890" };
	static DWORD msgCount = 0;
	uniform_int_distribution<DWORD> lengthRange(1, lstrlen(originalData));  // 범위안의 난수발생
	uniform_int_distribution<DWORD> typeRange(MSG_TYPE_ADD_STR, MSG_TYPE_PRINT_LIST);  // 범위안의 난수발생
	MsgHeadInfo msgHeadInfo;
	WORD waitTimeMS = 1000 / MAIN_WAIT_TIME;

	timeBeginPeriod(1);

	gMainAutoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	gWorkMenualEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeSRWLock(&gSRW_Lock);

	thread[WORK_THREAD] = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &threadID[WORK_THREAD]);
	thread[WORK_THREAD_1] = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &threadID[WORK_THREAD_1]);
	thread[WORK_THREAD_2] = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &threadID[WORK_THREAD_2]);
	thread[WORK_THREAD_3] = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &threadID[WORK_THREAD_3]);
	//thread[WORK_THREAD_4] = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &threadID[WORK_THREAD_4]);
	//thread[WORK_THREAD_5] = (HANDLE)_beginthreadex(NULL, 0, &WorkThread, NULL, 0, &threadID[WORK_THREAD_5]);



	while (true)
	{
		// 스페이스 키 입력 시 종료 처리 진행
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			msgHeadInfo.type = MSG_TYPE_QUIT;
			for (int i = 0; i < THREAD_MAX; ++i)
			{
				gMsgQueue.Enqueue((char*)&msgHeadInfo, sizeof(msgHeadInfo));
			}
			AcquireSRWLockShared(&gSRW_Lock);
			wprintf(L"MsgQ FreeSize:%d MsgQ UseSize:%d TPS:%d\n", gMsgQueue.GetFreeSize(), gMsgQueue.GetUseSize(), gTPS_Conut);
			ReleaseSRWLockShared(&gSRW_Lock);
			SetEvent(gWorkMenualEvent);
			break;
		}

		WaitForSingleObject(gMainAutoEvent, MAIN_WAIT_TIME);
		++msgCount;
		if (msgCount % waitTimeMS == 0) // 1초마다 출력
		{
			AcquireSRWLockShared(&gSRW_Lock);
			wprintf(L"MsgQ FreeSize:%d MsgQ UseSize:%d TPS:%d\n", gMsgQueue.GetFreeSize(), gMsgQueue.GetUseSize(), gTPS_Conut);
			ReleaseSRWLockShared(&gSRW_Lock);
			InterlockedExchange(&gTPS_Conut, 0);
		}

		msgHeadInfo.type = (short)typeRange(mtRand)/*MSG_TYPE_ADD_STR*/;
		if (msgHeadInfo.type == MSG_TYPE_ADD_STR)
		{
			msgHeadInfo.strLength = (short)(lengthRange(mtRand) * 2);
		}
		
		gMsgQueue.Enqueue((char*)&msgHeadInfo, sizeof(msgHeadInfo));
		if (msgHeadInfo.type == MSG_TYPE_ADD_STR)
		{
			gMsgQueue.Enqueue((char*)originalData, msgHeadInfo.strLength);
		}
		SetEvent(gWorkMenualEvent);
	}
	WaitForMultipleObjects(THREAD_MAX, thread, TRUE, INFINITE);


	timeEndPeriod(1);
	return 0;
}
