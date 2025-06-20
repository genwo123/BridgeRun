/* Exit Games Common - C++ Client Lib
* Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
* https://www.photonengine.com
* mailto:developer@photonengine.com
*/

#pragma once

#include "Common-cpp/inc/Helpers/TypeName.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"

namespace ExitGames
{
	namespace Common
	{
		template<typename T, nByte typeCode> class CustomType;

		namespace Helpers
		{
			template<typename T, nByte typeCode> class CustomTypeFactory : public Base
			{
			public:
				using ToString::toString;

				virtual ~CustomTypeFactory(void) {};

				virtual CustomType<T, typeCode>* create(unsigned int amount) const;
				virtual CustomType<T, typeCode>* copy(const CustomType<T, typeCode>* pToCopy, unsigned int amount) const;
				virtual void destroy(const CustomType<T, typeCode>* pToDestroy) const;
				virtual unsigned int sizeOf(void) const;

				virtual JString& toString(JString& retStr, bool withTypes=false) const;
			};



			template<typename T, nByte typeCode>
			CustomType<T, typeCode>* CustomTypeFactory<T, typeCode>::create(unsigned int amount) const
			{
				return MemoryManagement::allocateArray<T>(amount);
			}

			template<typename T, nByte typeCode>
			CustomType<T, typeCode>* CustomTypeFactory<T, typeCode>::copy(const CustomType<T, typeCode>* pToCopy, unsigned int amount) const
			{
				T* pRetVal = static_cast<T*>(create(amount));
				for(unsigned int  i=0; i<amount; ++i)
					pRetVal[i] = static_cast<const T*>(pToCopy)[i];
				return pRetVal;
			}

			template<typename T, nByte typeCode>
			void CustomTypeFactory<T, typeCode>::destroy(const CustomType<T, typeCode>* pToDestroy) const
			{
				MemoryManagement::deallocateArray(static_cast<const T*>(pToDestroy));
			}

			template<typename T, nByte typeCode>
			unsigned int CustomTypeFactory<T, typeCode>::sizeOf(void) const
			{
				return sizeof(T);
			}

			template<typename T, nByte typeCode>
			JString& CustomTypeFactory<T, typeCode>::toString(JString& retStr, bool /*withTypes*/) const
			{
				return retStr += L"{" + Helpers::TypeName::get(this) + L"}";
			}
		}
	}
}