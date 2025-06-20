/* Exit Games Photon Chat - C++ Client Lib
 * Copyright (C) 2004-2025 Exit Games GmbH. All rights reserved.
 * https://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "Chat-cpp/inc/AuthenticationValues.h"
#include "Chat-cpp/inc/Internal/AuthenticationValuesSecretSetter.h"

namespace ExitGames
{
	namespace Chat
	{
		namespace Internal
		{
			using namespace Common;

			void AuthenticationValuesSecretSetter::setSecret(AuthenticationValues& authenticationValues, const JString& secret)
			{
				authenticationValues.setSecret(secret);
			}
		}
	}
}