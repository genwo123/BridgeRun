/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "PhotonVoice-cpp/inc/AudioUtil.h"

namespace ExitGames
{
	namespace Voice
	{
		class AudioOutDelayConfig
		{
		public:
			AudioOutDelayConfig(int low, int high, int max, int speedUpPerc);
			int getLow(void) const;
			int getHigh(void) const;
			int getMax(void) const;
			int getSpeedUpPerc(void) const;
		private:
			int mLow;         // ms: (Target) Audio player initilizes the delay with this value on Start and after flush and moves to it during corrections
			int mHigh;        // ms: Audio player tries to keep the delay below this value.
			int mMax;         // ms: Audio player guarantees that the delay never exceeds this value.
			int mSpeedUpPerc; // playback speed-up to catch up the stream
		};

		// Consumes audio frames via push(), optionally resizes and writes (OutWrite) them to the output to keep constant delay
		// between output playback position (outGetPos) and input stream position (advanced with each write).
		// Assumes output is always playing.
		template<typename T>
		class AudioOutDelayControl : public IAudioOut<T>
		{
		public:
			AudioOutDelayControl(const Voice::Logger& logger, AudioOutDelayConfig delayConfig, const Common::JString& logPrefix, bool debugInfo);

			// methods to implement
			// returns playback position in samples, either absolute or in a ring buffer (in the latter case, the loop detector restores the absolute position)
			virtual int64 outGetPos(void) const = 0;
			virtual void outCreate(int frequency, int channels, int bufferSamples) = 0;
			virtual void outStart(void) = 0;
			// offsetSamples is an offset in the ring buffer
			virtual void outWrite(Buffer<T> data, int offsetSamples) = 0;

			// IAudioOut
			virtual bool isPlaying(void) const;
			virtual void start(int frequency, int channels, int frameSamples);
			virtual void flush(void);
			virtual void stop(void);
			virtual void push(Buffer<T> frame);
			virtual void service(void);
			virtual int getLag(void) const;

			bool isFlushed(void) const;
		protected:
			int mBufferSamples;
			int mFrequency;
			int mChannels;
			const Voice::Logger& mLogger;
			Common::JString mLogPrefix;
		private:
			static const int TEMPO_UP_SKIP_GROUP = 6;
			static const int NO_PUSH_TIMEOUT_MS = 100; // should be greater than Push() call interval

			int mFrameSamples;
			int mFrameSize;

			// stream playback state
			int64 mWriteSamplePos; // uupdated and read only in push()
			std::atomic<int64> mPlaySamplePos; // updated only in service(), read only in push()

			// loop detection and loop count
			int64 mOutPosPrev;
			int mPlayLoopCount;

			AudioOutDelayConfig mDelayConfig;
			bool mStarted;
			bool mFlushed;

			int mTargetDelaySamples;
			int mUpperTargetDelaySamples;       // correct if higher: gradually move to target via input frames resampling
			int mMaxDelaySamples;               // set delay to this value if delay is higher

			int mLastPushTime;

			bool mDebugInfo;

			AudioUtil::TempoUp<T> mTempoUp;
			bool mTempoChangeHQ; // true if library is available
			bool mCatchingUp;

			Buffer<T> mZeroFrame;
			Buffer<T> mResampledFrame;

            static Buffer<T> mEmptyBuffer;

			bool processFrame(Buffer<T> frame, int64 playSamplePos, bool flush);
			int writeResampled(Buffer<T> f, int resampledLenSamples);
			void logDebug(Common::JString const& header, int lagSamples, int64 playSamplePos) const;
		};

		template<typename T>
		Buffer<T> AudioOutDelayControl<T>::mEmptyBuffer;
	}
}