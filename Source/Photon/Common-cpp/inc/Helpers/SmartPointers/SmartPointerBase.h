/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/MemoryManagement/Allocate.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			class SmartPointerBase
			{
			protected:
				virtual ~SmartPointerBase(void) = 0;
			};
		}
	}
}