/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#if !defined _EG_LINUX_PLATFORM && !defined _EG_IPHONE_PLATFORM && !defined _EG_IMAC_PLATFORM && !defined _EG_WINDOWS_PLATFORM && !defined _EG_ANDROID_PLATFORM && !defined _EG_PS4_PLATFORM && !defined _EG_TVOS_PLATFORM && !defined _EG_WINDOWSSTORE_PLATFORM && !defined _EG_EMSCRIPTEN_PLATFORM && !defined _EG_XB1_PLATFORM && !defined _EG_SWITCH_PLATFORM && !defined _EG_GAMECORE_PLATFORM && !defined _EG_PS5_PLATFORM && !defined _EG_VISIONOS_PLATFORM && !defined _EG_SWITCH2_PLATFORM
#	include "Common-cpp/inc/platform_definition.h"
#endif

#if defined _EG_IPHONE_PLATFORM || defined _EG_IMAC_PLATFORM || defined _EG_TVOS_PLATFORM || defined _EG_VISIONOS_PLATFORM
#	define _EG_APPLE_PLATFORM true
#endif

#if defined _EG_IPHONE_PLATFORM
#	include <TargetConditionals.h>
	// attention: TARGET_OS_MACCATALYST is always defined on iOS: it is defined to 1 for catalyst and to 0 for iOS non-catalyst targets
#	if TARGET_OS_MACCATALYST
#		define _EG_IPHONE_MACCATALYST_PLATFORM
#	endif
#endif

#if defined _EG_SWITCH_PLATFORM
#	include <nn/TargetConfigs/build_Platform.h>
#	if defined NN_BUILD_TARGET_PLATFORM_NX
#		define _EG_SWITCH_NX_PLATFORM true
#	else
#		define _EG_SWITCH_WINDOWS_PLATFORM true
#	endif
#endif

#if defined _EG_SWITCH2_PLATFORM
#	include <nn/TargetConfigs/build_Platform.h>
#	if defined NN_BUILD_TARGET_PLATFORM_OUNCE
#		define _EG_SWITCH2_OUNCE_PLATFORM true
#	else
#		define _EG_SWITCH2_WINDOWS_PLATFORM true
#	endif
#endif

#if defined _EG_SWITCH_PLATFORM || defined _EG_SWITCH2_PLATFORM
#	define _EG_NINTENDO_PLATFORM true
#	if defined _EG_SWITCH_NX_PLATFORM || defined _EG_SWITCH2_OUNCE_PLATFORM
#		define _EG_NINTENDO_DEVICE_PLATFORM true
#	endif
#	if defined _EG_SWITCH_WINDOWS_PLATFORM || defined _EG_SWITCH2_WINDOWS_PLATFORM
#		define _EG_NINTENDO_WINDOWS_PLATFORM true
#	endif
#endif

#if defined _EG_PS4_PLATFORM || defined _EG_PS5_PLATFORM
#	define _EG_SONY_PLATFORM true
#	if !defined _EG_PS4_PLATFORM
#		define _EG_PS5_OR_NEWER_PLATFORM true
#	endif
#endif

#if defined _EG_LINUX_PLATFORM || defined _EG_APPLE_PLATFORM || defined _EG_ANDROID_PLATFORM || defined _EG_SONY_PLATFORM || defined _EG_EMSCRIPTEN_PLATFORM || defined _EG_NINTENDO_PLATFORM
#	define _EG_UNIX_PLATFORM true
#endif

#if defined _EG_GAMECORE_PLATFORM
#	if defined _GAMING_DESKTOP
#		define _EG_GAMECORE_DESKTOP_PLATFORM
#	elif defined _GAMING_XBOX_XBOXONE
#		define _EG_GAMECORE_XB1_PLATFORM
#	elif defined _GAMING_XBOX_SCARLETT
#		define _EG_GAMECORE_SCARLETT_PLATFORM
#	else
#		error unknown GameCore platform
#	endif
#endif

#if defined _EG_WINDOWS_PLATFORM || defined _EG_WINDOWSSTORE_PLATFORM || defined _EG_XB1_PLATFORM || defined _EG_GAMECORE_PLATFORM
#	define _EG_MICROSOFT_PLATFORM
#	if defined _EG_XB1_PLATFORM || defined _EG_GAMECORE_XB1_PLATFORM || defined _EG_GAMECORE_SCARLETT_PLATFORM
#		define _EG_MICROSOFT_CONSOLE_PLATFORM
#	endif
#endif

#if defined _EG_MICROSOFT_PLATFORM || defined _EG_NINTENDO_WINDOWS_PLATFORM
#	define _EG_MS_COMPILER true
#endif

#if 0
#	define _EG_BIGENDIAN_PLATFORM true
#else
#	define _EG_LITTLEENDIAN_PLATFORM true
#endif

// define EG_DEBUGGER when other macros indicate a debug configuration and it has not explicitly been deactivated via EG_NDEBUG
#if !defined EG_DEBUGGER && defined _DEBUG && !defined NDEBUG && !defined EG_NDEBUG
#	define EG_DEBUGGER true
#endif

#if !defined EG_NOLOGGING
#	define EG_LOGGING true
#endif

#if !defined EG_NO_CPP11
#	define EG_PLATFORM_SUPPORTS_CPP11 true
#endif

#if defined EG_PLATFORM_SUPPORTS_CPP11 && !defined _EG_LINUX_PLATFORM
#	define EG_PLATFORM_SUPPORTS_MOVE_SEMANTICS true
#endif

#if defined EG_PLATFORM_SUPPORTS_CPP11 && ((!defined _EG_LINUX_PLATFORM) || (__GNUC__ >= 5 || __GNUC__ == 4 && (__GNUC_MINOR__ >= 9)))
#	define EG_PLATFORM_SUPPORTS_ATOMICS true
#endif

#if !defined _EG_EMSCRIPTEN_PLATFORM
#	define EG_PLATFORM_SUPPORTS_MULTITHREADING true
#endif

#ifndef _nByte_defined_
#	define _nByte_defined_ true
	typedef unsigned char nByte; // unsigned 8  bit value type.
#endif

#ifndef _INT64
#	define _INT64 true
	typedef signed long long   int64;  /* Signed 64 bit value */
	typedef unsigned long long uint64; /* Unsigned 64 bit value */
#endif

#if defined _EG_UNIX_PLATFORM
#	if defined _EG_APPLE_PLATFORM
#		include <AvailabilityMacros.h>
#		import <CoreFoundation/CFString.h>
#		import <CoreFoundation/CoreFoundation.h>
#		include <sys/time.h>
#	elif defined _EG_ANDROID_PLATFORM
#		include <sys/time.h>
#		include <sys/limits.h>
#		include <string.h>
#	elif defined _EG_LINUX_PLATFORM
#		include <sys/time.h>
#		include <iconv.h>
#		include <limits.h>
#		include <string.h>
#	elif defined _EG_EMSCRIPTEN_PLATFORM
#		include <iconv.h>
#	endif
#endif

#include "Common-cpp/inc/Helpers/debug_assert.h"

// Definitions for memory management //////////////////////////////////////
#include <memory.h>
#define MEMCPY(dst, src, size) memcpy(dst, src, size)
#define MEMSET(dst, c, size)   memset(dst, c, size)
#define ZEROAT(ptr)            (void)MEMSET(ptr, 0, sizeof(*ptr))

// Strings //////////////////////////////////////
#ifndef STRINGIFY
#	define STRINGIFY(param) #param
#endif

#include <wchar.h>

/* ATTENTION:
wchar_t is UTF16 on Windows, but UTF32 on UNIX and it's inheritors OSX/iOS, although OSX/iOS-NSString/CFString uses UTF16,
so you have to convert EG_CHAR to/from NSString/CFString, but redefining wchar_t to be 16bit on UNIX or defining EG_CHAR to be always
16bit would have meant reimplementing the standard wchar_t library on UNIX */
typedef wchar_t EG_CHAR;

// widestring versions of predefined macros
// __WFILE__ and __WFUNCTION__ instead of __FILEW__ and __FUNCTIONW__, to avoid conflicts with MS system libs, which do not #ifdef their defines
#ifndef WIDEN
#	define WIDEN2(str) (L ## str)
#	define WIDEN(str) WIDEN2(str)
#endif
#ifndef __WFILE__
#	define __WFILE__ WIDEN(__FILE__)
#endif
#ifndef __WFUNCTION__
#	if defined _EG_MS_COMPILER
#		define __WFUNCTION__ WIDEN(__FUNCTION__)
#	else
#		define __WFUNCTION__ ExitGames::Common::JString(__FUNCTION__).cstr() // we can't use preprocessor token pasting here, as on GCC __FUNCTION__ is not a macro like in VS
#	endif
#endif

#if defined _EG_MICROSOFT_PLATFORM
#	define SNWPRINTF                        _snwprintf
#elif defined _EG_ANDROID_PLATFORM || defined _EG_EMSCRIPTEN_PLATFORM
#	define SNWPRINTF                        EG_swprintf // very expensive, use with care!
#else
#	define SNWPRINTF                        swprintf
#endif

// format specifiers:
#define EG_FRMT_SPCFR_STRTOSTR_A "s"
#define EG_FRMT_SPCFR_WSTRTOSTR_A "ls" // "S" would also work
#ifdef _EG_NINTENDO_DEVICE_PLATFORM
#	define EG_FRMT_SPCFR_STRTOWSTR_A "s"
#else
#	define EG_FRMT_SPCFR_STRTOWSTR_A "hs" // "s" would also work with other compilers, except with MSVC, for which it would have to be "S"
#endif
#define EG_FRMT_SPCFR_WSTRTOWSTR_A "ls"

#define EG_FRMT_SPCFR_STRTOSTR_W L"s"
#define EG_FRMT_SPCFR_WSTRTOSTR_W L"ls" // "S" would also work
#ifdef _EG_NINTENDO_DEVICE_PLATFORM
#	define EG_FRMT_SPCFR_STRTOWSTR_W L"s"
#else
#	define EG_FRMT_SPCFR_STRTOWSTR_W L"hs" // "s" would also work with other compilers, except with MSVC, for which it would have to be "S"
#endif
#define EG_FRMT_SPCFR_WSTRTOWSTR_W L"ls"

// Time access and control functions //////////////////////////////////////

// Attention: Do not use these macros, if you need the absolute time on different platforms, because they have completely different base dates
// on different platforms and are only to be used for time intervals between two calls

#include <time.h>
#if defined _EG_MICROSOFT_PLATFORM
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	ifndef _WIN32_WINNT
#		if defined _EG_GAMECORE_DESKTOP_PLATFORM
#			define _WIN32_WINNT _WIN32_WINNT_WIN10
#		else
#			define _WIN32_WINNT _WIN32_WINNT_WIN8
#		endif
#	endif
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <Windows.h>
#	if defined _EG_WINDOWS_PLATFORM || defined _EG_GAMECORE_DESKTOP_PLATFORM
#		include <Mmsystem.h>
#		define GETTIMEMS() static_cast<int>(timeGetTime()) // returns the number of milliseconds for which the computer was powered on (overflowing all about 49 days!), higher precision than GetTickCount64(), but only available on Windows Desktop and GameCore Desktop
#	else
#		define GETTIMEMS() static_cast<int>(GetTickCount64()) // returns the number of milliseconds for which the computer was powered on (overflowing all about 49 days!)
#	endif
#elif defined _EG_UNIX_PLATFORM
	int getTimeUnix(void);
#	define GETTIMEMS getTimeUnix // returns the number of milliseconds passed since 1970 (overflowing all about 49 days!)
#endif
#define GETUPTIMEMS GETTIMEMS

#include <stdio.h>

// Debug output functions ////////////////////////////////////////////////
#if defined _EG_LINUX_PLATFORM || defined _EG_EMSCRIPTEN_PLATFORM || defined _EG_NINTENDO_PLATFORM
	#include <stdarg.h>
#endif
#if defined DBGPRINTF_LEVEL || defined DBGPRINTF_MEMORY_ACTIVE || defined DBGPRINTF_PERFORMANCE_ACTIVE

#if defined _EG_MS_COMPILER && defined __cplusplus
#	pragma managed(push, off)
#endif
#ifdef __cplusplus
	extern "C"
	{
#endif
		static __inline void __DBGPRINTF(const char* message, ...)
		{
			va_list argptr;
			va_start(argptr, message);
			vfprintf(stderr, message, argptr);
			va_end(argptr);
			fprintf(stderr, "\n");
		}
#ifdef  __cplusplus
	}
#endif
#if defined _EG_MS_COMPILER && defined __cplusplus
#	pragma managed(pop)
#endif

#define DBGPRINTF_LEVEL_DEBUG_VERBOSE 6
#define DBGPRINTF_LEVEL_DEBUG         5
#define DBGPRINTF_LEVEL_INFO          4
#define DBGPRINTF_LEVEL_WARNING       3
#define DBGPRINTF_LEVEL_ERROR         2
#define DBGPRINTF_LEVEL_FATAL         1
#define DBGPRINTF_LEVEL_OFF           0

#define DBGPRINTF_EXSTR_FORMAT "%13s: %-30s %-50s line: %5u - "

#define CUTPATH(file) (strrchr(file, '/')?strrchr(file, '/')+1:strrchr(file, '\\')?strrchr(file, '\\')+1:file)

#if DBGPRINTF_LEVEL >= DBGPRINTF_LEVEL_DEBUG_VERBOSE
#	define DBGPRINTF_DEBUG_VERBOSE      fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "DEBUG_VERBOSE", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_DEBUG_VERBOSE(...) ((void)0)
#endif

#if DBGPRINTF_LEVEL >= DBGPRINTF_LEVEL_DEBUG
#	define DBGPRINTF_DEBUG              fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "DEBUG", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_DEBUG(...)         ((void)0)
#endif

#if DBGPRINTF_LEVEL >= DBGPRINTF_LEVEL_INFO
#	define DBGPRINTF_INFO               fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "INFO", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_INFO(...)          ((void)0)
#endif

#if DBGPRINTF_LEVEL >= DBGPRINTF_LEVEL_WARNING
#	define DBGPRINTF_WARNING            fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "WARNING", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_WARNING(...)       ((void)0)
#endif

#if DBGPRINTF_LEVEL >= DBGPRINTF_LEVEL_ERROR
#	define DBGPRINTF_ERROR              fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "ERROR", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_ERROR(...)         ((void)0)
#endif

#if DBGPRINTF_LEVEL >= DBGPRINTF_LEVEL_FATAL
#	define DBGPRINTF_FATAL              fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "FATAL", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_FATAL(...)         ((void)0)
#endif

#ifdef DBGPRINTF_MEMORY_ACTIVE
#	define DBGPRINTF_MEMORY             fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "MEMORY", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_MEMORY(...)        ((void)0)
#endif

#ifdef DBGPRINTF_PERFORMANCE_ACTIVE
#	define DBGPRINTF_PERFORMANCE        fprintf(stderr, DBGPRINTF_EXSTR_FORMAT, "PERFORMANCE", CUTPATH(__FILE__), __FUNCTION__, __LINE__), __DBGPRINTF
#else
#	define DBGPRINTF_PERFORMANCE(...)   ((void)0)
#endif

#define DBGPRINTF                       DBGPRINTF_ERROR

#else
#	define DBGPRINTF_DEBUG_VERBOSE(...) ((void)0)
#	define DBGPRINTF_DEBUG(...)         ((void)0)
#	define DBGPRINTF_INFO(...)          ((void)0)
#	define DBGPRINTF_WARNING(...)       ((void)0)
#	define DBGPRINTF_ERROR(...)         ((void)0)
#	define DBGPRINTF_FATAL(...)         ((void)0)
#	define DBGPRINTF_MEMORY(...)        ((void)0)
#	define DBGPRINTF_PERFORMANCE(...)   ((void)0)
#	define DBGPRINTF(...)               ((void)0)
#endif

// debugging helpers
#define DEBUG_ONLY_S(arg) (DEBUG_ONLY(arg))
#define RELEASE_ONLY_S(arg) (RELEASE_ONLY(arg))
#define DEBUG_RELEASE_S(dbgarg, relarg) (DEBUG_RELEASE(dbgarg, relarg))
#ifdef EG_DEBUGGER
#	define DEBUG_ONLY(arg) arg
#	define RELEASE_ONLY(arg) ((void)0)
#	define DEBUG_RELEASE(dbgarg, relarg) dbgarg
#else
#	define DEBUG_ONLY(arg) ((void)0)
#	define RELEASE_ONLY(arg) arg
#	define DEBUG_RELEASE(dbgarg, relarg) relarg
#endif

// suppress unused parameter warnings, but still let doxygen see parameters
#ifdef EG_DOC
#	define EG_UNUSED(arg) arg
#else
#	define EG_UNUSED(arg)
#endif

// attribute and pragma definitions //////////////////////////////////////
#if defined _EG_APPLE_PLATFORM
#	define EG_ATTRIBUTE_VISIBILITY_HIDDEN __attribute__((visibility("hidden")))
#	define EG_PRAGMA_VISIBILITY_PUSH_HIDDEN _Pragma("GCC visibility push(hidden)")
#	define EG_PRAGMA_VISIBILITY_POP _Pragma("GCC visibility pop")
#else
#	define EG_ATTRIBUTE_VISIBILITY_HIDDEN
#	define EG_PRAGMA_VISIBILITY_PUSH_HIDDEN
#	define EG_PRAGMA_VISIBILITY_POP
#endif
