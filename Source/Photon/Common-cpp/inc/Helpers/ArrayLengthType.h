/* Exit Games Common - C++ Client Lib
* Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
* https://www.photonengine.com
* mailto:developer@photonengine.com
*/

#pragma once

#include "Common-cpp/inc/defines.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype> struct ArrayLengthType{typedef short type;};
			template<> struct ArrayLengthType<nByte*>{typedef int type;};
			template<> struct ArrayLengthType<const nByte*>{typedef int type;};
		}
	}
}