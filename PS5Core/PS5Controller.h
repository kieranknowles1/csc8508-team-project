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
		float fireThreshold = 127.0f;

		bool buttonPressed(ScePadButtonDataOffset button) const;

		uint32_t padHandle;
		ScePadControllerInformation padInfo;
		ScePadData data;
	};
}