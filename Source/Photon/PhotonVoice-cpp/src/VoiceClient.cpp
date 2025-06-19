/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/VoiceClient.h"

/** @file PhotonVoice-cpp/inc/VoiceClient.h */

namespace ExitGames
{
	namespace Voice
	{
		using namespace Common;
		using namespace Common::Helpers;
		using namespace Common::MemoryManagement;

		SendFrameParams::SendFrameParams(bool targetMe, const int* targetPlayers, int numTargetPlayers, nByte interestGroup, bool reliable, bool encrypt)
			: mTargetMe(targetMe)
			, mpTargetPlayers(targetPlayers)
			, mNumTargetPlayers(numTargetPlayers)
			, mInterestGroup(interestGroup)
			, mReliable(reliable)
			, mEncrypt(encrypt)
		{
		}

		bool SendFrameParams::getTargetMe(void) const
		{
			return mTargetMe;
		}

		const int* SendFrameParams::getTargetPlayers(void) const
		{
			return mpTargetPlayers;
		}

		int SendFrameParams::getNumTargetPlayers(void) const
		{
			return mNumTargetPlayers;
		}

		nByte SendFrameParams::getInterestGroup(void) const
		{
			return mInterestGroup;
		}

		bool SendFrameParams::getReliable(void) const
		{
			return mReliable;
		}

		bool SendFrameParams::getEncrypt(void) const
		{
			return mEncrypt;
		}



		/// <summary>True if threading is enabled.</summary>
		bool VoiceClient::getThreadingEnabled(void) const
		{
#if defined _EG_EMSCRIPTEN_PLATFORM && !defined __EMSCRIPTEN_PTHREADS__
			return false;
#else
			return true;
#endif
		}

		void VoiceClient::setRemoteVoiceDelayFrames(Codec::Enum codec, int delayFrames)
		{
			mRemoteVoiceDelayFramesPerCodec.put(codec, delayFrames);
			for(unsigned int i=0; i<mRemoteVoices.getSize(); ++i)
			{
				JDictionary<nByte, SharedPointer<RemoteVoice> >& voices = mRemoteVoices[i];
				for(unsigned int j=0; j<voices.getSize(); ++j)
					voices[j]->setDelayFrames(delayFrames);
			}
		}

		/// <summary>Lost frames counter.</summary>
		int VoiceClient::getFramesLost(void) const
		{
			return mFramesLost;
		}

		/// <summary>Received frames counter.</summary>
		int VoiceClient::getFramesReceived(void) const
		{
			return mFramesReceived;
		}

		/// <summary>Sent frames counter.</summary>
		int VoiceClient::getFramesSent(void) const
		{
			int x = 0;
			mLocalVoicesLock.lock();
			for(unsigned int i=0; i<mLocalVoices.getSize(); ++i)
				x += mLocalVoices[i]->getFramesSent();
			mLocalVoicesLock.unlock();
			return x;
		}

		/// <summary>Sent frames bytes counter.</summary>
		int VoiceClient::getFramesSentBytes(void) const
		{
			int x = 0;
			mLocalVoicesLock.lock();
			for(unsigned int i = 0; i<mLocalVoices.getSize(); ++i)
				x += mLocalVoices[i]->getFramesSentBytes();
			mLocalVoicesLock.unlock();
			return x;
		}

		/// <summary>Average time required voice packet to return to sender.</summary>
		int VoiceClient::getRoundTripTime(void) const
		{
			return mRoundTripTime;
		}

		/// <summary>Average round trip time variation.</summary>
		int VoiceClient::getRoundTripTimeVariance(void) const
		{
			return mRoundTripTimeVariance;
		}

		/// <summary>Do not log warning when duplicate info received.</summary>
		bool VoiceClient::getSuppressInfoDuplicateWarning(void) const
		{
			return mSuppressInfoDuplicateWarning;
		}

		void VoiceClient::setSuppressInfoDuplicateWarning(bool yes)
		{
			mSuppressInfoDuplicateWarning = yes;
		}

		/// <summary>
		/// Register a method to be called when remote voice info arrived (after join or new new remote voice creation).
		/// </summary>
		void VoiceClient::setOnRemoteVoiceInfoAction(void* opaque, VoiceClient::RemoteVoiceInfoCallback* callback)
		{
			mpOnRemoteVoiceInfoAction = callback;
			mpOnRemoteVoiceInfoActionOpaque = opaque;
		}

		/// <summary>Lost frames simulation ratio.</summary>
		int VoiceClient::getDebugLostPercent(void) const
		{
			return mDebugLostPercent;
		}

		void VoiceClient::setDebugLostPercent(int x)
		{
			mDebugLostPercent = x;
		}

		void VoiceClient::forEachLocalVoice(void* opaque, void(*op)(void*, const LocalVoice&)) const
		{
			mLocalVoicesLock.lock();
			for(unsigned int i=0; i<mLocalVoices.getSize(); ++i)
				op(opaque, *mLocalVoices[i]);
			mLocalVoicesLock.unlock();
		}

		/// <summary>Returns copies of all local voices for given channel.</summary>
		void VoiceClient::forEachLocalVoiceInChannel(int channelId, void* opaque, void(*op)(void*, const LocalVoice&)) const
		{
			mLocalVoicesLock.lock();
			const JVector<Common::Helpers::SharedPointer<LocalVoice> >* channelVoices = mLocalVoicesPerChannel.getValue(channelId);
			mLocalVoicesLock.unlock();
			if(channelVoices)
				for(unsigned int i=0; i<channelVoices->getSize(); ++i)
					op(opaque, *(channelVoices->getElementAt(i)));
		}

		/// <summary>Iterates through all remote voices infos.</summary>
		JVector<RemoteVoiceInfo> VoiceClient::getRemoteVoiceInfos() const
		{
			Lockguard lock(mRemoteVoicesLock);

			JVector<RemoteVoiceInfo> res;
			for(unsigned int i=0; i<mRemoteVoices.getSize(); ++i)
			{
				int playerId = mRemoteVoices.getKeys()[i];
				const JDictionary<nByte, SharedPointer<RemoteVoice> >& voices = mRemoteVoices[i];
				for(unsigned int j=0; j<voices.getSize(); ++j)
				{
					nByte voiceId = voices.getKeys()[j];
					const RemoteVoice* voice = voices[j];
					res.addElement(RemoteVoiceInfo(voice->getChannelId(), playerId, voiceId, voice->getInfo()));
				}
			}
			return res;
		}


		VoiceClient::CreateOptions::CreateOptions()
			: mVoiceIDMin(DEFAULT_VOICE_ID_MIN)
			, mVoiceIDMax(DEFAULT_VOICE_ID_MAX)
		{
		}

		nByte VoiceClient::CreateOptions::getVoiceIDMin(void) const
		{
			return mVoiceIDMin;
		}

		VoiceClient::CreateOptions& VoiceClient::CreateOptions::setVoiceIDMin(nByte v)
		{
			mVoiceIDMin = v;
			return *this;
		}

		nByte VoiceClient::CreateOptions::getVoiceIDMax(void) const
		{
			return mVoiceIDMax;
		}

		VoiceClient::CreateOptions& VoiceClient::CreateOptions::setVoiceIDMax(nByte v)
		{
			mVoiceIDMax = v;
			return *this;
		};


		/// <summary>Creates VoiceClient instance</summary>
		VoiceClient::VoiceClient(const Voice::Logger& logger, IVoiceTransport* transport, const CreateOptions& options)
			: mpTransport(transport)
			, mLogger(logger)
			, mVoiceIDMin(options.getVoiceIDMin())
			, mVoiceIDMax(options.getVoiceIDMax())
			, mVoiceIdLast(options.getVoiceIDMax())
#if defined PV_DEBUG_ECHO_RTT_MEASURE
			, mPrevRtt(0)
#endif
			, mFramesLost(0)
			, mFramesFragPart(0)
			, mFramesRecovered(0)
			, mFramesLate(0)
			, mFramesReceived(0)
			, mFramesReceivedFEC(0)
			, mFramesTryFEC(0)
			, mFramesReceivedFragments(0)
			, mFramesReceivedFragmented(0)
			, mFramesSent(0)
			, mFramesSentBytes(0)
			, mRoundTripTime(0)
			, mRoundTripTimeVariance(0)
			, mSuppressInfoDuplicateWarning(false)
			, mpOnRemoteVoiceInfoAction(NULL)
			, mpOnRemoteVoiceInfoActionOpaque(NULL)
			, mDebugLostPercent(0)
		{
			transport->mpVoiceClient = this;
		}

		/// <summary>
		/// This method dispatches all available incoming commands and then sends this client's outgoing commands.
		/// Call this method regularly (2..20 times a second).
		/// </summary>
		void VoiceClient::service()
		{
			Lockguard lock(mLocalVoicesLock);
			for(unsigned int i=0; i<mLocalVoices.getSize(); ++i)
				mLocalVoices[i]->service();
		}

		LocalVoice* VoiceClient::tryAddLocalVoice(LocalVoice* pLocalVoice)
		{
			Lockguard lock(mLocalVoicesLock);
			if(pLocalVoice->getId())
			{
				pLocalVoice->setupEncoder();
				addLocalVoice(pLocalVoice->getId(), pLocalVoice->getChannelId(), pLocalVoice);
				EGLOG(DebugLevel::INFO, pLocalVoice->getLogPrefix() + L" added enc: " + pLocalVoice->getInfo().toString());
				return pLocalVoice;
			}
			else
			{
				deallocate(pLocalVoice);
				return NULL;
			}
		}

#if EG_PHOTON_VOICE_VIDEO_ENABLE
		/// <summary>
		/// Creates outgoing video stream consuming sequence of image buffers.
		/// </summary>
		/// <param name="voiceInfo">Outgoing stream parameters. Set applicable fields to read them by encoder and by receiving client when voice created.</param>
		/// <param name="channelId">Transport channel specific to transport.</param>
		/// <param name="encoder">Encoder compressing video data. Set to null to use default VP8 implementation.</param>
		/// <returns>Outgoing stream handler.</returns>
		//inline LocalVoiceVideo* VoiceClient::CreateLocalVoiceVideo(VoiceInfo voiceInfo, int channelId, SharedPointer<IEncoder> encoder)
		//{
		//	return static_cast<LocalVoiceVideo*>(createLocalVoice(voiceInfo, channelId, (vId, chId) = > new LocalVoiceVideo(this, encoder, vId, voiceInfo, chId)));
		//}
#endif

		nByte VoiceClient::idInc(nByte id)
		{
			return id == mVoiceIDMax ? mVoiceIDMin : (nByte)(id + 1);
		}

		nByte VoiceClient::getNewVoiceId(void)
		{
			bool used[256] = {}; // sets all to false;
			for(unsigned int i=0; i < mLocalVoices.getSize(); ++i)
				used[mLocalVoices[i]->mId] = true;

			for(nByte id=idInc(mVoiceIdLast); id!=mVoiceIdLast; id=idInc(id))
			{
				if(!used[id])
				{
					mVoiceIdLast = id;
					return id;
				}
			}
			return 0;
		}

		void VoiceClient::addLocalVoice(nByte newId, int channelId, LocalVoice* pV)
		{
			SharedPointer<LocalVoice> spVoice = pV;
			mLocalVoices.put(newId, spVoice);

			JVector<SharedPointer<LocalVoice> >* channelVoices = mLocalVoicesPerChannel.getValue(channelId);
			if(!channelVoices)
			{
				JVector<SharedPointer<LocalVoice> > vec(1);
				vec.addElement(spVoice);
				mLocalVoicesPerChannel.put(channelId, vec);
			}
			else
				channelVoices->addElement(spVoice);

			if(mpTransport->isChannelJoined(channelId))
				pV->onJoinChannel();
		}

		/// <summary>
		/// Removes local voice (outgoing data stream).
		/// <param name="voice">Handler of outgoing stream to be removed.</param>
		/// </summary>
		void VoiceClient::removeLocalVoice(LocalVoice& voice)
		{
			mLocalVoicesLock.lock();

			SharedPointer<LocalVoice> spVoice = *mLocalVoices.getValue(voice.mId);
			mLocalVoices.remove(voice.mId);

			JVector<SharedPointer<LocalVoice> >* pChannelVoices = mLocalVoicesPerChannel.getValue(voice.mChannelId);
			if(pChannelVoices)
			{
				pChannelVoices->removeElement(spVoice); // we can't pass the raw LocalVoice* here, as that would mean creating a temporary SharedPointer instance which would not share ownership with the existing SharedPointer instances

				mLocalVoicesLock.unlock();

				if(mpTransport->isChannelJoined(voice.mChannelId))
					voice.onLeaveChannel();

				EGLOG(DebugLevel::INFO, voice.getLogPrefix() + L" removed");
			}
			else
				mLocalVoicesLock.unlock();
		}

		IVoiceTransport* VoiceClient::getTransport(void) const
		{
			return mpTransport;
		}

		void VoiceClient::clearRemoteVoices(void)
		{
			Lockguard lock(mRemoteVoicesLock);

			mRemoteVoices.removeAllElements();
			EGLOG(DebugLevel::INFO, L"[PV] Remote voices cleared");
		}

		void VoiceClient::clearRemoteVoicesInChannel(int channelId)
		{
			Lockguard lock(mRemoteVoicesLock);

			JVector<int> playerToRemove;
			for(unsigned int i=0; i<mRemoteVoices.getSize(); ++i)
			{
				JDictionary<nByte, SharedPointer<RemoteVoice> >& voices = mRemoteVoices[i];
				JVector<nByte> voicesToRemove;
				for(unsigned int j=0; j<voices.getSize(); ++j)
					if(voices[j]->getChannelId() == channelId)
						voicesToRemove.addElement(voices.getKeys()[j]);
				for(unsigned int j=0; j<voicesToRemove.getSize(); ++j)
					voices.remove(voicesToRemove[j]);
				if(!voices.getSize())
					playerToRemove.addElement(mRemoteVoices.getKeys()[i]);
			}
			for(unsigned int i=0; i<mRemoteVoices.getSize(); ++i)
				mRemoteVoices.remove(playerToRemove[i]);

			EGLOG(DebugLevel::INFO, L"[PV] Remote voices for channel " + channelStr(channelId) + L" cleared");
		}

		void VoiceClient::clearRemoteVoicesForPlayer(bool allChannels, int channelId, int playerId)
		{
			Lockguard lock(mRemoteVoicesLock);

			JDictionary<nByte, SharedPointer<RemoteVoice> >* voices = mRemoteVoices.getValue(playerId);
			if(voices)
			{
				for(unsigned int i=0; i<voices->getSize(); ++i)
				{
					const RemoteVoice* v = (*voices)[i];
					JVector<nByte> voicesToRemove;
					if(allChannels || v->getChannelId() == channelId)
						voicesToRemove.addElement(voices->getKeys()[i]);
					for(unsigned int j=0; j<voicesToRemove.getSize(); ++j)
						voices->remove(voicesToRemove[j]);
				}
				mRemoteVoices.remove(playerId);
			}
			EGLOG(DebugLevel::INFO, JString(L"[PV] Remote voices in ") + (allChannels ? JString(L"all channels") : L"channel" + channelStr(channelId)) + L" for player " + playerId + L" cleared");
		}

		void VoiceClient::onJoinChannel(int channelId)
		{
			// my join, broadcast
			mLocalVoicesLock.lock();

			JVector<SharedPointer<LocalVoice> >* channelVoices = mLocalVoicesPerChannel.getValue(channelId);
			if(channelVoices)
			{
				// const copy of channel voices
				JVector<const LocalVoice*> voices(channelVoices->getSize());
				for(unsigned int i=0; i<channelVoices->getSize(); ++i)
					(*channelVoices)[i]->onJoinChannel();
			}

			mLocalVoicesLock.unlock();
		}

		void VoiceClient::onJoinAllChannels(void)
		{
			// my join, broadcast
			mLocalVoicesLock.lock();

			for(unsigned int i=0; i<mLocalVoices.getSize(); ++i)
				mLocalVoices[i]->onJoinChannel();

			mLocalVoicesLock.unlock();
		}

		void VoiceClient::onLeaveChannel(int channelId)
		{
			clearRemoteVoicesInChannel(channelId);
		}

		void VoiceClient::onLeaveAllChannels(void)
		{
			clearRemoteVoices();
		}

		void VoiceClient::onPlayerJoin(int channelId, int playerId)
		{
			mLocalVoicesLock.lock();

			JVector<SharedPointer<LocalVoice> >* channelVoices = mLocalVoicesPerChannel.getValue(channelId);
			if(channelVoices)
			{
				for(unsigned int i=0; i<channelVoices->getSize(); ++i)
					(*channelVoices)[i]->onPlayerJoin(playerId);
			}

			mLocalVoicesLock.unlock();
		}

		void VoiceClient::onPlayerJoin(int playerId)
		{
			mLocalVoicesLock.lock();

			for(unsigned int i=0; i<mLocalVoices.getSize(); ++i)
				mLocalVoices[i]->onPlayerJoin(playerId);

			mLocalVoicesLock.unlock();
		}

		void VoiceClient::onPlayerLeave(int channelId, int playerId)
		{
			clearRemoteVoicesForPlayer(false, channelId, playerId);
		}

		void VoiceClient::onPlayerLeave(int playerId)
		{
			clearRemoteVoicesForPlayer(true, 0, playerId);
		}

		void VoiceClient::onVoiceInfo(int channelId, int playerId, nByte voiceId, int eventBufferSize, const VoiceInfo& info)
		{
			Lockguard lock(mRemoteVoicesLock);

			JDictionary<nByte, SharedPointer<RemoteVoice> >* voices = mRemoteVoices.getValue(playerId);
			if(!voices)
			{
				mRemoteVoices.put(playerId, JDictionary<nByte, SharedPointer<RemoteVoice> >());
				voices = mRemoteVoices.getValue(playerId);
			}
			if(!voices->getValue(voiceId))
			{
				JString voiceStr(L" ch#" + channelStr(channelId) + L" p#" + playerStr(playerId) + L" v#" + voiceId);
				EGLOG(DebugLevel::INFO, L"[PV]" + voiceStr + L" Info received: " + info.toString() + L" evbs=" + eventBufferSize);

				RemoteVoiceOptions options(mLogger, JString(L"[PV] Remote ") + info.getCodec() + voiceStr, info);
				// create default decoder
				// may be overwritten in OnRemoteVoiceInfoAction call
				if(mpOnRemoteVoiceInfoAction)
					mpOnRemoteVoiceInfoAction(mpOnRemoteVoiceInfoActionOpaque, channelId, playerId, voiceId, info, options);
				RemoteVoice* pRemoteVoice;
				voices->put(voiceId, ALLOCATE(RemoteVoice, pRemoteVoice, mLogger, this, options, channelId, playerId, voiceId, info, eventBufferSize));

				if(mRemoteVoiceDelayFramesPerCodec.contains(info.getCodec()))
					pRemoteVoice->setDelayFrames(*mRemoteVoiceDelayFramesPerCodec.getValue(info.getCodec()));
			}
			else if(!mSuppressInfoDuplicateWarning)
				EGLOG(DebugLevel::WARNINGS, JString(L"[PV] Info duplicate for voice #") + voiceId + L" of player " + playerStr(playerId) + " at channel " + channelStr(channelId));
		}

		void VoiceClient::onVoiceRemove(int channelId, int playerId, const JVector<nByte>& voiceIds)
		{
			Lockguard lock(mRemoteVoicesLock);

			JDictionary<nByte, SharedPointer<RemoteVoice> >* voices = mRemoteVoices.getValue(playerId);
			if(voices)
			{
				for(unsigned int i=0; i<voiceIds.getSize(); ++i)
				{
					nByte voiceId = voiceIds[i];
					const SharedPointer<RemoteVoice>* rv = voices->getValue(voiceId);
					if(rv)
					{
						voices->remove(voiceId);
						EGLOG(DebugLevel::INFO, JString(L"[PV] Remote voice #") + voiceId + L" of player " + playerStr(playerId) + L" at channel " + channelStr(channelId) + L" removed");
					}
					else
						EGLOG(DebugLevel::WARNINGS, JString(L"[PV] Remote voice #") + voiceId + L" of player " + playerStr(playerId) + " at channel " + channelStr(channelId) + " not found when trying to remove");
				}
			}
			else
				EGLOG(DebugLevel::WARNINGS, JString("L[PV] Remote voice list of player ") + playerStr(playerId) + " at channel " + channelStr(channelId) + " not found when trying to remove voice(s)");
		}

		void VoiceClient::onFrame(int channelId, int playerId, nByte voiceId, unsigned short evNumber, const FrameBuffer& receivedBytes, bool isLocalPlayer)
		{
#if defined PV_DEBUG_ECHO_RTT_MEASURE
			if(isLocalPlayer)
			{
				SharedPointer<LocalVoice>* pspV;
				{
					Lockguard lock(mLocalVoicesLock);
					// rtt measurement in debug echo mode
					pspV = mLocalVoices.getValue(voiceId);
				}
				if(pspV)
				{
					const int* pTimestamp;
					{
						Lockguard lock((*pspV)->mEventTimestamps);
						pTimestamp = (*pspV)->mEventTimestamps.getValue(evNumber);
					}
					if(pTimestamp)
					{
						int sendTime = *pTimestamp;
						int rtt = GETTIMEMS() - sendTime;
						int rttvar = rtt - mPrevRtt;
						mPrevRtt = rtt;
						if(rttvar < 0) rttvar = -rttvar;
						mRoundTripTimeVariance = (rttvar + getRoundTripTimeVariance()*19)/20;
						mRoundTripTime = (rtt + getRoundTripTime()*19)/20;
					}
				}
			}
#endif
			if(mDebugLostPercent > 0 && (rand()*100/RAND_MAX) < mDebugLostPercent)
			{
				EGLOG(DebugLevel::WARNINGS, L"[PV] Debug Lost Sim: 1 packet dropped");
				return;
			}

			mFramesReceived++;

			mRemoteVoicesLock.lock();
			JDictionary<nByte, SharedPointer<RemoteVoice> >* voices = mRemoteVoices.getValue(playerId);
			if(voices)
			{
				SharedPointer<RemoteVoice>* v = voices->getValue(voiceId);
				mRemoteVoicesLock.unlock();
				if(v)
					(**v).receiveBytes(receivedBytes, evNumber);
				else
					EGLOG(DebugLevel::WARNINGS, JString(L"[PV] Frame event for not initialized voice #") + voiceId + L" of player " + playerStr(playerId) + L" at channel " + channelStr(channelId));
			}
			else
			{
				mRemoteVoicesLock.unlock();
				EGLOG(DebugLevel::WARNINGS, JString(L"[PV] Frame event for voice #") + voiceId + L" of not initialized player " + playerStr(playerId) + L" at channel " + channelStr(channelId));
			}
		}

		JString VoiceClient::channelStr(int channelId)
		{
			const JString& str = mpTransport->channelIdStr(channelId);
			if(str.length())
				return JString() + channelId + L"(" + str + L")";
			else
				return JString() + channelId;
		}

		JString VoiceClient::playerStr(int playerId)
		{
			const JString& str = mpTransport->playerIdStr(playerId);

			if(str.length())
				return JString() + playerId + "(" + str + ")";
			else
				return JString() + playerId;
		}

		VoiceClient::~VoiceClient(void)
		{
			Lockguard lock(mLocalVoicesLock);
			Lockguard lock2(mRemoteVoicesLock);
		}
	}
}