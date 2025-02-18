/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Controller.h"
#include "pad.h"

namespace NCL::PS5 {
	class PS5Controller : public Controller {
	public:
		PS5Controller(SceUserServiceUserId id);
		virtual ~PS5Controller(void);

		void Update(float dt) override;
		float GetAnalogue(AnalogueControl control) const override;
		bool GetDigital(DigitalControl button) const override;
	private:
		// If this button is pressed, listen to debug inputs and ignore others
		ScePadButtonDataOffset DebugMask = SCE_PAD_BUTTON_L1;
		float fireThreshold = 127.0f;
		float lookSensitivity = 2.0f;

		bool buttonPressed(ScePadButtonDataOffset button, bool isDebug = false, bool thisFrame = false) const;

		uint32_t padHandle;
		ScePadControllerInformation padInfo;
		ScePadData data;
		ScePadData lastFrameData;
	};
}
