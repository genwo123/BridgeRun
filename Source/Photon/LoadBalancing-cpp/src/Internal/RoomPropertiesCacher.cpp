/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Internal/RoomPropertiesCacher.h"
#include "LoadBalancing-cpp/inc/Room.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;

			void RoomPropertiesCacher::cache(Room& room, const Hashtable& properties)
			{
				room.cacheProperties(properties);
			}
		}
	}
}