/* Exit Games Common - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Helpers/Stringification/PrimitiveToStringConverterBase.h"

namespace ExitGames
{
	namespace Common
	{
		namespace Helpers
		{
			template<typename Etype> class PrimitiveToStringConverter : public PrimitiveToStringConverterBase<Etype>{};
			template<typename Etype> class PrimitiveToStringConverter<Etype*> : public PrimitiveToStringConverterBase<Etype*>{};
		}
	}
}