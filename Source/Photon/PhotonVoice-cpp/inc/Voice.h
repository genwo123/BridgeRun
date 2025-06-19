/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "Common-cpp/inc/Logger.h"
#include "PhotonVoice-cpp/inc/VoiceCodec.h"
#include "PhotonVoice-cpp/inc/Logger.h"

namespace ExitGames
{
	namespace Voice
	{
		class IEncoder;
		class IDecoder;
		class VoiceClient;
		class SendFrameParams;

		class VoiceCreateOptions
		{
		public:
			VoiceCreateOptions(void);
			VoiceCreateOptions& setEncoder(const Common::Helpers::SharedPointer<IEncoder>&);
			VoiceCreateOptions& setInterestGroup(nByte);
			VoiceCreateOptions& setTargetPlayers(const int* targetPlayers, int numTargetPlayers);
			VoiceCreateOptions& setDebugEchoMode(bool);
			VoiceCreateOptions& setReliable(bool);
			VoiceCreateOptions& setEncrypt(bool);
			VoiceCreateOptions& setFragment(bool);
			VoiceCreateOptions& setFEC(int);
			VoiceCreateOptions& setEventBufSize(int);
		private:
			Common::Helpers::SharedPointer<IEncoder> mspEncoder;
			nByte mInterestGroup;
			bool mTargetPlayersEnabled;
			Common::JVector<int> mTargetPlayers;
			bool mDebugEchoMode;
			bool mReliable;
			bool mEncrypt;
			bool mFragment;
			int mFEC;
			int mEventBufSize;

			friend class LocalVoice;
		};

		/// <summary>
		/// Represents outgoing data stream.
		/// </summary>
		class LocalVoice : public Common::ToString
		{
		public:
			const int NO_TRANSMIT_TIMEOUT_MS = 100;

			using ToString::toString;

			virtual ~LocalVoice(void);

			void removeSelf(void);

			nByte getId(void) const;
			int getChannelId(void) const;
			const VoiceInfo& getInfo(void) const;
			int getEventBufferSize(void) const;
			bool getReliable(void) const;
			void setReliable(bool reliable);
			bool getEncrypt(void) const;
			void setEncrypt(bool encrypt);
			bool getFragment(void) const;
			void setFragment(bool fragment);
			int getFEC(void) const;
			void setFEC(int fec);
			nByte getInterestGroup(void) const;
			void setInterestGroup(nByte group);
			bool getTransmitEnabled(void) const;
			void setTransmitEnabled(bool transmitEnabled);
			bool getIsCurrentlyTransmitting(void) const;
			int getFramesSent(void) const;
			int getFramesSentFragmented(void) const;
			int getFramesSentFragments(void) const;
			int getFramesSentBytes(void) const;

			bool getDebugEchoMode(void) const;
			void setDebugEchoMode(bool debugEchoMode);

			void setTargetPlayersDisabled(void);
			void setTargetPlayers(const int* targetPlayers, int numTargetPlayers);

			Common::JString getName(void) const;
			Common::JString getLogPrefix(void) const;

			virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;

			static const int DATA_POOL_CAPACITY = 50; // TODO: may depend on data type and properties, set for average audio stream
		protected:
			LocalVoice(const Voice::Logger& logger); // for dummy voices
			LocalVoice(const Voice::Logger& logger, VoiceClient* pVoiceClient, nByte id, const VoiceInfo& voiceInfo, int channelId, const VoiceCreateOptions& options);

			virtual void service(void);
			void onJoinChannel(void);
			void onLeaveChannel(void);
			void onPlayerJoin(int playerId);

			void sendFrame(const Buffer<nByte>& compressed, FrameFlags::Enum flags);
			void setupEncoder(void); // setup moved out of constructor because it calls virtual createDefaultEncoder
			virtual IEncoder* createDefaultEncoder(const VoiceInfo& info);

			const Voice::Logger& mLogger;
			nByte mId;
			int mChannelId;
			int EV_BUF_SIZE;
			unsigned short mEvNumber;
			VoiceInfo mInfo;
			bool mDebugEchoMode;
			bool mTargetPlayersEnabled;
			Common::JVector<int> mTargetPlayers;
			VoiceClient* mpVoiceClient;
			bool mThreadingEnabled;
			Buffer<nByte> mConfigFrame;

			Common::Helpers::SharedPointer<IEncoder> mspEncoder;
#if defined PV_DEBUG_ECHO_RTT_MEASURE
			Common::Helpers::SpinLockable<Common::JDictionary<unsigned short, int> > mEventTimestamps;
#endif
		private:
			LocalVoice(const LocalVoice&);
			LocalVoice& operator=(const LocalVoice&); // non-copyable

			bool isJoined(void) const;
			static bool targetExits(bool targetMe, const Common::JVector<int>* targetPlayers);
			Common::JString getTargetStr(bool targetMe, const Common::JVector<int>* targetPlayers) const;
			void sendVoiceInfoAndConfigFrame(bool targetMe, const Common::JVector<int>* targetPlayers);
			void sendVoiceRemove(bool targetMe, const Common::JVector<int>* targetPlayers);
			void sendFrameParts(const Buffer<nByte>& buffer, FrameFlags::Enum flags, bool targetMe, const Common::JVector<int>* targetPlayers, nByte interestGroup, bool reliable);
			void sendFrame0(const Buffer<nByte>& buffer, FrameFlags::Enum flags, bool targetMe, const Common::JVector<int>* targetPlayers, nByte interestGroup, bool reliable);
			void sendFrameEvent(const Buffer<nByte>& data, FrameFlags::Enum flags, const SendFrameParams& sendFramePar);
			void resetFEC();

			static void sendFrame(void* opaque, const Buffer<nByte>& buffer, FrameFlags::Enum flags);

			nByte mInterestGroup;
			bool mTransmitEnabled;
			int mLastTransmitTime;
			unsigned int mFramesSent;
			unsigned int mFramesSentFragmented;
			unsigned int mFramesSentFragments;
			unsigned int mFramesSentBytes;
			bool mReliable;
			bool mEncrypt;
			bool mFragment;
			int mFEC;
			int mNoTransmitCnt;

			int FEC_INFO_SIZE(void) const; // write additional data required for recovery at the buffer end
			Buffer<nByte> mFecBuffer;
			nByte mFecFlags;
			nByte mFecFrameNumber;
			int mFecTotSize;
			int mFecMaxSize; // should be always updated synchronously with FEC data because resetFEC() clears exactly fecMaxSize + FEC_INFO_SIZE bytes
			nByte mFecCnt;

			friend class VoiceClient;
		};
	}
}