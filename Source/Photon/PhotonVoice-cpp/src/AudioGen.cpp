/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/AudioGen.h"

/** @file PhotonVoice-cpp/inc/AudioGen.h */

namespace ExitGames
{
	namespace Voice
	{
		namespace AudioUtil
		{
			using namespace Common;
			using namespace Common::Helpers;

			template<typename T>
			static float toneAmp(void)
			{
				return 0;
			}

			template<>
			float toneAmp<float>(void)
			{
				return 0.2f;
			}

			template<>
			float toneAmp<short>(void)
			{
				return 0.2f*SHRT_MAX;
			}

			template<typename T>
			ToneGen<T>::ToneGen(int frequency, int samplingRate, int channels)
				: mSamplingRate(samplingRate)
				, mChannels(channels)
				, mAmp(toneAmp<T>())
				, mK(2*3.14159265358979323846*frequency/samplingRate)
				, timeSamples(0)
			{
			}

			template<typename T>
			void ToneGen<T>::next(Buffer<T>& buf)
			{
				T* b = buf.getArray();
				for(int i=0; i<buf.getSize()/mChannels; ++i)
				{
					double v = sin(timeSamples++*mK)*mAmp;
					for(int j=0; j<mChannels; ++j)
						*b++ = static_cast<T>(v);
				}
			}


			template<typename T>
			ToneAudioPusherBase<T>::ToneAudioPusherBase(int frequency, int samplingRate, int channels)
				: mSamplingRate(samplingRate)
				, mChannels(channels)
				, mpOpaque(NULL)
				, mpCallback(NULL)
				, mToneGen(frequency, samplingRate, channels)
			{
			}

			template<typename T>
			void ToneAudioPusherBase<T>::setCallback(void* opaque, void(*pCallback)(void*, const Buffer<T>&))
			{
				mpOpaque = opaque;
				mpCallback = pCallback;
			}

			/// <summary>Number of channels in the audio signal.</summary>
			template<typename T>
			int ToneAudioPusherBase<T>::getChannels(void) const
			{
				return mChannels;
			}

			/// <summary>Sampling rate of the audio signal (in Hz).</summary>
			template<typename T>
			int ToneAudioPusherBase<T>::getSamplingRate(void) const
			{
				return mSamplingRate;
			}

			/// <summary>If not an empty string, then the audio object is in invalid state.</summary>
			template<typename T>
			JString ToneAudioPusherBase<T>::getError(void) const
			{
				return JString();
			}

			template<typename T>
			JString& ToneAudioPusherBase<T>::toString(JString& retStr, bool /*withTypes*/) const
			{
				return retStr
					+= JString(L"{")
					+ L"samplingRate=" + mSamplingRate
					+ L" ch=" + mChannels
					+ L"}";
			}


			/// <summary>Create a new ToneAudioPusher instance</summary>
			/// <param name="frequency">Frequency of the generated tone (in Hz).</param>
			/// <param name="bufSizeMs">Size of buffers to push (in milliseconds).</param>
			/// <param name="samplingRate">Sampling rate of the audio signal (in Hz).</param>
			/// <param name="channels">Number of channels in the audio signal.</param>
			template<typename T>
			ToneAudioPusher<T>::ToneAudioPusher(int frequency, int bufSizeMs, int samplingRate, int channels)
				: ToneAudioPusherBase<T>(frequency, samplingRate, channels)
				, mBufSizeSamples(bufSizeMs*samplingRate/1000)
				, mWorkerThreadRunning(false)
				, mWorkerThreadExit(false)
			{
			}

			template<typename T>
			ToneAudioPusher<T>::~ToneAudioPusher(void)
			{
				mWorkerThreadExit = true;
				while(mWorkerThreadRunning);
			}

			/// <summary>Set the callback function used for pushing data</summary>
			/// <param name="opaque">Opaque pointer for the context</param>
			/// <param name="pCallback">Callback function to use</param>
			template<typename T>
			void ToneAudioPusher<T>::setCallback(void* opaque, void(*pCallback)(void*, const Buffer<T>&))
			{
				ToneAudioPusherBase<T>::setCallback(opaque, pCallback);
				mWorkerThreadRunning = true;
				Thread::create(workerStarter, this);
			}

			template<typename T>
			void ToneAudioPusher<T>::workerStarter(void* arg)
			{
				static_cast<ToneAudioPusher<T>*>(arg)->worker();
			}

#if defined _EG_EMSCRIPTEN_PLATFORM
			int usleep(unsigned); // implemented but not decalred in Emscripten
#endif
			template<typename T>
			void ToneAudioPusher<T>::worker(void)
			{
				int timeMs = GETTIMEMS();
				while(!mWorkerThreadExit)
				{
					Buffer<T> buf(mBufSizeSamples*this->mChannels);
					this->mToneGen.next(buf);
					this->mpCallback(this->mpOpaque, buf);

					timeMs += mBufSizeSamples*1000/this->mSamplingRate;
					int dt = timeMs-GETTIMEMS();
					if(dt > 0)
						SLEEP(dt);
				}

				mWorkerThreadRunning = false;
			}

			/// <summary>Create a new ToneAudioPusherService instance</summary>
			/// <param name="frequency">Frequency of the generated tone (in Hz).</param>
			/// <param name="samplingRate">Sampling rate of the audio signal (in Hz).</param>
			/// <param name="channels">Number of channels in the audio signal.</param>
			template<typename T>
			ToneAudioPusherService<T>::ToneAudioPusherService(int frequency, int samplingRate, int channels)
				: ToneAudioPusherBase<T>(frequency, samplingRate, channels)
				, mTimeMs(0)
			{

			}

			/// <summary>Set the callback function used for pushing data</summary>
			/// <param name="opaque">Opaque pointer for the context</param>
			/// <param name="pCallback">Callback function to use</param>
			template<typename T>
			void ToneAudioPusherService<T>::setCallback(void* opaque, void(*pCallback)(void*, const Buffer<T>&))
			{
				ToneAudioPusherBase<T>::setCallback(opaque, pCallback);
				mTimeMs = GETTIMEMS(); // assuming service() is called immediately
			}

			/// <summary>Call this regularly at least 10 times per sec.</summary>
			template<typename T>
			void ToneAudioPusherService<T>::service(void)
			{
				int prevTimeMs = mTimeMs;
				mTimeMs = GETTIMEMS();
				unsigned int delta = mTimeMs-prevTimeMs;
				if(delta > 200) // prevents from too large buffer when service() is not called on time
					delta = 200;
				Buffer<T> buf(this->mSamplingRate*this->mChannels*delta/1000);
				this->mToneGen.next(buf);
				this->mpCallback(this->mpOpaque, buf);
			}

			template class ToneAudioPusher<float>;
			template class ToneAudioPusher<short>;
			template class ToneAudioPusherService<float>;
			template class ToneAudioPusherService<short>;
		}
	}
}