/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/Buffer.h"

/** @file PhotonVoice-cpp/inc/Buffer.h */

namespace ExitGames
{
	namespace Voice
	{
		template class Buffer<float>;
		template class Buffer<short>;
		template class Buffer<nByte>;
	}
}