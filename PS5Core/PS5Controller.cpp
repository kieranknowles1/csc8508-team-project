/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "PS5Controller.h"
#include <sys\_defines\_sce_ok.h>
#include <pad.h>
#include <assert.h>

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
	int ret = scePadReadState(padHandle, &data);

	if (ret != SCE_OK || !data.connected) {
		std::cerr << "Failed to read controller state" << std::endl;
	}
};

float PS5Controller::GetAnalogue(AnalogueControl control) const {
	switch (control)
	{
	case Controller::AnalogueControl::MoveSidestep:
		return ConvertAxis(data.leftStick.x, padInfo.stickInfo.deadZoneLeft);
	case Controller::AnalogueControl::MoveUpDown:
		if (digitalDown(SCE_PAD_BUTTON_DOWN)) return -1.0f;
		else return digitalDown(SCE_PAD_BUTTON_UP) ? 1.0f : 0.0f;
	case Controller::AnalogueControl::MoveForward:
		return -ConvertAxis(data.leftStick.y, padInfo.stickInfo.deadZoneLeft);
	case Controller::AnalogueControl::LookX:
		return ConvertAxis(data.rightStick.x, padInfo.stickInfo.deadZoneRight);
	case Controller::AnalogueControl::LookY:
		return ConvertAxis(data.rightStick.y, padInfo.stickInfo.deadZoneRight);
	default:
		assert(false);
	}
}

bool PS5Controller::digitalDown(ScePadButtonDataOffset button) const {
	return data.buttons & button ? true : false;
}

bool PS5Controller::GetDigital(DigitalControl control) const {
	switch (control)
	{
	case Controller::DigitalControl::Fire:
		return data.analogButtons.r2 >= fireThreshold;
	case Controller::DigitalControl::Jump:
		return digitalDown(SCE_PAD_BUTTON_CROSS);
	case Controller::DigitalControl::Sprint:
		return digitalDown(SCE_PAD_BUTTON_L3);
	case Controller::DigitalControl::Crouch:
		return digitalDown(SCE_PAD_BUTTON_CIRCLE);
	default:
		assert(false);
	}
}