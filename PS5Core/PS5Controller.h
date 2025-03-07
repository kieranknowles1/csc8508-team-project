/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pad.h"

#include "JoystickController.h"

namespace NCL::PS5 {
	class PS5Controller : public JoystickController {
	public:
		PS5Controller(SceUserServiceUserId id);
		virtual ~PS5Controller(void);

		void Update(float dt) override;
	protected:
		uint32_t padHandle;
		ScePadControllerInformation padInfo;
		ScePadData data;

		bool internalButtonPressed(Button button) override;
		float internalAnalogueValue(Analogue analogue) override;
	};
}
