#pragma once
class RingBuffer
{
public:
	RingBuffer() = default;
	~RingBuffer() = default;

public:
	enum QUEUE_DATA_INDEX
	{
		MAX_BUFFER_SIZE = 100,
	};
	enum ERROR_INDEX
	{
		NORMAL = 0,
		USE_COUNT_OVER_FLOW = -1,
		USE_COUNT_UNDER_FLOW = 2,
	};

private:
	int mReadPos = 0;
	int mWritePos = 0;
	int mUseCount = 0;
	char mData[MAX_BUFFER_SIZE] = { 0, };

public:
	int Enqueue(const char* inputData, int dataSize);
	int ConfirmDequeue(char* outputData, int dataSize);
	int Peek(char* outputData, int dataSize);
	int Dequeue(char* outputData, int dataSize);
	int GetFreeSize() const;
	int GetUseSize() const;
	bool MoveReadPos(const int readSize);
};

