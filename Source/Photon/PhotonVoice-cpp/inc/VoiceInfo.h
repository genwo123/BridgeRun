/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "PhotonVoice-cpp/inc/defines.h"
#include "PhotonVoice-cpp/inc/Enums/Codec.h"

namespace ExitGames
{
	namespace Voice
	{
		class IAudioDesc;
		/// <summary>Describes stream properties.</summary>
		class VoiceInfo : public Common::ToString
		{
		public:
			using ToString::toString;

			VoiceInfo(void);

			Codec::Enum getCodec(void) const;
			VoiceInfo& setCodec(Codec::Enum codec);
			int getSamplingRate(void) const;
			VoiceInfo& setSamplingRate(int samplingRate);
			int getChannels(void) const;
			VoiceInfo& setChannels(int channels);
			int getFrameDurationUs(void) const;
			VoiceInfo& setFrameDurationUs(int frameDurationUs);
			int getBitrate(void) const;
			VoiceInfo& setBitrate(int bitrate);
			const Common::Object& getUserData(void) const;
			VoiceInfo& setUserData(const Common::Object& userData);

			int getFrameDurationSamples(void) const;
			int getFrameSize(void) const;
			int getWidth(void) const;
			VoiceInfo& setWidth(int width);
			int getHeight(void) const;
			VoiceInfo& setHeight(int height);
			int getFPS(void) const;
			VoiceInfo& setFPS(int height);
			int getKeyFrameInt(void) const;
			VoiceInfo& setKeyFrameInt(int height);

			static VoiceInfo createAudioOpus(int samplingRate, int channels, int frameDurationUs, int bitrate, const Common::Object& userdata=Common::Object());
#if EG_PHOTON_VOICE_VIDEO_ENABLE
			static VoiceInfo createVideoVP8(int bitrate, int width=0, int height=-1, const Common::Object& userdata=Common::Object());
#endif
			virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
		private:
			Codec::Enum mCodec;

			// audio
			int mSamplingRate;
			int mChannels;
			int mFrameDurationUs;
			int mBitrate;
			Common::Object mUserData;

			// video
			int mWidth;
			int mHeight;
			int mFPS;
			int mKeyFrameInt;

			friend class VoiceClient;
		};
	}
}