/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/RaiseEventOptions.h"
#include "PhotonVoice-cpp/inc/Transport/PhotonTransport.h"
#include "PhotonVoice-cpp/inc/VoiceClient.h"

namespace ExitGames
{
	namespace Voice
	{
		/// <summary>
		/// Provides transport for VoiceClient based on given LoadBalancing Client
		/// Creates and maintains VoiceClient
		/// </summary>
		/// <remarks>
		class LoadBalancingTransport : public IVoiceTransport
		{
		public:
			LoadBalancingTransport(const Voice::Logger& logger, LoadBalancing::Client& lbClient, bool direct=false);

			virtual bool isChannelJoined(int channelId);
			virtual void sendVoiceInfo(const LocalVoice& voice, int channelId, bool targetMe, const int* targetPlayers, int numTargetPlayers);
			virtual void sendVoiceRemove(const LocalVoice& voice, int channelId, bool targetMe, const int* targetPlayers, int numTargetPlayers);
			virtual void sendFrame(Buffer<nByte> data, FrameFlags::Enum flags, unsigned short evNumber, nByte frNumber, nByte voiceId, int channelId, const SendFrameParams& par);
			virtual Common::JString channelIdStr(int channelId);
			virtual Common::JString playerIdStr(int playerId);
			virtual int getPayloadFragmentSize(const SendFrameParams& par);

			// Call this regularly (in LoadBalancing::Client::service)
			void service(void);
			// Call this on each incoming event (in LoadBalancing::Listener::customEventAction).
			void onCustomEvent(int playerNr, nByte eventCode, const Common::Object& eventContent);
			// Call this on each incoming direct event (in LoadBalancing::Listener::onDirectMessage).
			void onDirectMessage(const Common::Object& msg, int playerNr, bool relay);
			// Call this when LoadBalancing Client joins a room.
			void onJoinRoom(void);
			// Call this when LoadBalancing Client leaves a room or gets disconnected.
			void onLeaveRoom(void);
			// Call this when other player joins a room.
			void onPlayerJoinRoom(int playerNr);
			// Call this when other player leaves a room.
			void onPlayerLeaveRoom(int playerNr);

			virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;
		protected:
			const Voice::Logger& mLogger;
		private:
			bool buildEvOptFromTargets(bool targetMe, const int* targetPlayers, int numTargetPlayers, LoadBalancing::RaiseEventOptions& opt);
			void sendFrameRaiseEvents(void);
			void onVoiceFrameEvent(int playerNr, const Common::Object& eventContent);
			void onVoiceEvent(int playerNr, const Common::Object& eventContent);

			LoadBalancing::Client& mLBClient;
			PhotonTransportProtocol mProtocol;
			bool mDirect;

			// raising frame events in main thread
			struct SendFrameEventParams : public Common::ToString
			{
				using ToString::toString;

				virtual Common::JString& toString(Common::JString& retStr, bool withTypes=false) const;

				Buffer<nByte> mData;
				unsigned short mEvNumber;
				nByte mFrNumber;
				FrameFlags::Enum mFlags;
				nByte mVoiceId;
				bool mReliable;
				LoadBalancing::RaiseEventOptions mRaiseEventOptions;
			};

			Common::Helpers::SpinLockable<Common::JQueue<SendFrameEventParams> > mSendFrameRaiseEventQueue;
		};
	}
}