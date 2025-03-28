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

	memset(&data, 0, sizeof(data));
	memset(&prevData, 0, sizeof(prevData));
}

PS5Controller::~PS5Controller(void) {
	scePadClose(padHandle);
}

void PS5Controller::Update(float dt) {
	prevData = data;
	int ret = scePadReadState(padHandle, &data);
	if (ret != SCE_OK) {
		std::cerr << "Failed to read input" << std::endl;
	}

	ScePadLightBarParam light;
	light.r = feedback.color.x * 255.0f;
	light.g = feedback.color.y * 255.0f;
	light.b = feedback.color.z * 255.0f;
	scePadSetLightBar(padHandle, &light);
	JoystickController::Update(dt);
}

ScePadButtonDataOffset toSce(JoystickController::Button button) {
	using enum JoystickController::Button;
	switch (button)
	{
	case PsCross: return SCE_PAD_BUTTON_CROSS;
	case PsCircle: return SCE_PAD_BUTTON_CIRCLE;
	case PsSquare: return SCE_PAD_BUTTON_SQUARE;
	case PsTriangle: return SCE_PAD_BUTTON_TRIANGLE;
	case PadUp: return SCE_PAD_BUTTON_UP;
	case PadDown: return SCE_PAD_BUTTON_DOWN;
	case PadLeft: return SCE_PAD_BUTTON_LEFT;
	case PadRight: return SCE_PAD_BUTTON_RIGHT;
	case Start: return SCE_PAD_BUTTON_START;
	case L1: return SCE_PAD_BUTTON_L1;
	case L3: return SCE_PAD_BUTTON_L3;
	case R1: return SCE_PAD_BUTTON_R1;
	case R3: return SCE_PAD_BUTTON_R3;
	case TrackpadClick: return SCE_PAD_BUTTON_TOUCH_PAD;
	case Select:
	case DeckL4:
	case DeckL5:
	case DeckR4:
	case DeckR5:
		return (ScePadButtonDataOffset)0; // Not present, will fail bitwise AND
	default: assert(false);
	}
}

bool NCL::PS5::PS5Controller::internalButtonPressed(Button button)
{
	auto sce = toSce(button);
	return (data.buttons & sce) != 0;
}

float ConvertAxis(uint8_t rawValue, uint8_t deadZone) {
	if (std::abs(rawValue - 128) <= deadZone) {
		return 0.0f;
	}
	return (rawValue / 128.0f) - 1.0f;
}

float triggerValue(uint8_t raw) {
	return float(raw) / 256.0f;
}

Maths::Vector2 PS5Controller::trackMovement() const {
	if (data.touchData.touchNum == 0 || prevData.touchData.touchNum == 0) {
		return Maths::Vector2(0, 0);
	}

	return Maths::Vector2(
		(data.touchData.touch[0].x - prevData.touchData.touch[0].x) / 32.0f,
		(data.touchData.touch[0].y - prevData.touchData.touch[0].y) / 32.0f
	);
}

float PS5Controller::internalAnalogueValue(Analogue analogue)
{
	using enum JoystickController::Analogue;
	switch (analogue)
	{
	case LeftStickX: return ConvertAxis(data.leftStick.x, padInfo.stickInfo.deadZoneLeft);
	case LeftStickY: return ConvertAxis(data.leftStick.y, padInfo.stickInfo.deadZoneLeft);
	case RightStickX: return ConvertAxis(data.rightStick.x, padInfo.stickInfo.deadZoneRight);
	case RightStickY: return  ConvertAxis(data.rightStick.y, padInfo.stickInfo.deadZoneRight);
	case L2: return triggerValue(data.analogButtons.l2);
	case R2: return triggerValue(data.analogButtons.r2);
	case TrackpadX: return trackMovement().x;
	case TrackpadY: return trackMovement().y;
	default: assert(false);
	}
}
