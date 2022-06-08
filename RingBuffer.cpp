#include "pch.h"
#include "RingBuffer.h"

/*
ReadPos ��ġ�� ä������������ �ְ� ����� ���� �� �ִ� �����̴�.(�а� ������ġ �̵�)
WritePos ��ġ�� ����� �ִ� �����̴�.(���� ���� �� �̵�)
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
		// ������ �� ��
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

	// ���� ó�� ����� Buffer �Ѱ�ġ �ʰ�
	if (mUseCount >= mMaxBufferSize)
		return USE_COUNT_OVER_FLOW;

	return count;
}

/*
* Peek üũ �� ��ť ����.(������ �Լ�)
* Peek �Լ��� �� ȣ��Ǿ�� ��.
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
		//������ ��� ����
		if (readPos == mWritePos)
		{
			//readPos�� �迭 ������ �������� �Ѿ���
			if (rewindCount > 0)
			{
				memcpy(compareData, mBuffer + mReadPos, count);
				memcpy(compareData + count, mBuffer, rewindCount);
			}
			else
				memcpy(compareData, mBuffer + mReadPos, count);

			// Peek�Լ� ȣ�� �� Output ������� �� 
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

	// Peek�Լ� ȣ�� �� Output ������� �� 
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
		//������ ��� ����
		if (readPos == mWritePos)
		{
			//readPos�� �迭 ������ �������� �Ѿ���
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

	//readPos�� �迭 ������ �������� �Ѿ���
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
		//������ ��� ����
		if (readPos == mWritePos)
		{
			//readPos�� �迭 ������ �������� �Ѿ���
			if (rewindCount > 0)
			{
				memcpy(outputData, mBuffer + mReadPos, count);
				memcpy(outputData + count, mBuffer, rewindCount);
			}
			else
				memcpy(outputData, mBuffer + mReadPos, count);

			//���� ������ �Ѱ踦 ���Ѵ�.
			totalCount = count + rewindCount;

			//���� ������ ��ġ ����
			mReadPos = readPos;

			//���� ��ŭ UseCount ����
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

	//readPos�� �迭 ������ �������� �Ѿ���
	if (rewindCount > 0)
	{
		memcpy(outputData, mBuffer + mReadPos, count);
		memcpy(outputData + count, mBuffer, rewindCount);
	}
	else
		memcpy(outputData, mBuffer + mReadPos, count);

	//���� ������ �Ѱ踦 ���Ѵ�.
	totalCount = count + rewindCount;

	//���� ������ ��ġ ����
	mReadPos = readPos;

	//���� ��ŭ UseCount ����
	InterlockedAdd(&mUseCount, -totalCount);

	if (mUseCount < 0)
		return USE_COUNT_UNDER_FLOW;

	return totalCount;
}

int RingBuffer::GetFreeSize() const
{
	int freeSize = mMaxBufferSize - mUseCount;

	freeSize -= 1; // 1byte�� ��� ���ϴ� ����

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

	// ����ó��
	if (mUseCount < 0)
		return USE_COUNT_UNDER_FLOW;

	return USE_COUNT_NORMAL;
}

int RingBuffer::MoveWritePos(const int writeSize)
{
	mWritePos += writeSize;
	mWritePos %= mMaxBufferSize;
	mUseCount += writeSize;

	// ����ó��
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
		// ������ �� ��
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
	// ���ۿ� ���� count ��ŭ UseCount ����
	InterlockedAdd(&mUseCount, count);
	
	// ���� ó�� ����� Buffer �Ѱ�ġ �ʰ�
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
		//������ ��� ����
		if (readPos == mWritePos)
		{
			//readPos�� �迭 ������ �������� �Ѿ���
			if (rewindCount > 0)
			{
				memcpy(outputData, mBuffer + mReadPos, count);
				memcpy(outputData + count, mBuffer, rewindCount);
			}
			else
				memcpy(outputData, mBuffer + mReadPos, count);

			//���� ������ �Ѱ踦 ���Ѵ�.
			totalCount = count + rewindCount;

			//���� ������ ��ġ ����
			mReadPos = readPos;

			//���� ��ŭ UseCount ����
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

	//readPos�� �迭 ������ �������� �Ѿ���
	if (rewindCount > 0)
	{
		memcpy(outputData, mBuffer + mReadPos, count);
		memcpy(outputData + count, mBuffer, rewindCount);
	}
	else
		memcpy(outputData, mBuffer + mReadPos, count);

	//���� ������ �Ѱ踦 ���Ѵ�.
	totalCount = count + rewindCount;

	//���� ������ ��ġ ����
	mReadPos = readPos;

	//���� ��ŭ UseCount ����
	InterlockedAdd(&mUseCount, -totalCount);

	ReleaseSRWLockExclusive(&mSRW_DequeueLock);

	if (mUseCount < 0)
	{
		ReleaseSRWLockExclusive(&mSRW_DequeueLock);
		return USE_COUNT_UNDER_FLOW;
	}

	return totalCount;

	
}
