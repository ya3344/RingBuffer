#include "pch.h"
#include "RingBuffer.h"

int RingBuffer::Enqueue(const char* inputData, int dataSize)
{
	int count = 0;

	while (dataSize != 0)
	{
		// ������ �� ��
		if ((mWritePos + 1) % MAX_BUFFER_SIZE == mReadPos)
		{
			return count;
		}
		mData[mWritePos] = *inputData;
		mWritePos = (mWritePos + 1) % MAX_BUFFER_SIZE;

		++count;
		++inputData;
		--dataSize;
		++mUseCount;

		// ���� ó�� ����� Buffer �Ѱ�ġ �ʰ�
		if (mUseCount >= MAX_BUFFER_SIZE)
			return USE_COUNT_OVER_FLOW;
	}

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
				memcpy(compareData, mData + mReadPos, count);
				memcpy(compareData + count, mData, rewindCount);
			}
			else
				memcpy(compareData, mData + mReadPos, count);

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
		memcpy(compareData, mData + mReadPos, count);
		memcpy(compareData + count, mData, rewindCount);
	}
	else
		memcpy(compareData, mData + mReadPos, count);

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

	while (dataSize != 0)
	{
		//������ ��� ����
		if (readPos == mWritePos)
		{
			//readPos�� �迭 ������ �������� �Ѿ���
			if (rewindCount > 0)
			{
				memcpy(outputData, mData + mReadPos, count);
				memcpy(outputData + count, mData, rewindCount);
			}
			else
				memcpy(outputData, mData + mReadPos, count);

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
		memcpy(outputData, mData + mReadPos, count);
		memcpy(outputData + count, mData, rewindCount);
	}
	else
		memcpy(outputData, mData + mReadPos, count);

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

	while (dataSize != 0)
	{
		//������ ��� ����
		if (readPos == mWritePos)
		{
			//readPos�� �迭 ������ �������� �Ѿ���
			if (rewindCount > 0)
			{
				memcpy(outputData, mData + mReadPos, count);
				memcpy(outputData + count, mData, rewindCount);
			}
			else
				memcpy(outputData, mData + mReadPos, count);

			//���� ������ �Ѱ踦 ���Ѵ�.
			totalCount = count + rewindCount;

			//���� ������ ��ġ ����
			mReadPos = readPos;

			//���� ��ŭ UseCount ����
			mUseCount = mUseCount - totalCount;
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
		memcpy(outputData, mData + mReadPos, count);
		memcpy(outputData + count, mData, rewindCount);
	}
	else
		memcpy(outputData, mData + mReadPos, count);

	//���� ������ �Ѱ踦 ���Ѵ�.
	totalCount = count + rewindCount;

	//���� ������ ��ġ ����
	mReadPos = readPos;

	//���� ��ŭ UseCount ����
	mUseCount = mUseCount - totalCount;
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

bool RingBuffer::MoveReadPos(const int readSize)
{
	mReadPos += readSize;
	mReadPos %= MAX_BUFFER_SIZE;
	mUseCount -= readSize;

	// ����ó��
	if (mUseCount < 0)
		return false;

	return true;
}
