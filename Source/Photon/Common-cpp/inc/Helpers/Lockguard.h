/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/Spinlock.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			class Lockguard
			{
			public:
				Lockguard(Spinlock& lock);
				~Lockguard(void);
			private:
				Lockguard(const Lockguard& toCopy);
				Lockguard& operator=(const Lockguard& toCopy);

				Spinlock& mLock;
			};
		}
	}
}