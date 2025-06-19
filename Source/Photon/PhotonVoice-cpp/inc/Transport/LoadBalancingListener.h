/* Exit Games Photon Voice - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

#include "LoadBalancing-cpp/inc/Client.h"

namespace ExitGames
{
	namespace Voice
	{
		class VoiceClient;
		class LoadBalancingTransport;

		// This helper provides all callbacks required for LoadBalancing transport.
		// Extend it and make sure that base methods called if overridden.
		class LoadBalancingListener : public LoadBalancing::Listener
		{
		public:
			LoadBalancingListener(void);

			virtual void createRoomReturn(int localPlayerNr, const Common::Hashtable& roomProperties, const Common::Hashtable& playerProperties, int errorCode, const Common::JString& errorString);
			virtual void joinOrCreateRoomReturn(int localPlayerNr, const Common::Hashtable& roomProperties, const Common::Hashtable& playerProperties, int errorCode, const Common::JString& errorString);
			virtual void joinRandomOrCreateRoomReturn(int localPlayerNr, const Common::Hashtable& roomProperties, const Common::Hashtable& playerProperties, int errorCode, const Common::JString& errorString);
			virtual void joinRoomReturn(int localPlayerNr, const Common::Hashtable& roomProperties, const Common::Hashtable& playerProperties, int errorCode, const Common::JString& errorString);
			virtual void joinRandomRoomReturn(int localPlayerNr, const Common::Hashtable& roomProperties, const Common::Hashtable& playerProperties, int errorCode, const Common::JString& errorString);
			virtual void leaveRoomReturn(int errorCode, const Common::JString& errorString);
			virtual void disconnectReturn(void);
			virtual void joinRoomEventAction(int playerNr, const Common::JVector<int>& playernrs, const LoadBalancing::Player& player);
			virtual void leaveRoomEventAction(int playerNr, bool isInactive);
			virtual void customEventAction(int playerNr, nByte eventCode, const Common::Object& eventContent);
			virtual void onDirectMessage(const Common::Object& msg, int remoteID, bool relay);
		protected:
			VoiceClient* mpVoiceClient;
			LoadBalancingTransport* mpTransport;
		};
	}
}