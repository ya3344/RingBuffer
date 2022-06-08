#include "pch.h"
#include "RingBuffer.h"

/*
ReadPos 위치는 채워져있을수도 있고 비워져 있을 수 있는 상태이다.(읽고 다음위치 이동)
WritePos 위치는 비어져 있는 상태이다.(쓰기 진행 후 이동)
*/
RingBuffer::RingBuffer(const long bufferSize)
{
	mBuffer = new char[bufferSize];
	memset(mBuffer, 0, bufferSize);
	InitializeSRWLock(&mSRW_EnqueueLock);
	InitializeSRWLock(&mSRW_DequeueLock);
	mMaxBufferSize = bufferSize;
}

RingBuffer::~RingBuffer()
{
	if (mBuffer)
	{
		delete[] mBuffer;
		mBuffer = nullptr;
	}
}

int RingBuffer::Enqueue(const char* inputData, int dataSize)
{
	int count = 0;
	if (GetFreeSize() < dataSize)
		return DATA_FULL;

	while (dataSize != 0)
	{
		// 데이터 꽉 참
		if ((mWritePos + 1) % mMaxBufferSize == mReadPos)
		{
			return DATA_FULL_ERROR;
		}

		mBuffer[mWritePos] = *inputData;
		mWritePos = (mWritePos + 1) % mMaxBufferSize;

		++count;
		++inputData;
		--dataSize;
	}
	InterlockedAdd(&mUseCount, count);

	// 에러 처리 사용한 Buffer 한계치 초과
	if (mUseCount >= mMaxBufferSize)
		return USE_COUNT_OVER_FLOW;

	return count;
}

/*
* Peek 체크 후 디큐 진행.(검증용 함수)
* Peek 함수가 선 호출되어야 함.
*/
int RingBuffer::ConfirmDequeue(char* outputData, int dataSize)
{
	int count = 0;
	int readPos = mReadPos;
	bool isPosInit = false;
	int rewindCount = 0;
	char compareData[100] = { 0, };
	int totalCount = 0;

	while (dataSize != 0)
	{
		//데이터 비어 있음
		if (readPos == mWritePos)
		{
			//readPos가 배열 끝에서 시작으로 넘어갈경우
			if (rewindCount > 0)
			{
				memcpy(compareData, mBuffer + mReadPos, count);
				memcpy(compareData + count, mBuffer, rewindCount);
			}
			else
				memcpy(compareData, mBuffer + mReadPos, count);

			// Peek함수 호출 후 Output 결과값과 비교 
			if (memcmp(compareData, outputData, strlen(outputData)) == 0)
			{
				mReadPos = readPos;
				totalCount = count + rewindCount;
				return totalCount;
			}
			else
			{
				return -1;
			}
		}

		readPos = (readPos + 1) % mMaxBufferSize;

		if (isPosInit == true)
			++rewindCount;
		else
			++count;

		if (readPos == 0)
			isPosInit = true;

		--dataSize;
	}

	if (rewindCount > 0)
	{
		memcpy(compareData, mBuffer + mReadPos, count);
		memcpy(compareData + count, mBuffer, rewindCount);
	}
	else
		memcpy(compareData, mBuffer + mReadPos, count);

	// Peek함수 호출 후 Output 결과값과 비교 
	if (memcmp(compareData, outputData, strlen(outputData)) == 0)
	{
		mReadPos = readPos;
		totalCount = count + rewindCount;
		return totalCount;
	}
	else
	{
		return -1;
	}

	return 0;
}

int RingBuffer::Peek(char* outputData, int dataSize)
{
	int count = 0;
	int readPos = mReadPos;
	bool isPosInit = false;
	int rewindCount = 0;
	int totalCount = 0;

	if (mUseCount < dataSize)
		return DATA_LACK;

	while (dataSize != 0)
	{
		//데이터 비어 있음
		if (readPos == mWritePos)
		{
			//readPos가 배열 끝에서 시작으로 넘어갈경우
			if (rewindCount > 0)
			{
				memcpy(outputData, mBuffer + mReadPos, count);
				memcpy(outputData + count, mBuffer, rewindCount);
			}
			else
				memcpy(outputData, mBuffer + mReadPos, count);

			totalCount = count + rewindCount;

			return totalCount;
		}

		readPos = (readPos + 1) % mMaxBufferSize;

		if (isPosInit == true)
			++rewindCount;
		else
			++count;

		if (readPos == 0)
			isPosInit = true;

		--dataSize;
	}

	//readPos가 배열 끝에서 시작으로 넘어갈경우
	if (rewindCount > 0)
	{
		memcpy(outputData, mBuffer + mReadPos, count);
		memcpy(outputData + count, mBuffer, rewindCount);
	}
	else
		memcpy(outputData, mBuffer + mReadPos, count);

	totalCount = count + rewindCount;

	return totalCount;
}

int RingBuffer::Dequeue(char* outputData, int dataSize)
{
	int count = 0;
	int readPos = mReadPos;
	bool isPosInit = false;
	int rewindCount = 0;
	int totalCount = 0;

	if (mUseCount < dataSize)
		return DATA_LACK;

	while (dataSize != 0)
	{
		//데이터 비어 있음
		if (readPos == mWritePos)
		{
			//readPos가 배열 끝에서 시작으로 넘어갈경우
			if (rewindCount > 0)
			{
				memcpy(outputData, mBuffer + mReadPos, count);
				memcpy(outputData + count, mBuffer, rewindCount);
			}
			else
				memcpy(outputData, mBuffer + mReadPos, count);

			//읽은 데이터 총계를 더한다.
			totalCount = count + rewindCount;

			//읽은 데이터 위치 갱신
			mReadPos = readPos;

			//읽은 만큼 UseCount 감소
			InterlockedAdd(&mUseCount, -totalCount);

			if (mUseCount < 0)
				return USE_COUNT_UNDER_FLOW;

			return totalCount;
		}

		readPos = (readPos + 1) % mMaxBufferSize;

		if (isPosInit == true)
			++rewindCount;
		else
			++count;

		if (readPos == 0)
			isPosInit = true;

		--dataSize;
	}

	//readPos가 배열 끝에서 시작으로 넘어갈경우
	if (rewindCount > 0)
	{
		memcpy(outputData, mBuffer + mReadPos, count);
		memcpy(outputData + count, mBuffer, rewindCount);
	}
	else
		memcpy(outputData, mBuffer + mReadPos, count);

	//읽은 데이터 총계를 더한다.
	totalCount = count + rewindCount;

	//읽은 데이터 위치 갱신
	mReadPos = readPos;

	//읽은 만큼 UseCount 감소
	InterlockedAdd(&mUseCount, -totalCount);

	if (mUseCount < 0)
		return USE_COUNT_UNDER_FLOW;

	return totalCount;
}

int RingBuffer::GetFreeSize() const
{
	int freeSize = mMaxBufferSize - mUseCount;

	freeSize -= 1; // 1byte는 사용 못하는 공간

	return freeSize;
}

int RingBuffer::GetUseSize() const
{
	return mUseCount;
}

int RingBuffer::GetNotBroken_WriteSize() const
{
	int notBroken_writePos = 0;

	if (mWritePos < mReadPos)
	{
		notBroken_writePos = mReadPos - (mWritePos + 1);
	}
	else
	{
		notBroken_writePos = mMaxBufferSize - mWritePos;
		if (mReadPos == 0)
		{
			notBroken_writePos -= 1;
		}
	} 
	
	return notBroken_writePos;
}

int RingBuffer::GetBroken_WriteSize() const
{
	int writePos = 0;

	if (mWritePos < mReadPos)
	{
		return 0;
	}
	else
	{
		writePos = mReadPos - 1;

		if (writePos <= 0)
			writePos = 0;
	}

	return writePos;
}

int RingBuffer::MoveReadPos(const int readSize)
{
	mReadPos += readSize;
	mReadPos %= mMaxBufferSize;
	mUseCount -= readSize;

	// 에러처리
	if (mUseCount < 0)
		return USE_COUNT_UNDER_FLOW;

	return USE_COUNT_NORMAL;
}

int RingBuffer::MoveWritePos(const int writeSize)
{
	mWritePos += writeSize;
	mWritePos %= mMaxBufferSize;
	mUseCount += writeSize;

	// 에러처리
	if (mUseCount >= mMaxBufferSize - 1)
		return USE_COUNT_OVER_FLOW;

	return USE_COUNT_NORMAL;
}

char* RingBuffer::GetBroken_BufferPtr(void)
{
	return mBuffer + 0;
}

void RingBuffer::Clear()
{
	mReadPos = 0;
	mWritePos = 0;
	mUseCount = 0;
}

int RingBuffer::LockEnqueue(const char* inputData, int dataSize)
{
	int count = 0;

	AcquireSRWLockExclusive(&mSRW_EnqueueLock);
	if (GetFreeSize() < dataSize)
	{
		ReleaseSRWLockExclusive(&mSRW_EnqueueLock);
		return DATA_FULL;
	}
	
	while (dataSize != 0)
	{
		// 데이터 꽉 참
		if ((mWritePos + 1) % mMaxBufferSize == mReadPos)
		{
			mWritePos -= count;
			ReleaseSRWLockExclusive(&mSRW_EnqueueLock);
			return DATA_FULL_ERROR;
		}

		mBuffer[mWritePos] = *inputData;
		mWritePos = (mWritePos + 1) % mMaxBufferSize;

		++count;
		++inputData;
		--dataSize;
	}
	ReleaseSRWLockExclusive(&mSRW_EnqueueLock);
	// 버퍼에 담은 count 만큼 UseCount 증가
	InterlockedAdd(&mUseCount, count);
	
	// 에러 처리 사용한 Buffer 한계치 초과
	if (mUseCount >= mMaxBufferSize)
	{
		ReleaseSRWLockExclusive(&mSRW_EnqueueLock);
		return USE_COUNT_OVER_FLOW;
	}

	return count;
}

int RingBuffer::LockDequeue(char* outputData, int dataSize)
{
	int count = 0;
	int readPos = 0;
	bool isPosInit = false;
	int rewindCount = 0;
	int totalCount = 0;

	AcquireSRWLockExclusive(&mSRW_DequeueLock);

	if (mUseCount < dataSize || dataSize == 0)
	{
		ReleaseSRWLockExclusive(&mSRW_DequeueLock);
		return DATA_LACK;
	}
	readPos = mReadPos;

	while (dataSize != 0)
	{
		//데이터 비어 있음
		if (readPos == mWritePos)
		{
			//readPos가 배열 끝에서 시작으로 넘어갈경우
			if (rewindCount > 0)
			{
				memcpy(outputData, mBuffer + mReadPos, count);
				memcpy(outputData + count, mBuffer, rewindCount);
			}
			else
				memcpy(outputData, mBuffer + mReadPos, count);

			//읽은 데이터 총계를 더한다.
			totalCount = count + rewindCount;

			//읽은 데이터 위치 갱신
			mReadPos = readPos;

			//읽은 만큼 UseCount 감소
			InterlockedAdd(&mUseCount, -totalCount);

			if (mUseCount < 0)
			{
				ReleaseSRWLockExclusive(&mSRW_DequeueLock);
				return USE_COUNT_UNDER_FLOW;
			}

			ReleaseSRWLockExclusive(&mSRW_DequeueLock);
			return totalCount;
		}

		readPos = (readPos + 1) % mMaxBufferSize;

		if (isPosInit == true)
			++rewindCount;
		else
			++count;

		if (readPos == 0)
			isPosInit = true;

		--dataSize;
	}

	//readPos가 배열 끝에서 시작으로 넘어갈경우
	if (rewindCount > 0)
	{
		memcpy(outputData, mBuffer + mReadPos, count);
		memcpy(outputData + count, mBuffer, rewindCount);
	}
	else
		memcpy(outputData, mBuffer + mReadPos, count);

	//읽은 데이터 총계를 더한다.
	totalCount = count + rewindCount;

	//읽은 데이터 위치 갱신
	mReadPos = readPos;

	//읽은 만큼 UseCount 감소
	InterlockedAdd(&mUseCount, -totalCount);

	ReleaseSRWLockExclusive(&mSRW_DequeueLock);

	if (mUseCount < 0)
	{
		ReleaseSRWLockExclusive(&mSRW_DequeueLock);
		return USE_COUNT_UNDER_FLOW;
	}

	return totalCount;

	
}
