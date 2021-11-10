#pragma once
class RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();

public:
	enum QUEUE_DATA_INDEX
	{
		MAX_BUFFER_SIZE = 10000,
	};
	enum ERROR_INDEX
	{
		USE_COUNT_NORMAL = 0,
		USE_COUNT_OVER_FLOW = -1,
		USE_COUNT_UNDER_FLOW = -2,
		DATA_FULL = -3,
		DATA_FULL_ERROR = -4,
		DATA_LACK = -5,
	};

public:
	int Enqueue(const char* inputData, int dataSize);
	int ConfirmDequeue(char* outputData, int dataSize);
	int Peek(char* outputData, int dataSize);
	int Dequeue(char* outputData, int dataSize);
	int GetFreeSize() const;
	int GetUseSize() const;
	int GetReadSize() const { return mReadPos; }
	int GetWriteSize() const { return mWritePos; }
	int GetNotBroken_WriteSize() const;
	int GetBroken_WriteSize() const;
	int MoveReadPos(const int readSize);
	int MoveWritePos(const int writeSize);
	char* GetNotBroken_BufferPtr(void) const { return mBuffer + mWritePos; }
	char* GetBroken_BufferPtr(void);
// Lock 관련 함수
public:
	int LockEnqueue(const char* inputData, int dataSize);
	int LockDequeue(char* outputData, int dataSize);

private:
	int mReadPos = 0;
	int mWritePos = 0;
	long mUseCount = 0;
	char* mBuffer = nullptr;

// Lock 관련 변수(SRW Lock)
private:
	SRWLOCK mSRW_EnqueueLock;
	SRWLOCK mSRW_DequeueLock;
};

