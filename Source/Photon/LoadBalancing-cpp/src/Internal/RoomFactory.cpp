/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Room.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;
			using namespace Common::MemoryManagement;

			Room* RoomFactory::create(const JString& name, const Hashtable& properties)
			{
				Room* p;
				return ALLOCATE(Room, p, name, properties);
			}

			void RoomFactory::destroy(const Room* pRoom)
			{
				deallocate(pRoom);
			}
		}
	}
}