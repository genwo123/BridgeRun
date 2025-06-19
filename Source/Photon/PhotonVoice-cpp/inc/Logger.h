/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Common.h"
#include "Common-cpp/inc/Helpers/LockableNonAssignable.h"

namespace ExitGames
{
	namespace Voice
	{
		class Logger : public Common::Logger
		{
		public:
			inline virtual void log(int debugLevel, const EG_CHAR* file, const EG_CHAR* function, bool printBrackets, unsigned int line, const EG_CHAR* dbgMsg, ...) const
			{
#ifdef EG_LOGGING
				std::lock_guard<std::mutex> lock(mutex);
				va_list args;
				va_start(args, dbgMsg);
				vlog(debugLevel, file, function, printBrackets, line, dbgMsg, args);
				va_end(args);
#endif
			}
		private:
			mutable std::mutex mutex;
		};
	}
}
