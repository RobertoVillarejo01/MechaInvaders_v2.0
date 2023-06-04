#pragma once

#include "Events/event.h"


class GamepadEvent : public Event
{
public:
	GamepadEvent(unsigned button_, unsigned time):button(button_), double_press(false), time_pressed(time) {}
	unsigned button;
	bool double_press;
	unsigned time_pressed;
};
