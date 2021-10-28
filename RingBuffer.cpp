#include "pch.h"
#include "RingBuffer.h"

/*
ReadPos ��ġ�� ä������������ �ְ� ����� ���� �� �ִ� �����̴�.(�а� ������ġ �̵�)
WritePos ��ġ�� ����� �ִ� �����̴�.(���� ���� �� �̵�)
*/
RingBuffer::RingBuffer()
{
	mBuffer = new char[MAX_BUFFER_SIZE];
	memset(mBuffer, 0, MAX_BUFFER_SIZE);
	InitializeSRWLock(&mSRW_EnqueueLock);
	InitializeSRWLock(&mSRW_DequeueLock);
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
		if ((mWritePos + 1) % MAX_BUFFER_SIZE == mReadPos)
		{
			return DATA_FULL_ERROR;
		}

		mBuffer[mWritePos] = *inputData;
		mWritePos = (mWritePos + 1) % MAX_BUFFER_SIZE;

		++count;
		++inputData;
		--dataSize;
	}
	InterlockedAdd(&mUseCount, count);
	//mUseCount += count;
	// ���� ó�� ����� Buffer �Ѱ�ġ �ʰ�
	if (mUseCount >= MAX_BUFFER_SIZE)
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

		readPos = (readPos + 1) % MAX_BUFFER_SIZE;

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

		readPos = (readPos + 1) % MAX_BUFFER_SIZE;

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
			//mUseCount = mUseCount - totalCount;
			if (mUseCount < 0)
				return USE_COUNT_UNDER_FLOW;

			return totalCount;
		}

		readPos = (readPos + 1) % MAX_BUFFER_SIZE;

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
	//mUseCount = mUseCount - totalCount;
	if (mUseCount < 0)
		return USE_COUNT_UNDER_FLOW;

	return totalCount;
}

int RingBuffer::GetFreeSize() const
{
	int freeSize = MAX_BUFFER_SIZE - mUseCount;

	freeSize -= 1; // 1byte�� ��� ���ϴ� ����

	return freeSize;
}

int RingBuffer::GetUseSize() const
{
	return mUseCount;
}

int RingBuffer::GetNotBroken_WriteSize() const
{
	if (mWritePos < mReadPos)
	{
		return mReadPos - mWritePos - 1;
	}
	else if (mWritePos == mReadPos)
	{
		return MAX_BUFFER_SIZE - 1;
	}
	else
	{
		return MAX_BUFFER_SIZE - mWritePos - 1;
	}
	if (mReadPos - mWritePos < 0)
	{
		return MAX_BUFFER_SIZE + (mReadPos - mWritePos);
	}
	else if (mReadPos - mWritePos > 0)
	{

	}
	
}

int RingBuffer::MoveReadPos(const int readSize)
{
	mReadPos += readSize;
	mReadPos %= MAX_BUFFER_SIZE;
	mUseCount -= readSize;

	// ����ó��
	if (mUseCount < 0)
		return USE_COUNT_UNDER_FLOW;

	return USE_COUNT_NORMAL;
}

int RingBuffer::MoveWritePos(const int writeSize)
{
	mWritePos += writeSize;
	mWritePos %= MAX_BUFFER_SIZE;
	mUseCount += writeSize;

	// ����ó��
	if (mUseCount >= MAX_BUFFER_SIZE - 1)
		return USE_COUNT_OVER_FLOW;

	return USE_COUNT_NORMAL;
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
		if ((mWritePos + 1) % MAX_BUFFER_SIZE == mReadPos)
		{
			ReleaseSRWLockExclusive(&mSRW_EnqueueLock);
			mWritePos -= count;
			return DATA_FULL_ERROR;
		}

		mBuffer[mWritePos] = *inputData;
		mWritePos = (mWritePos + 1) % MAX_BUFFER_SIZE;

		++count;
		++inputData;
		--dataSize;
	}
	// ���ۿ� ���� count ��ŭ UseCount ����
	InterlockedAdd(&mUseCount, count);
	ReleaseSRWLockExclusive(&mSRW_EnqueueLock);

	// ���� ó�� ����� Buffer �Ѱ�ġ �ʰ�
	if (mUseCount >= MAX_BUFFER_SIZE)
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
	readPos = mReadPos;

	if (mUseCount < dataSize || dataSize == 0)
	{
		ReleaseSRWLockExclusive(&mSRW_DequeueLock);
		return DATA_LACK;
	}
	//readPos = mReadPos;

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

		readPos = (readPos + 1) % MAX_BUFFER_SIZE;

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
