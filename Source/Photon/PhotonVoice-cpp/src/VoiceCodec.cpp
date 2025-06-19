/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/VoiceCodec.h"

/** @file PhotonVoice-cpp/inc/VoiceCodec.h */

namespace ExitGames
{
	namespace Voice
	{
		using namespace Common;

		UnsupportedCodecError::UnsupportedCodecError(const JString& info, int codec, const Voice::Logger& mLogger)
		{
			mError = JString("[PV] ") + info + L": unsupported codec " + codec;
			EGLOG(DebugLevel::ERRORS, mError);
		}

		void UnsupportedCodecError::close(void)
		{
		}

		JString UnsupportedCodecError::getError(void) const
		{
			return mError;
		}

		void UnsupportedCodecError::open(const VoiceInfo& /*info*/)
		{
		}

		void UnsupportedCodecError::setOutput(void* /*opaque*/, void(* /*output*/)(void*, const Buffer<nByte>&, FrameFlags::Enum))
		{
		}

		Buffer<nByte> UnsupportedCodecError::dequeueOutput(FrameFlags::Enum& /*flags*/)
		{
			return Buffer<nByte>();
		}

		void UnsupportedCodecError::input(const FrameBuffer& /*buf*/)
		{
		}
	}
}