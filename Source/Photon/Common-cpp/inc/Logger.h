/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/BaseListener.h"
#include "Common-cpp/inc/LogFormatOptions.h"

namespace ExitGames
{
	namespace Common
	{
		class Logger : public Common::ToString
		{
		public:
			using ToString::toString;

			Logger(int debugLevel=DebugLevel::WARNINGS);
			virtual ~Logger(void);

			bool operator==(const Logger& toCompare) const;
			bool operator!=(const Logger& toCompare) const;

			virtual void log(int debugLevel, const EG_CHAR* file, const EG_CHAR* function, bool printBrackets, unsigned int line, const EG_CHAR* dbgMsg, ...) const;
			void vlog(int debugLevel, const EG_CHAR* file, const EG_CHAR* function, bool printBrackets, unsigned int line, const EG_CHAR* dbgMsg, va_list args) const;

			int getDebugOutputLevel(void) const;
			bool setDebugOutputLevel(int debugLevel);
			void setListener(const BaseListener& listener);
			const LogFormatOptions& getFormatOptions(void) const;
			void setFormatOptions(const LogFormatOptions& formatOptions);

			virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
		private:
			Logger(const Logger& notCopyable);
			Logger& operator=(const Logger& notCopyable);

			static JString padString(JString str, unsigned int padding);

			int mDebugLevel;
#ifdef EG_LOGGING
			BaseListener* mpListener;
#	ifdef _EG_NINTENDO_WINDOWS_PLATFORM
			mutable bool mInitialized;
#	endif
#endif
			LogFormatOptions mOptions;
		};
	}
}

// public logging API
#ifdef EG_LOGGING
#	define EGLOG(debugLevel, ...) ((debugLevel)<=mLogger.getDebugOutputLevel()?mLogger.log((debugLevel), __WFILE__, __WFUNCTION__, true, __LINE__, __VA_ARGS__):((void)0))
#else
#	define EGLOG(debugLevel, ...) ((void)0)
#endif