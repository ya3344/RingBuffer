#pragma once
class RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();

public:
	enum QUEUE_DATA_INDEX
	{
		MAX_BUFFER_SIZE = 50,
	};
	enum ERROR_INDEX
	{
		USE_COUNT_NORMAL = 0,
		USE_COUNT_OVER_FLOW = -1,
		USE_COUNT_UNDER_FLOW = -2,
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
	int MoveReadPos(const int readSize);
	int MoveWritePos(const int writeSize);
	char* GetBufferPtr(void) const { return mBuffer + mWritePos; }




private:
	int mReadPos = 0;
	int mWritePos = 0;
	int mUseCount = 0;
	char* mBuffer = nullptr;
};

