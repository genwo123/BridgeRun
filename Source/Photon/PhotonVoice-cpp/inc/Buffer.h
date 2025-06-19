/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Common.h"

namespace ExitGames
{
	namespace Voice
	{
		template<typename T>
		class Buffer : public Common::ToString
		{
		public:
			using ToString::toString;

			Buffer(void); // empty
			explicit Buffer(int size); // non-empty, even if the size is 0
			Buffer(const Buffer<T>& b, int offset, int size); // segment in b
			T* getArray(void);
			const T* getArray(void) const;
			int getSize(void) const;
			bool isEmpty(void) const;

			virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
		private:
			Common::Helpers::SharedPointer<T[]> mspArr;
			int mOffset;
			int mSize;
		};

		template<typename T>
		Buffer<T>::Buffer(void) // empty buffer: the pointer is NULL
			: mOffset(0)
			, mSize(0)
		{
		}

		template<typename T>
		Buffer<T>::Buffer(int size) // non-empty buffer: the pointer is always not NULL; to ensure this, allocate 1 element array for size = 0
			: mspArr(Common::MemoryManagement::allocateArray<T>(!size ? 1 : size))
			, mOffset(!size ? 1 : 0)
			, mSize(size)
		{
		}

		template<typename T>
		Buffer<T>::Buffer(const Buffer<T>& b, int offset, int size)
			: mspArr(b.mspArr)
			, mOffset(b.mOffset + offset)
			, mSize(size)
		{
		}

		template<typename T>
		T* Buffer<T>::getArray(void)
		{
			return mspArr + mOffset;
		}

		template<typename T>
		const T* Buffer<T>::getArray(void) const
		{
			return mspArr + mOffset;
		}

		template<typename T>
		int Buffer<T>::getSize(void) const
		{
			return mSize;
		}

		template<typename T>
		bool Buffer<T>::isEmpty(void) const
		{
			return &*mspArr == reinterpret_cast<const T*>(NULL);
		}

		template<typename T>
		Common::JString& Buffer<T>::toString(Common::JString& retStr, bool /*withTypes*/) const
		{
			return retStr += Common::Helpers::Stringifier<T>::toString(mspArr, getSize());
		}

		namespace FrameFlags
		{
			enum Enum : nByte
			{
				None = 0,
				Config = 1,
				KeyFrame = 2,
				PartNotBeg = 4,
				EndOfStream = 8,
				FragNotBeg = 16,
				FragNotEnd = 32,
				FEC = 64,
				PartNotEnd = 128,

				// 00 for unfragmented
				// 01 11 11 .. 11 10 for fragmented
				MaskFrag = FragNotBeg | FragNotEnd,

				MaskPart = PartNotBeg | PartNotEnd,
			};
		}

		class FrameBuffer : public Buffer<nByte> {
		public:
			using ToString::toString;

			FrameBuffer(void);
			FrameBuffer(int size, FrameFlags::Enum flags, nByte frNumber);
			FrameBuffer(const Buffer& b, int offset, int size, FrameFlags::Enum flags, nByte frNumber);
			FrameFlags::Enum getFlags(void) const;
			nByte getFrameNum(void) const;
			bool isFEC(void) const;
			bool isConfig(void) const;
			bool isKeyframe(void) const;

			virtual Common::JString& toString(Common::JString& retStr, bool withTypes = false) const;
		private:
			FrameFlags::Enum mFlags;
			nByte mFrameNum;
		};

		inline FrameBuffer::FrameBuffer(void)
			: Buffer()
			, mFlags(FrameFlags::None)
			, mFrameNum(0)
		{

		}

		inline FrameBuffer::FrameBuffer(int size, FrameFlags::Enum flags, nByte frNumber)
			: Buffer(size)
			, mFlags(flags)
			, mFrameNum(frNumber)
		{
		}

		inline FrameBuffer::FrameBuffer(const Buffer& b, int offset, int size, FrameFlags::Enum flags, nByte frNumber)
			: Buffer(b, offset, size)
			, mFlags(flags)
			, mFrameNum(frNumber)
		{
		}

		inline FrameFlags::Enum FrameBuffer::getFlags(void) const
		{
			return mFlags;
		}

		inline nByte FrameBuffer::getFrameNum(void) const
		{
			return mFrameNum;
		}

		inline bool FrameBuffer::isFEC(void) const
		{
			return (mFlags & FrameFlags::FEC) != 0;
		}

		inline bool FrameBuffer::isConfig(void) const
		{
			 return (mFlags & FrameFlags::Config) != 0;
		}

		inline bool FrameBuffer::isKeyframe(void) const
		{
			return (mFlags & FrameFlags::KeyFrame) != 0;
		}

		inline Common::JString& FrameBuffer::toString(Common::JString& retStr, bool /*withTypes*/) const
		{
			return retStr += Common::JString(L"fr#") + mFrameNum + " flags: " + static_cast<nByte>(mFlags) + L" size: " + getSize();
		}

	}
}