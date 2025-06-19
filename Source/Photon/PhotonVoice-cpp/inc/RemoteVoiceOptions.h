/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "PhotonVoice-cpp/inc/VoiceCodec.h"

namespace ExitGames
{
	namespace Voice
	{
		class LocalVoice;
		class VoiceClient;
		class IEncoder;
		class IDecoder;

		/// <summary>Event Actions and other options for a remote voice (incoming stream).</summary>
		class RemoteVoiceOptions : public Common::ToString
		{
		public:
			using ToString::toString;

			RemoteVoiceOptions(const Voice::Logger& logger, const Common::JString& logPrefix, const VoiceInfo& voiceInfo);

			void setRemoteVoiceRemoveAction(void* opaque, void(*action)(void*));
			void setOutput(void* opaque, void(*output)(void*, const Buffer<float>&));
			void setOutput(void* opaque, void(*output)(void*, const Buffer<short>&));
			void setDecoder(Common::Helpers::SharedPointer<IDecoder> decoder);

			virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
		private:
			template <class T>
			void setOutput(void* opaque, void(*output)(void*, const Buffer<T>&), const Common::JString& typeName);

			const Voice::Logger& mLogger;
			Common::JString mLogPrefix;
			VoiceInfo mVoiceInfo;

			void* mpRemoteVoiceRemoveActionOpaque;
			void(*mpRemoteVoiceRemoveAction)(void*);
			Common::Helpers::SharedPointer<IDecoder> mspDecoder;

			friend class RemoteVoice;
		};
	}
}