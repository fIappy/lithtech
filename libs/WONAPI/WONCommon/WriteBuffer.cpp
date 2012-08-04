#include "WriteBuffer.h"
#include "LittleEndian.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::Reset()
{
	delete [] mData;

	mData = NULL;
	mDataLen = 0;
	mCapacity = 0;

	if(mLengthFieldSize>0)
		SkipBytes(mLengthFieldSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::Release()
{
	mData = NULL;
	Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WriteBuffer::WriteBuffer(unsigned char theLengthFieldSize, bool lengthIncludesLengthFieldSize) 
{
	mData = NULL;
	mLengthFieldSize = theLengthFieldSize;
	mLengthIncludesLengthFieldSize = lengthIncludesLengthFieldSize;

	Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WriteBuffer::~WriteBuffer()
{
	delete [] mData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::CheckWrite(unsigned long theWriteLen)
{
	int aTotalLen = mDataLen + theWriteLen;
	if(aTotalLen>mCapacity)
	{
		int aSize = mCapacity;
		if(aSize<32)
			aSize = 32;

		while(aSize<aTotalLen)
			aSize<<=1;

		Reserve(aSize);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::Reserve(unsigned long theNumBytes)
{
	if(mCapacity<theNumBytes)
	{
		char *newBuffer = new char[theNumBytes];
		memcpy(newBuffer,mData,mDataLen);
		delete [] mData;
		mData = newBuffer;
		mCapacity = theNumBytes;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::SetSize(unsigned long theNumBytes)
{
	Reserve(theNumBytes);
	mDataLen = theNumBytes;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::SkipBytes(unsigned long theLen)
{
	CheckWrite(theLen);
	mDataLen += theLen;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendBytes(const void *theBytes, unsigned long theLen)
{
	CheckWrite(theLen);
	memcpy(mData + mDataLen, theBytes, theLen);
	mDataLen += theLen;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendByte(char theByte)
{
	AppendBytes(&theByte,1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void WriteBuffer::AppendBool(bool theBool)
{
	unsigned char aVal = theBool?1:0;
	AppendBytes(&aVal,1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendShort(short theShort)
{
	AppendBytes(&(theShort=ShortToLittleEndian(theShort)),2);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendLong(long theLong)
{
	AppendBytes(&(theLong=LongToLittleEndian(theLong)),4);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendLength(unsigned long theLen, unsigned char theLengthFieldSize)
{
	switch(theLengthFieldSize)
	{
		case 1: AppendByte(theLen); break;
		case 2: AppendShort(theLen); break;
		case 4: AppendLong(theLen); break;
		default: break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendString(const std::string &theStr, unsigned char theLengthFieldSize)
{
	AppendLength(theStr.length(),theLengthFieldSize);
	AppendBytes(theStr.data(),theStr.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendWString(const std::wstring &theWStr, unsigned char theLengthFieldSize)
{
	AppendLength(theWStr.length(),theLengthFieldSize);

	if(IsLittleEndian() && sizeof(wchar_t)==2)
		AppendBytes(theWStr.data(),theWStr.length()*2);
	else
	{
		for(int i=0; i<theWStr.length(); i++)
			AppendShort(theWStr[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::AppendBuffer(const ByteBuffer* theBuffer, unsigned char theLengthFieldSize)
{
	unsigned long aLen = theBuffer?theBuffer->length():0;
	AppendLength(aLen,theLengthFieldSize);

	if(theBuffer!=NULL && aLen>0)
		AppendBytes(theBuffer->data(),aLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::SetBytes(unsigned long thePos, const void *theBytes, unsigned long theLen)
{
	memcpy(mData+thePos,theBytes,theLen);
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::SetByte(unsigned long thePos, char theByte)
{
	SetBytes(thePos,&theByte,1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::SetShort(unsigned long thePos, short theShort)
{
	SetBytes(thePos,&(theShort=ShortToLittleEndian(theShort)),2);
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void WriteBuffer::SetLong(unsigned long thePos, long theLong)
{
	SetBytes(thePos,&(theLong=LongToLittleEndian(theLong)),4);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr WriteBuffer::ToByteBuffer(bool release)
{
	unsigned long aLen = mDataLen;
	if(!mLengthIncludesLengthFieldSize)
		aLen -= mLengthFieldSize;

	switch(mLengthFieldSize)
	{
		case 1: *mData = aLen; break;
		case 2: (*(unsigned short*)mData) = ShortToLittleEndian(aLen); break;
		case 4: (*(unsigned long*)mData) = LongToLittleEndian(aLen); break;
		default: break;
	}
/*	if(mLengthFieldSize>0) // store length
		memcpy(mData,&mDataLen,mLengthFieldSize);*/

	char *data = mData;
	unsigned long datalen = mDataLen;
	
	if(release)
		Release();

	return new ByteBuffer(data,datalen,release);
}
