/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "PS5Controller.h"
#include <sys\_defines\_sce_ok.h>
#include <pad.h>

using namespace NCL;
using namespace PS5;

PS5Controller::PS5Controller(SceUserServiceUserId id) {
	padHandle = scePadOpen(id, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);

	scePadGetControllerInformation(padHandle, &padInfo);
}

PS5Controller::~PS5Controller(void) {
	scePadClose(padHandle);
}

float ConvertAxis(uint8_t rawValue, uint8_t deadZone) {
	if (std::abs(rawValue - 128) <= deadZone) {
		return 0.0f;
	}
	return (rawValue / 128.0f) - 1.0f;
}

void	PS5Controller::Update(float dt) {
	ScePadData data;

	int ret = scePadReadState(padHandle, &data);
}

bool PS5Controller::buttonPressed(ScePadButtonDataOffset button) const {
	return data.buttons & button ? true : false;
}

float PS5Controller::GetAnalogue(AnalogueControl control) const
{
	switch (control)
	{
	case Controller::AnalogueControl::MoveSidestep:
		return ConvertAxis(data.leftStick.x, padInfo.stickInfo.deadZoneLeft);
	case Controller::AnalogueControl::MoveUpDown:
		if (buttonPressed(SCE_PAD_BUTTON_UP)) return 1.0f;
		return buttonPressed(SCE_PAD_BUTTON_DOWN) ? 1.0f : 0.0f;
	case Controller::AnalogueControl::MoveForward:
		return ConvertAxis(data.leftStick.y, padInfo.stickInfo.deadZoneLeft);
	case Controller::AnalogueControl::LookX:
		return ConvertAxis(data.rightStick.x, padInfo.stickInfo.deadZoneRight);
	case Controller::AnalogueControl::LookY:
		return ConvertAxis(data.rightStick.y, padInfo.stickInfo.deadZoneRight);
	default:
		assert(false);
	}
}

bool PS5Controller::GetDigital(DigitalControl button) const
{
	switch (button)
	{
	case Controller::DigitalControl::Fire:
		return data.analogButtons.r2 >= fireThreshold;
	case Controller::DigitalControl::Jump:
		return buttonPressed(SCE_PAD_BUTTON_CROSS);
	case Controller::DigitalControl::Sprint:
		return buttonPressed(SCE_PAD_BUTTON_L3);
	case Controller::DigitalControl::Crouch:
		return buttonPressed(SCE_PAD_BUTTON_CIRCLE);
	default:
		assert(false);
	}
}