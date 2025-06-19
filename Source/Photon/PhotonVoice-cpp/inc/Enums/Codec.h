/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Common.h"
#include "PhotonVoice-cpp/inc/defines.h"

namespace ExitGames
{
	namespace Voice
	{
		/// <summary>Enum for Media Codecs supported by PhotonVoice.</summary>
		/// <remarks>Transmitted in <see cref="VoiceInfo"></see>. Do not change the values of this Enum!</remarks>
		namespace Codec
		{
			enum Enum
			{
				NONE = 0,
				/// <summary>OPUS audio</summary>
				AUDIO_OPUS = 11,
#if EG_PHOTON_VOICE_VIDEO_ENABLE
				VIDEO_VP8 = 21,
				VIDEO_VP9 = 22,
				VIDEO_AV1 = 23,
				VIDEO_H264 = 31,
				VIDEO_H265 = 32,
#endif
			};
		}
	}
}