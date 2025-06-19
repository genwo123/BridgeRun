/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "PhotonVoice-cpp/inc/VoiceAudio.h"

namespace ExitGames
{
	namespace Voice
	{
		namespace AudioUtil
		{
			template<typename T>
			class ToneGen
			{
			public:
				ToneGen(int frequency, int samplingRate, int channels);
				void next(Buffer<T>& b);
			private:
				int mSamplingRate;
				int mChannels;
				float mAmp;
				double mK;
				int64 timeSamples;
			};

			template<typename T>
			class ToneAudioPusherBase : public IAudioPusher<T>
			{
			public:
				using Common::ToString::toString;

				ToneAudioPusherBase(int frequency, int samplingRate, int channels);

				virtual int getSamplingRate(void) const;
				virtual int getChannels(void) const;
				virtual Common::JString getError(void) const;
				virtual void setCallback(void* opaque, void(*pCallback)(void*, const Buffer<T>&));

				virtual Common::JString& toString(Common::JString& retStr, bool withTypes = false) const;
			protected:
				int mSamplingRate;
				int mChannels;

				void* mpOpaque;
				void(*mpCallback)(void*, const Buffer<T>&);

				ToneGen<T> mToneGen;
			};

			/// <summary>IAudioPusher that provides a constant tone signal via a callback in a thread.</summary>
			// Helpful for debug
			template<typename T>
			class ToneAudioPusher : public ToneAudioPusherBase<T>
			{
			public:
				using Common::ToString::toString;

				ToneAudioPusher(int frequency=440, int bufSizeMs=100, int samplingRate=44100, int channels=2);
				virtual ~ToneAudioPusher(void);
				virtual void setCallback(void* opaque, void(*pCallback)(void*, const Buffer<T>&));
			private:
				static void workerStarter(void*);
				void worker(void);

				int mBufSizeSamples;
				std::atomic<bool> mWorkerThreadRunning;
				std::atomic<bool> mWorkerThreadExit;
			};

			/// <summary>IAudioPusher that provides a constant tone signal via a callback in service() called by the user.</summary>
			template<typename T>
			class ToneAudioPusherService : public ToneAudioPusherBase<T>
			{
			public:
				ToneAudioPusherService(int frequency = 440, int samplingRate = 44100, int channels = 2);
				virtual void setCallback(void* opaque, void(*pCallback)(void*, const Buffer<T>&));
				virtual void service(void);
			private:
				int mTimeMs;
			};
		}
	}
}