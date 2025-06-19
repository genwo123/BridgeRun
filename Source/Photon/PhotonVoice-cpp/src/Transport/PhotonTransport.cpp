/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "PhotonVoice-cpp/inc/Transport/PhotonTransport.h"

/** @file PhotonVoice-cpp/inc/Transport/PhotonTransport.h */

namespace ExitGames
{
	namespace Voice
	{
		using namespace Common;
		using namespace Common::Helpers;
		using namespace Common::MemoryManagement;

		namespace EventSubcode
		{
			static const nByte VOICE_INFO = 1;
			static const nByte VOICE_REMOVE = 2;
			//static const nByte FRAME = 3;
		}

		namespace EventParam
		{
			static const nByte VOICE_ID = 1;
			static const nByte SAMPLING_RATE = 2;
			static const nByte CHANNELS = 3;
			static const nByte FRAME_DURATION_US = 4;
			static const nByte BITRATE = 5;
			static const nByte WIDTH = 6;
			static const nByte HEIGHT = 7;
			static const nByte FPS = 8;
			static const nByte KEY_FRAME_INT = 9;
			static const nByte USER_DATA = 10;
			static const nByte EVENT_NUMBER = 11;
			static const nByte CODEC = 12;
			static const nByte EVENT_BUFFER_SIZE = 13;
		}

		PhotonTransportProtocol::PhotonTransportProtocol(const Voice::Logger& logger)
			: mLogger(logger)
		{
		}

		ValueObject<Object*> PhotonTransportProtocol::buildVoicesInfo(const LocalVoice& v, bool logInfo) const
		{
			Object infos[1];

			Dictionary<nByte, Object> dict;
			const VoiceInfo& vInfo = v.getInfo();
			dict.put(EventParam::VOICE_ID, ValueObject<nByte>(v.getId()));
			dict.put(EventParam::CODEC, ValueObject<int>(vInfo.getCodec()));
			dict.put(EventParam::SAMPLING_RATE, ValueObject<int>(vInfo.getSamplingRate()));
			dict.put(EventParam::CHANNELS, ValueObject<int>(vInfo.getChannels()));
			dict.put(EventParam::FRAME_DURATION_US, ValueObject<int>(vInfo.getFrameDurationUs()));
			dict.put(EventParam::BITRATE, ValueObject<int>(vInfo.getBitrate()));
			dict.put(EventParam::WIDTH, ValueObject<int>(vInfo.getWidth()));
			dict.put(EventParam::HEIGHT, ValueObject<int>(vInfo.getHeight()));
			dict.put(EventParam::FPS, ValueObject<int>(vInfo.getFPS()));
			dict.put(EventParam::KEY_FRAME_INT, ValueObject<int>(vInfo.getKeyFrameInt()));
			dict.put(EventParam::USER_DATA, vInfo.getUserData());
			dict.put(EventParam::EVENT_NUMBER, ValueObject<nByte>((nByte)0));
			dict.put(EventParam::EVENT_BUFFER_SIZE, ValueObject<int>(v.getEventBufferSize()));

			infos[0] = ValueObject<Dictionary<nByte, Object> >(dict);

			if(logInfo)
				EGLOG(DebugLevel::INFO, v.getLogPrefix() + L" Sending info: " + vInfo.toString());

			Object content[] =
			{
				ValueObject<nByte>((nByte)0),
				ValueObject<nByte>(EventSubcode::VOICE_INFO),
				ValueObject<Object*>(infos, 1),
			};

			ValueObject<Object*> res = ValueObject<Object*>(content, 3);
			return res;
		}

		ValueObject<Object*> PhotonTransportProtocol::buildVoiceRemoveMessage(const LocalVoice& v) const
		{
			nByte ids[] ={v.getId()};

			Object content[] =
			{
				ValueObject<nByte>((nByte)0),
				ValueObject<nByte>(EventSubcode::VOICE_REMOVE),
				ValueObject<nByte*>(ids, 1),
			};

			EGLOG(DebugLevel::INFO, v.getLogPrefix() + L" remove sent");

			return ValueObject<Object*>(content, 3);
		}

		// Transport 2 uses dedicated code for frame events
		void PhotonTransportProtocol::onVoiceFrameEvent(VoiceClient& voiceClient, const Object& content, int channelId, int playerId, int localPlayerId)
		{
			nByte* data = *static_cast<const ValueObject<nByte*>&>(content).getDataAddress();
			unsigned int size = *static_cast<const ValueObject<nByte*>&>(content).getSizes();
			if(size < 3)
			{
				EGLOG(DebugLevel::ERRORS, L"onVoiceFrameEvent did not receive data (readable as byte[])");
				return;
			}

			nByte dataOffset = data[0];
			nByte voiceId = data[1];
			unsigned short evNumber = data[2];
			FrameFlags::Enum flags = FrameFlags::None;
			if(dataOffset > 3)
				flags = (FrameFlags::Enum)data[3];
			nByte frNumber = evNumber & 0xFF;
			if(dataOffset > 4)
				frNumber = data[4];
			if(dataOffset > 5)
				evNumber += static_cast<unsigned short>(data[5] << 8);
			FrameBuffer receivedBytes(size-dataOffset, flags, frNumber);
			memcpy(receivedBytes.getArray(), data+dataOffset, receivedBytes.getSize());
			voiceClient.onFrame(channelId, playerId, voiceId, evNumber, receivedBytes, playerId == localPlayerId);
		}

		// Payloads are arrays. If first array element is 0 than next is event subcode. Otherwise, the event is Transport 1 data frame with voiceId in 1st element.
		void PhotonTransportProtocol::onVoiceEvent(VoiceClient& voiceClient, const Object& content, int channelId, int playerId, int localPlayerId)
		{
			Object* data = *static_cast<const ValueObject<Object*>&>(content).getDataAddress();
			nByte voiceId = ValueObject<nByte>(data[0]).getDataCopy();
			nByte byte1 = ValueObject<nByte>(data[1]).getDataCopy();

			if(!voiceId) // control event
			{
				switch(byte1) // event subcode
				{
				case EventSubcode::VOICE_INFO:
					onVoiceInfo(voiceClient, channelId, playerId, data[2]);
					break;
				case EventSubcode::VOICE_REMOVE:
					onVoiceRemove(voiceClient, channelId, playerId, data[2]);
					break;
				default:
				EGLOG(DebugLevel::ERRORS, JString(L"[PV] Unknown event subcode ") + byte1);
					break;
				}
			}
			else // stream packet Transport 1
			{
				unsigned short evNumber = byte1;
				const ValueObject<nByte*>& receivedBytesObj = (const ValueObject<nByte*>&)data[2];
				FrameFlags::Enum flags = FrameFlags::None;
				if(data->getSizes()[0] > 3)
					flags = (FrameFlags::Enum)ValueObject<nByte>(data[1]).getDataCopy();
				nByte frNumber = evNumber & 0xFF;
				if(data->getSizes()[0] > 4)
					frNumber = ValueObject<nByte>(data[4]).getDataCopy();
				if(data->getSizes()[0] > 5)
					evNumber += ValueObject<nByte>(data[5]).getDataCopy() << 8;

				FrameBuffer receivedBytes(receivedBytesObj.getSizes()[0], flags, frNumber);
				memcpy(receivedBytes.getArray(), *receivedBytesObj.getDataAddress(), receivedBytes.getSize());
				voiceClient.onFrame(channelId, playerId, voiceId, evNumber, receivedBytes, playerId == localPlayerId);
			}
		}

		void PhotonTransportProtocol::onVoiceInfo(VoiceClient& voiceClient, int channelId, int playerId, const Object& payload)
		{
			//mpLogger.logInfo(JString("====== ") + payload.toString());
			const ValueObject<Object*>& voiceInfos = static_cast<const ValueObject<Object*>&>(payload);
			Object* voiceInfosArr = *voiceInfos.getDataAddress();
			for(int i=0; i<voiceInfos.getSizes()[0]; ++i)
			{
				Dictionary<nByte, Object>* el = static_cast<const ValueObject<Dictionary<nByte, Object> >&>(voiceInfosArr[i]).getDataAddress();
				nByte voiceId = ValueObject<nByte>(el->getValue(EventParam::VOICE_ID)).getDataCopy();
				unsigned short eventBufferSize = ValueObject<int>(el->getValue(EventParam::EVENT_BUFFER_SIZE)).getDataCopy();
				VoiceInfo info = createVoiceInfoFromEventPayload(*el);
				voiceClient.onVoiceInfo(channelId, playerId, voiceId, eventBufferSize, info);
			}
		}

		void PhotonTransportProtocol::onVoiceRemove(VoiceClient& voiceClient, int channelId, int playerId, const Object& payload)
		{
			const ValueObject<nByte*>& voiceIds = (const ValueObject<nByte*>&)payload;
			voiceClient.onVoiceRemove(channelId, playerId, JVector<nByte>(*voiceIds.getDataAddress(), voiceIds.getSizes()[0]));
		}

		VoiceInfo PhotonTransportProtocol::createVoiceInfoFromEventPayload(const Dictionary<nByte, Object>& h)
		{
			return VoiceInfo()
				.setCodec((Codec::Enum)ValueObject<int>(h.getValue(EventParam::CODEC)).getDataCopy())
				.setSamplingRate(ValueObject<int>(h.getValue(EventParam::SAMPLING_RATE)).getDataCopy())
				.setChannels(ValueObject<int>(h.getValue(EventParam::CHANNELS)).getDataCopy())
				.setFrameDurationUs(ValueObject<int>(h.getValue(EventParam::FRAME_DURATION_US)).getDataCopy())
				.setBitrate(ValueObject<int>(h.getValue(EventParam::BITRATE)).getDataCopy())
				.setWidth(ValueObject<int>(h.getValue(EventParam::WIDTH)).getDataCopy())
				.setHeight(ValueObject<int>(h.getValue(EventParam::HEIGHT)).getDataCopy())
				.setFPS(ValueObject<int>(h.getValue(EventParam::FPS)).getDataCopy())
				.setKeyFrameInt(ValueObject<int>(h.getValue(EventParam::KEY_FRAME_INT)).getDataCopy())
				.setUserData(*h.getValue(EventParam::USER_DATA))
				;
		}

		const int MAX_DATA_OFFSET = 6;

		int PhotonTransportProtocol::getPayloadFragmentSize(const SendFrameParams& par)
		{
			// rough estimate, TODO: improve and test
			int overhead = 3 * 2; // possible InterestGroup and Receivers: key, type, value
			if(par.getTargetPlayers())
				overhead += 3 + par.getNumTargetPlayers(); // key, type, compressed ength and array

			return 1118 - MAX_DATA_OFFSET - overhead; // <- protocol 18 theoretical encrypted; experimental encoded: 1119, non-encrypted: 1134
		}

		nByte* PhotonTransportProtocol::buildFrameEvent(const Buffer<nByte>& data, nByte voiceId, unsigned short evNumber, nByte frNumber, FrameFlags::Enum flags, int& outSize)
		{
			// Transport 2 protocol
			nByte* pContent = allocateArray<nByte>(data.getSize() + MAX_DATA_OFFSET);
			int pos = 1;
			pContent[pos++] = voiceId;
			pContent[pos++] = evNumber & 0xFF;
			pContent[pos++] = flags;
			if(evNumber != frNumber) // save 1 byte if numbers match
			{
				pContent[pos++] = frNumber;
				if(evNumber >> 8) // save 1 byte if evNumber < 256, also backward compatibility
					pContent[pos++] = evNumber >> 8;
			}
			pContent[0] = pos;
			MEMCPY(pContent + pos, data.getArray(), data.getSize());

			outSize = pos;
			return pContent;
		}
	}
}