/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/TypeTraits/IsScalar.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Ctype> struct IsObject{static const bool is = IsScalar<Ctype>::is || IsArray<Ctype>::is || IsUnion<Ctype>::is || IsClass<Ctype>::is;};

			/** @file */
		}
	}
}