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

		float GetAnalogue(AnalogueControl axis) const override;
		bool GetDigital(DigitalControl control) const override;
	
		void	Update(float dt)		override;

	private:
		// Amount the trigger must be pulled
		float fireThreshold = 128.0f;
		bool digitalDown(ScePadButtonDataOffset button) const;

		ScePadData data;
		uint32_t padHandle;
		ScePadControllerInformation padInfo;
	};
}