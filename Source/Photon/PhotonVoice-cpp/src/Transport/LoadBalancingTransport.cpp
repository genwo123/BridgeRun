/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "Photon-cpp/inc/Enums/ReceiverGroup.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/EventCode.h"
#include "PhotonVoice-cpp/inc/Transport/LoadBalancingTransport.h"

/** @file PhotonVoice-cpp/inc/Transport/LoadBalancingTransport.h */

namespace ExitGames
{
	namespace Voice
	{
		using namespace Common;
		using namespace Common::Helpers;
		using namespace Common::MemoryManagement;
		using namespace LoadBalancing;
		using namespace LoadBalancing::Internal;

		// Channel is used only by local voice (to specify Enet channel) and ignored by remote voices which are all on the same channel.
		// Join / leave per channel is not supported.
		// It's important to call onJoinAllChannels() instead of onJoinChannel() to avoid ignoring local non-0 channel voices.
		static const int REMOTE_VOICE_CHANNEL = 0;

		LoadBalancingTransport::LoadBalancingTransport(const Voice::Logger& logger, Client& lbClient, bool direct)
			: mLogger(logger)
			, mLBClient(lbClient)
			, mProtocol(mLogger)
			, mDirect(direct)
		{
		}

		bool LoadBalancingTransport::isChannelJoined(int /*channelId*/)
		{
			return mLBClient.getIsInRoom();
		}

		/// <summary>
		/// This method dispatches all available incoming commands and then sends this client's outgoing commands.
		/// Call this method regularly (2 to 20 times a second).
		/// </summary>
		void LoadBalancingTransport::service(void)
		{
			sendFrameRaiseEvents();
		}

		// Photon transport specific:
		// Empty TargetPlayers is the same as NULL: sending to all except the local client.
		// if TargetPlayers is not null and non-empty, InterestGroup and ReceiverGroup are ignored
		// if TargetPlayers is null or empty and InterestGroup is set, ReceiverGroup is ignored
		bool LoadBalancingTransport::buildEvOptFromTargets(bool targetMe, const int* targetPlayers, int numTargetPlayers, LoadBalancing::RaiseEventOptions& opt)
		{
			if(targetMe)
			{
				if(targetPlayers == NULL) // all others and me
					opt.setReceiverGroup(Lite::ReceiverGroup::ALL);
				else if(!numTargetPlayers) // only me
				{
					int n = mLBClient.getLocalPlayer().getNumber();
					opt.setTargetPlayers(&n, 1);
				}
				else
				{ // some others and me

					int* tp = allocateArray<int>(numTargetPlayers+1);

					MEMCPY(tp, targetPlayers, numTargetPlayers*sizeof(int));
					tp[numTargetPlayers] = mLBClient.getLocalPlayer().getNumber();
					opt.setTargetPlayers(tp, numTargetPlayers+1);

					deallocateArray(tp);
				}
			}
			else
			{
				if(targetPlayers != NULL && !numTargetPlayers) // Voice Core does not do such calls but better check again because LoadBalancing sends to all except the local client if the list is empty
				{
					EGLOG(DebugLevel::ERRORS, L"[PV] LoadBalancingTransport: no targets specified in Send* method call");

					return false;
				}
				if(targetPlayers) // if targetPlayers is NULL, numTargetPlayers is undefined, so avoid using it
					opt.setTargetPlayers(targetPlayers, numTargetPlayers);
			}

			return true;
		}

		void LoadBalancingTransport::sendVoiceInfo(const LocalVoice& voice, int channelId, bool targetMe, const int* targetPlayers, int numTargetPlayers)
		{
			Common::ValueObject<Common::Object*> content = mProtocol.buildVoicesInfo(voice, true);
			RaiseEventOptions opt;
			if(!buildEvOptFromTargets(targetMe, targetPlayers, numTargetPlayers, opt)) // empty targets check
				return;
			opt.setChannelID(static_cast<nByte>(channelId));
			mLBClient.opRaiseEvent(true, *content.getDataAddress(), *content.getSizes(), EventCode::VOICE_DATA, opt);
		}

		void LoadBalancingTransport::sendVoiceRemove(const LocalVoice& voice, int channelId, bool targetMe, const int* targetPlayers, int numTargetPlayers)
		{
			Common::ValueObject<Common::Object*> content = mProtocol.buildVoiceRemoveMessage(voice);

			RaiseEventOptions opt;
			if(!buildEvOptFromTargets(targetMe, targetPlayers, numTargetPlayers, opt)) // empty targets check
				return;
			opt.setChannelID(static_cast<nByte>(channelId));
			mLBClient.opRaiseEvent(true, *content.getDataAddress(), *content.getSizes(), EventCode::VOICE_DATA, opt);
		}

		// Raise event from decoding thread. Requires LoadBalancing client calls synchronization

		//void LoadBalancingTransport::SendFrame(Buffer<nByte> data, nByte evNumber, nByte voiceId, int channelId, const LocalVoice* localVoice)
		//{
		//	Object content[3];
		//	content[0] = ValueObject<nByte>(voiceId);
		//	content[1] = ValueObject<nByte>(evNumber);
		//	content[2] = ValueObject<nByte*>(data.getArray(), data.getSize());

		//	RaiseEventOptions opt;
		//	opt.setChannelID(channelId);
		//	//Encrypt = localVoice.Encrypt
		//	if(localVoice->getDebugEchoMode())
		//		opt.setReceiverGroup(Lite::ReceiverGroup::ALL);
		//	opt.setInterestGroup(localVoice->getInterestGroup());

		//	lbClient->opRaiseEvent(localVoice->getReliable(), content, 3, VoiceEventCode::GetCode(channelId), opt);
		//	lbClient->sendOutgoingCommands();
		//}

		// Push outgoing frame event to the queue
		void LoadBalancingTransport::sendFrame(Buffer<nByte> data, FrameFlags::Enum flags, unsigned short evNumber, nByte frNumber, nByte voiceId, int channelId, const SendFrameParams& par)
		{
			RaiseEventOptions opt;
			if(!buildEvOptFromTargets(par.getTargetMe(), par.getTargetPlayers(), par.getNumTargetPlayers(), opt)) // empty targets check
				return;
			SendFrameEventParams p;
			p.mData = data;
			p.mFlags = flags;
			p.mEvNumber = evNumber;
			p.mFrNumber = frNumber;
			p.mVoiceId = voiceId;
			opt.setChannelID(static_cast<nByte>(channelId));
			opt.setInterestGroup(par.getInterestGroup());
			opt.setEncrypt(par.getEncrypt());
			p.mReliable = par.getReliable();
			p.mRaiseEventOptions = opt;

			Lockguard lock(mSendFrameRaiseEventQueue);
			mSendFrameRaiseEventQueue.enqueue(p);
		}

		int LoadBalancingTransport::getPayloadFragmentSize(const SendFrameParams& par)
		{
			return mProtocol.getPayloadFragmentSize(par);
		}

		// ... then send frame event in main thread
		void LoadBalancingTransport::sendFrameRaiseEvents(void)
		{
			bool empty = false;
			while(!empty) // Dequeue and process while the queue is not empty
			{
				SendFrameEventParams p;
				{
					Lockguard lock(mSendFrameRaiseEventQueue);
					if(mSendFrameRaiseEventQueue.getSize() > 0)
						p = mSendFrameRaiseEventQueue.dequeue();
					else
						empty = true;
				}
				if(!empty)
				{
					int size;
					nByte* pContent = mProtocol.buildFrameEvent(p.mData, p.mVoiceId, p.mEvNumber, p.mFrNumber, p.mFlags, size);

					if(mDirect)
						mLBClient.sendDirect(pContent, p.mData.getSize()+size, SendDirectOptions(p.mRaiseEventOptions).setFallbackRelay(true));
					else
					{
						//Encrypt = localVoice.Encrypt
						mLBClient.opRaiseEvent(p.mReliable, pContent, p.mData.getSize()+size, EventCode::VOICE_FRAME_DATA, p.mRaiseEventOptions);
						mLBClient.sendOutgoingCommands();
					}

					deallocateArray(pContent);
				}
			}
		}

		JString LoadBalancingTransport::channelIdStr(int /*channelId*/)
		{
			return JString();
		}

		JString LoadBalancingTransport::playerIdStr(int /*playerId*/)
		{
			return JString();
		}

		void LoadBalancingTransport::onCustomEvent(int playerNr, nByte eventCode, const Object& eventContent)
		{
			if(eventCode == EventCode::VOICE_FRAME_DATA)
				onVoiceFrameEvent(playerNr, eventContent);
			else if(eventCode == EventCode::VOICE_DATA)
				onVoiceEvent(playerNr, eventContent);
		}

		void LoadBalancingTransport::onDirectMessage(const Object& msg, int playerNr, bool /*relay*/)
		{
			onVoiceFrameEvent(playerNr, msg);
		}

		void LoadBalancingTransport::onVoiceFrameEvent(int playerNr, const Object& eventContent)
		{
			mProtocol.onVoiceFrameEvent(*mpVoiceClient, eventContent, REMOTE_VOICE_CHANNEL, playerNr, mLBClient.getLocalPlayer().getNumber());
		}

		void LoadBalancingTransport::onVoiceEvent(int playerNr, const Object& eventContent)
		{
			mProtocol.onVoiceEvent(*mpVoiceClient, eventContent, REMOTE_VOICE_CHANNEL, playerNr, mLBClient.getLocalPlayer().getNumber());
		}

		void LoadBalancingTransport::onJoinRoom(void)
		{
			mpVoiceClient->onJoinAllChannels();
		}

		void LoadBalancingTransport::onLeaveRoom(void)
		{
			mpVoiceClient->onLeaveAllChannels();
		}

		void LoadBalancingTransport::onPlayerJoinRoom(int playerNr)
		{
			if(playerNr != mLBClient.getLocalPlayer().getNumber())
				mpVoiceClient->onPlayerJoin(playerNr);
		}

		void LoadBalancingTransport::onPlayerLeaveRoom(int playerNr)
		{
			if(playerNr == mLBClient.getLocalPlayer().getNumber())
				mpVoiceClient->onLeaveAllChannels();
			else
				mpVoiceClient->onPlayerLeave(playerNr);
		}

		JString& LoadBalancingTransport::toString(JString& retStr, bool withTypes) const
		{
			return retStr
				+= JString(L"{")
				+ L" direct=" + (mDirect?L"true":L"false")
				+ L" sendFrameRaiseEventQueue=" + mSendFrameRaiseEventQueue.toString(withTypes)
				+ L"}";
		}

		JString& LoadBalancingTransport::SendFrameEventParams::toString(JString& retStr, bool withTypes) const
		{
			return retStr
				+= JString(L"{")
				+ L"data=" + mData.toString(withTypes)
				+ L" evNumber=" + mEvNumber
				+ L" frNumber=" + mFrNumber
				+ L" flags=" + static_cast<nByte>(mFlags)
				+ L" voiceId=" + mVoiceId
				+ L" reliable=" + (mReliable?L"true":L"false")
				+ L" raiseEventOptions=" +  mRaiseEventOptions.toString()
				+ L"}";
		}
	}
}