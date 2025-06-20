/* Exit Games Photon Chat - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#pragma once

namespace ExitGames
{
	namespace Common
	{
		class JString;
	}

	namespace Chat
	{
		class Client;

		namespace Internal
		{
			class ChannelFactory
			{
				static Channel* create(const Common::JString& name, int id, bool isPrivate);
				static void destroy(const Channel* pChannel);

				friend class Chat::Client;
			};
		}
	}
}