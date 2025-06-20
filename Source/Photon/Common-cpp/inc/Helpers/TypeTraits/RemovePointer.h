/* Exit Games Common - C++ Client Lib
* Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
* https://www.photonengine.com
* mailto:developer@photonengine.com
*/

#pragma once

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename T> struct RemovePointer
			{
				typedef T type;
			};

			template<typename T> struct RemovePointer<T*>
			{
				typedef T type;
			};

			template<typename T> struct RemovePointer<T const*>
			{
				typedef T const type;
			};

			template<typename T> struct RemovePointer<T* const>
			{
				typedef T const type;
			};
		}
	}
}