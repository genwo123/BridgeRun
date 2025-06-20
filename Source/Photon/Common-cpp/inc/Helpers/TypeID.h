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
			template<typename T>
			struct TypeID
			{
				static size_t get(void);
			};



			template<typename T>
			size_t TypeID<T>::get(void)
			{
				return reinterpret_cast<size_t>(&get);
			}
		}
	}
}