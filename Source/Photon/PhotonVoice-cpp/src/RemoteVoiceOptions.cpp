/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/RemoteVoiceOptions.h"
#include "PhotonVoice-cpp/inc/OpusCodec.h"

namespace ExitGames
{
	namespace Voice
	{
		using namespace Common;
		using namespace Common::Helpers;
		using namespace Common::MemoryManagement;

		RemoteVoiceOptions::RemoteVoiceOptions(const Voice::Logger& logger, const Common::JString& logPrefix, const VoiceInfo& voiceInfo)
			: mLogger(logger)
			, mLogPrefix(logPrefix)
			, mVoiceInfo(voiceInfo)
			, mpRemoteVoiceRemoveActionOpaque(NULL)
			, mpRemoteVoiceRemoveAction(NULL)
		{
		}

		/// <summary>
		/// Register a method to be called when the remote voice is removed.
		/// </summary>
		void RemoteVoiceOptions::setRemoteVoiceRemoveAction(void* pOpaque, void(*pAction)(void*))
		{
			mpRemoteVoiceRemoveActionOpaque = pOpaque;
			mpRemoteVoiceRemoveAction = pAction;
		}

		void RemoteVoiceOptions::setOutput(void* pOpaque, void(*pOutput)(void*, const Buffer<float>&))
		{
			setOutput(pOpaque, pOutput, L"Buffer<float>");
		}
		void RemoteVoiceOptions::setOutput(void* pOpaque, void(*pOutput)(void*, const Buffer<short>&))
		{
			setOutput(pOpaque, pOutput, L"Buffer<short>");
		}

		template <class T>
		void RemoteVoiceOptions::setOutput(void* opaque, void(*output)(void*, const Buffer<T>&), const JString& typeName)
		{
			if(mVoiceInfo.getCodec() == Codec::AUDIO_OPUS)
			{
				EGLOG(DebugLevel::INFO, mLogPrefix + L": Creating default decoder " + mVoiceInfo.getCodec() + " for output " + typeName);
				mspDecoder = allocate<Opus::Decoder<T> >(opaque, output, mLogger);
			}
			else
				EGLOG(DebugLevel::ERRORS, mLogPrefix + L": " + typeName + L" output set for non - audio decoder " + mVoiceInfo.getCodec());
		}

		void RemoteVoiceOptions::setDecoder(SharedPointer<IDecoder> decoder)
		{
			mspDecoder = decoder;
		}

		JString& RemoteVoiceOptions::toString(JString& retStr, bool /*withTypes*/) const
		{
			return retStr
				+= JString(L"{")
				+ L" decoder=" + L"{" + Helpers::TypeName::get(mspDecoder) + L"}"
				+ L"}";
		}
	}
}