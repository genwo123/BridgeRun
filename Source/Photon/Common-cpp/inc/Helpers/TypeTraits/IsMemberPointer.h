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
			template<typename Ctype> struct IsMemberPointer{static const bool is = false;};
			template<typename Ctype, typename Dtype> struct IsMemberPointer<Ctype* Dtype::*>{static const bool is = true;};
			template<typename Ctype, typename Dtype> struct IsMemberPointer<const Ctype* Dtype::*>{static const bool is = true;};

			/** @file */
		}
	}
}