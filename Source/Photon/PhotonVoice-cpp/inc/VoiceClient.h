/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Common.h"
#include "PhotonVoice-cpp/inc/Internal/RemoteVoice.h"
#include "PhotonVoice-cpp/inc/RemoteVoiceInfo.h"
#include "PhotonVoice-cpp/inc/Voice.h"
#include "PhotonVoice-cpp/inc/Buffer.h"

namespace ExitGames
{
	namespace Voice
	{
		class VoiceClient;
		class VoiceInfo;
		class LocalVoice;
		template<typename T> class LocalVoiceFramed;
		template<typename T> class LocalVoiceAudio;
		template<typename T> class IAudioPusher;
		class RemoteVoiceOptions;
		class IEncoder;

		class SendFrameParams
		{
		public:
			SendFrameParams(bool targetMe, const int* targetPlayers, int numTargetPlayers, nByte interestGroup, bool reliable, bool encrypt);
			bool getTargetMe(void) const;
			const int* getTargetPlayers(void) const;
			int getNumTargetPlayers(void) const;
			nByte getInterestGroup(void) const;
			bool getReliable(void) const;
			bool getEncrypt(void) const;
		private:
			bool mTargetMe;
			const int* mpTargetPlayers;
			int mNumTargetPlayers;
			nByte mInterestGroup;
			bool mReliable;
			bool mEncrypt;
		};

		class IVoiceTransport
		{
		public:
			IVoiceTransport(void) : mpVoiceClient(NULL) {}
			virtual ~IVoiceTransport(void) {}

			virtual bool isChannelJoined(int channelId) = 0;
			// 3 methods below are never called with targetMe == false AND targetPlayers != NULL AND targetPlayers size == 0, because there are no targets in this case
			// if targetMe == true, local player is targeted
			// if targetPlayers == NULL, all other players are targeted
			// if targetPlayers != NULL, other players in possibly empty targetPlayers are targeted
			// Transport should not modify targetPlayers.
			virtual void sendVoiceInfo(const LocalVoice& voice, int channelId, bool targetMe, const int* targetPlayers, int numTargetPlayers) = 0;
			virtual void sendVoiceRemove(const LocalVoice& voice, int channelId, bool targetMe, const int* targetPlayers, int numTargetPlayers) = 0;
			// unlike C#, the buffer can't be reused by the caller after this call because buffers may be accumulated in the queue
			virtual void sendFrame(Buffer<nByte> data, FrameFlags::Enum flags, unsigned short evNumber, nByte frNumber, nByte voiceId, int channelId, const SendFrameParams& par) = 0;

			virtual Common::JString channelIdStr(int channelId) = 0;
			virtual Common::JString playerIdStr(int playerId) = 0;
			virtual int getPayloadFragmentSize(const SendFrameParams& par) = 0;
		protected:
			VoiceClient* mpVoiceClient; // set by Voice Client in constructor
		private:
			friend class VoiceClient;
		};

		/// <summary>
		/// Voice client interacts with other clients on network via IVoiceTransport.
		/// </summary>
		class VoiceClient
		{
		public:
			class CreateOptions
			{
			public:
				CreateOptions();
				// a range of available voice ids (defaults to 1-15) to avoid conflicts when multiple clients may create voices for the same player (e.g.client and server plugin)
				nByte getVoiceIDMin(void) const;
				CreateOptions& setVoiceIDMin(nByte);
				nByte getVoiceIDMax(void) const;
				CreateOptions& setVoiceIDMax(nByte);
			private:
				static const nByte DEFAULT_VOICE_ID_MIN = 1; // 0 means invalid id
				static const nByte DEFAULT_VOICE_ID_MAX = 15; // preserve ids for other clients creating voices for the same player (server plugin)

				nByte mVoiceIDMin;
				nByte mVoiceIDMax;
			};

			VoiceClient(const Voice::Logger& logger, IVoiceTransport* transport, const CreateOptions& options = CreateOptions());
			virtual ~VoiceClient(void);

			typedef void(RemoteVoiceInfoCallback)(void* opaque, int channelId, int playerId, nByte voiceId, const VoiceInfo& voiceInfo, RemoteVoiceOptions& options);
			void setOnRemoteVoiceInfoAction(void* opaque, RemoteVoiceInfoCallback* callback);
			void service(void);
			IVoiceTransport* getTransport(void) const;
			void  forEachLocalVoice(void* opaque, void(*op)(void*, const LocalVoice&)) const;
			void forEachLocalVoiceInChannel(int channelId, void* opaque, void(*op)(void*, const LocalVoice&)) const;
			Common::JVector<RemoteVoiceInfo> getRemoteVoiceInfos(void) const;

			bool getThreadingEnabled(void) const; // is constant per platform but we can support setter and single-threaded voices on multi-threaded platforms
			void setRemoteVoiceDelayFrames(Codec::Enum codec, int delayFrames);

			int getFramesLost(void) const;
			int getFramesReceived(void) const;
			int getFramesSent(void) const;
			int getFramesSentBytes(void) const;
			int getRoundTripTime(void) const;
			int getRoundTripTimeVariance(void) const;
			bool getSuppressInfoDuplicateWarning(void) const;
			void setSuppressInfoDuplicateWarning(bool yes);
			int getDebugLostPercent(void) const;
			void setDebugLostPercent(int debugLostPercent);

			// LovalVoice create utility
			LocalVoice* createLocalVoice(const VoiceInfo& voiceInfo, int channelId=0, const VoiceCreateOptions& options = VoiceCreateOptions());
			template <typename T>LocalVoiceFramed<T>* createLocalVoiceFramed(const VoiceInfo& voiceInfo, int frameSize, int channelId=0, const VoiceCreateOptions& options = VoiceCreateOptions());
			template <typename T>LocalVoiceAudio<T>* createLocalVoiceAudio(const VoiceInfo& voiceInfo, const IAudioDesc& audioSourceDesc, int channelId = 0, const VoiceCreateOptions& options = VoiceCreateOptions());
			template <typename T>LocalVoiceAudio<T>* createLocalVoiceAudioFromSource(const VoiceInfo& voiceInfo, IAudioPusher<T>* source, int channelId=0, const VoiceCreateOptions& options = VoiceCreateOptions());

#if EG_PHOTON_VOICE_VIDEO_ENABLE
//			LocalVoiceVideo* CreateLocalVoiceVideo(const VoiceInfo& voiceInfo, int channelId = 0,  const VoiceCreateOptions& options = VoiceCreateOptions());
#endif
			void removeLocalVoice(LocalVoice& voice);

			/*internal*/ Common::JString channelStr(int channelId);
			/*internal*/ Common::JString playerStr(int playerId);

			// transport to voice client interface (TODO: not public to user code)

			void onJoinChannel(int channel);
			void onJoinAllChannels(void);
			void onLeaveChannel(int channel);
			void onLeaveAllChannels(void);
			void onPlayerJoin(int channelId, int playerId);
			void onPlayerJoin(int playerId);
			void onPlayerLeave(int channelId, int playerId);
			void onPlayerLeave(int playerId);
			void onVoiceInfo(int channelId, int playerId, nByte voiceId, int eventBufferSize, const VoiceInfo& info);
			void onVoiceRemove(int channelId, int playerId, const Common::JVector<nByte>& voiceIds);

			/*internal*/ void onFrame(int channelId, int playerId, nByte voiceId, unsigned short evNumber, const FrameBuffer& receivedBytes, bool isLocalPlayer);
		private:
			VoiceClient(const VoiceClient&);
			VoiceClient& operator=(const VoiceClient&); // non-copyable

			LocalVoice* tryAddLocalVoice(LocalVoice* pLocalVoice);
			nByte getNewVoiceId(void);
			nByte idInc(nByte id);
			void addLocalVoice(nByte newId, int channelId, LocalVoice* pV);
			void clearRemoteVoices(void);
			void clearRemoteVoicesInChannel(int channelId);
			void clearRemoteVoicesForPlayer(bool allChannels, int channelId, int playerId);

			IVoiceTransport* mpTransport;
			const Voice::Logger& mLogger;
			nByte mVoiceIDMin;
			nByte mVoiceIDMax;
			nByte mVoiceIdLast;

			// voice id -> LocalVoice*
			// VoiceClient owns LocalVoice instances, use removeLocalVoice() or LocalVoice::removeSelf() for cleanup
			Common::JDictionary<nByte, Common::Helpers::SharedPointer<LocalVoice> > mLocalVoices;
			Common::JDictionary<int, Common::JVector<Common::Helpers::SharedPointer<LocalVoice> > > mLocalVoicesPerChannel;
			mutable Common::Helpers::Spinlock mLocalVoicesLock; // controls access to both dictionaries above
			// player id -> voice id -> RemoteVoice*
			// SharedPointer simplifies RemoteVoice instances management in multiple threads
			Common::JDictionary<int, Common::JDictionary<nByte, Common::Helpers::SharedPointer<RemoteVoice> > > mRemoteVoices;
			mutable Common::Helpers::Spinlock mRemoteVoicesLock;

			Common::JDictionary<Codec::Enum, int> mRemoteVoiceDelayFramesPerCodec;
#if defined PV_DEBUG_ECHO_RTT_MEASURE
			int mPrevRtt;
#endif
			int mEventsLost;
			int mFramesLost;

			int mFramesFragPart;
			int mFramesRecovered;
			int mFramesLate;

			int mFramesReceived;

			int mFramesReceivedFEC;
			int mFramesTryFEC;
			int mFramesReceivedFragments;
			int mFramesReceivedFragmented;

			int mFramesSent;
			int mFramesSentBytes;
			int mRoundTripTime;
			int mRoundTripTimeVariance;
			bool mSuppressInfoDuplicateWarning;
			RemoteVoiceInfoCallback* mpOnRemoteVoiceInfoAction;
			void* mpOnRemoteVoiceInfoActionOpaque;
			int mDebugLostPercent;

			friend class RemoteVoice;
		};



		/// <summary>
		/// Creates basic outgoing stream w/o data processing support. Provided encoder should generate output data stream.
		/// </summary>
		/// <param name="voiceInfo">Outgoing stream parameters. Set applicable fields to read them by encoder and by receiving client when voice created.</param>
		/// <param name="channelId">Transport channel specific to transport.</param>
		/// <param name="options">Voice creation options.</param>
		/// <returns>Outgoing stream handler.</returns>
		inline LocalVoice* VoiceClient::createLocalVoice(const VoiceInfo& voiceInfo, int channelId, const VoiceCreateOptions& options)
		{
			LocalVoice* v;
			ALLOCATE(LocalVoice, v, mLogger, this, getNewVoiceId(), voiceInfo, channelId, options);
			return tryAddLocalVoice(v);
		}

		/// <summary>
		/// Creates outgoing stream consuming sequence of values passed in array buffers of arbitrary length which repacked in frames of constant length for further processing and encoding.
		/// </summary>
		/// <typeparam name="T">Type of data consumed by outgoing stream (element type of array buffers).</typeparam>
		/// <param name="voiceInfo">Outgoing stream parameters. Set applicable fields to read them by encoder and by receiving client when voice created.</param>
		/// <param name="frameSize">Size of buffer LocalVoiceFramed repacks input data stream to.</param>
		/// <param name="channelId">Transport channel specific to transport.</param>
		/// <param name="options">Voice creation options.</param>
		/// <returns>Outgoing stream handler.</returns>
		template<typename T>
		inline LocalVoiceFramed<T>* VoiceClient::createLocalVoiceFramed(const VoiceInfo& voiceInfo, int frameSize, int channelId, const VoiceCreateOptions& options)
		{
			LocalVoiceFramed<T>* v;
			ALLOCATE(LocalVoiceFramed<T>, v, this, getNewVoiceId(), voiceInfo, channelId, frameSize, options);
			return tryAddLocalVoice(v) ? v : NULL;
		}

		/// <summary>
		/// Creates outgoing audio stream. Adds audio specific features (e.g. resampling, level meter) to processing pipeline and to returning stream handler.
		/// </summary>
		/// <typeparam name="T">Element type of audio array buffers.</typeparam>
		/// <param name="voiceInfo">Outgoing audio stream parameters. Set applicable fields to read them by encoder and by receiving client when voice created.</param>
		/// <param name="audioSourceDesc">Streaming audio source description.</param>
		/// <param name="channelId">Transport channel specific to transport.</param>
		/// <param name="options">Voice creation options.</param>
		/// <returns>Outgoing stream handler.</returns>
		/// <remarks>
		/// Sampling rates of audioSourceDesc voiceInfo may do not match. Automatic resampling will occur in this case.
		/// </remarks>
		template<typename T>
		inline LocalVoiceAudio<T>* VoiceClient::createLocalVoiceAudio(const VoiceInfo& voiceInfo, const IAudioDesc& audioSourceDesc, int channelId, const VoiceCreateOptions& options)
		{
			LocalVoiceAudio<T>* v;
			ALLOCATE(LocalVoiceAudio<T>, v, mLogger, this, getNewVoiceId(), voiceInfo, audioSourceDesc, channelId, options);
			return tryAddLocalVoice(v) ? v : NULL;
		}

		template<typename T>
		static void audioPusherSourceCallback(void* obj, const Buffer<T>& buf)
		{
			LocalVoiceAudio<T>* localVoice = static_cast<LocalVoiceAudio<T>*>(obj);
			localVoice->pushDataAsync(buf);
		}

		/// <summary>
		/// Creates outgoing audio stream.
		/// Adds audio specific features (e.g. resampling, level meter) to processing pipeline and to returning stream handler.
		/// </summary>
		/// <param name="voiceInfo">Outgoing audio stream parameters. Set applicable fields to read them by encoder and by receiving client when voice created.</param>
		/// <param name="source">Streaming audio source.</param>
		/// <param name="channelId">Transport channel specific to transport.</param>
		/// <param name="options">Voice creation options.</param>
		/// <returns>Outgoing stream handler.</returns>
		/// <remarks>
		/// Sampling rates of audioSourceDesc voiceInfo may do not match. Automatic resampling will occur in this case.
		/// </remarks>
		template <typename T>
		inline LocalVoiceAudio<T>* VoiceClient::createLocalVoiceAudioFromSource(const VoiceInfo& voiceInfo, IAudioPusher<T>* source, int channelId, const VoiceCreateOptions& options)
		{
			LocalVoiceAudio<T>* localVoice = createLocalVoiceAudio<T>(voiceInfo, *source, channelId, options);
			source->setCallback(localVoice, audioPusherSourceCallback);
			return localVoice;
		}
	}
}