#include "Events/EventDispatcher.h"
#include "Input.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui_impl_sdl.h"
#endif

namespace glm
{
	std::ostream& operator<<(std::ostream& os, const ivec2& v)
	{
		os << v.x << " " << v.y;
		return os;
	}
	std::ostream& operator<<(std::ostream& os, const vec2& v)
	{
		os << v.x << " " << v.y;
		return os;
	}
}

bool InputHandler::Initialize()
{
	// Set both Current and previous arrays of booleans to false
	for (unsigned i = 0; i < MOUSE_KEY_AMOUNT; ++i)
	{
		mMouseCurrent[i] = 0;
		mMousePrevious[i] = 0;
	}

	for (unsigned i = 0; i < KEYBOARD_KEY_AMOUNT; ++i)
	{
		mKeyCurrent[i] = 0;
		mKeyPrevious[i] = 0;
	}

	mPreviousButton[SDL_CONTROLLER_BUTTON_A] = 0;				mCurrentButton[SDL_CONTROLLER_BUTTON_A] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_B] = 0;				mCurrentButton[SDL_CONTROLLER_BUTTON_B] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_X] = 0;				mCurrentButton[SDL_CONTROLLER_BUTTON_X] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_Y] = 0;				mCurrentButton[SDL_CONTROLLER_BUTTON_Y] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = 0;		mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_UP] = 0;			mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_UP] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = 0;		mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_START] = 0;			mCurrentButton[SDL_CONTROLLER_BUTTON_START] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = 0;		mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = 0;	mCurrentButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_LEFTSTICK] = 0;		mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSTICK] = 0;
	mPreviousButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = 0;		mCurrentButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = 0;

	return true;
}
void InputHandler::StartFrame()
{
	/* Reset the Current and Previous arrays */
	for (unsigned i = 0; i < MOUSE_KEY_AMOUNT; ++i)
		mMousePrevious[i] = mMouseCurrent[i];

	for (unsigned i = 0; i < KEYBOARD_KEY_AMOUNT; ++i) 
		mKeyPrevious[i] = mKeyCurrent[i];

	mWheelCurrent = false;
	mWheelScroll = 0;

	mPreviousButton[SDL_CONTROLLER_BUTTON_A] = mCurrentButton[SDL_CONTROLLER_BUTTON_A];
	mPreviousButton[SDL_CONTROLLER_BUTTON_B] = mCurrentButton[SDL_CONTROLLER_BUTTON_B];
	mPreviousButton[SDL_CONTROLLER_BUTTON_X] = mCurrentButton[SDL_CONTROLLER_BUTTON_X];
	mPreviousButton[SDL_CONTROLLER_BUTTON_Y] = mCurrentButton[SDL_CONTROLLER_BUTTON_Y];
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN];
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_UP] = mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_UP];
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT];
	mPreviousButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT];
	mPreviousButton[SDL_CONTROLLER_BUTTON_START] = mCurrentButton[SDL_CONTROLLER_BUTTON_START];
	mPreviousButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER];
	mPreviousButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER];
	mPreviousButton[SDL_CONTROLLER_BUTTON_LEFTSTICK] = mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSTICK];
	mPreviousButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = mCurrentButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK];
}
void InputHandler::HandleEnvents(bool* _quit)
{
	GetRawMouse();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		#ifdef EDITOR
		ImGui_ImplSDL2_ProcessEvent(&event);
		#endif

		switch (event.type)
		{
		case SDL_QUIT:
			*_quit = true;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			InputManager.HandleKeyEvent(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			InputManager.HandleMouseEvent(event);
			break;
		case SDL_MOUSEWHEEL:
			InputManager.HandleMouseWheel(event);
			break;
		case SDL_MOUSEMOTION:
			mRelMouse = glm::ivec2(event.motion.xrel, event.motion.yrel);
			break;
		}
		HandleGamePadEvent(event);
	}
}

#pragma region // MOUSE //

void InputHandler::HandleMouseEvent(SDL_Event event)
{
	// Access the index with -1 beacuse they go:
	// LEFT = 1, MIDDLE = 2, RIGHT = 3
	mMouseCurrent[event.button.button - 1] = event.button.state ? true : false;
}
bool InputHandler::WheelTriggered()
{
	return mWheelCurrent;
}
bool InputHandler::MouseIsDown(MouseKey index)
{
	return mMouseCurrent[static_cast<unsigned>(index)];
}
bool InputHandler::MouseIsUp(MouseKey index)
{
	return !mMouseCurrent[static_cast<unsigned>(index)];
}
bool InputHandler::MouseIsTriggered(MouseKey index)
{
	if (mMouseCurrent[static_cast<unsigned>(index)] == true)
	{
		if (mMouseCurrent[static_cast<unsigned>(index)] != mMousePrevious[static_cast<unsigned>(index)])
			return true;
	}
	return false;
}
bool InputHandler::MouseIsReleased(MouseKey index)
{
	if (mMouseCurrent[static_cast<unsigned>(index)] == false)
	{
		if (mMouseCurrent[static_cast<unsigned>(index)] != mMousePrevious[static_cast<unsigned>(index)])
			return true;
	}
	return false;
}

int InputHandler::GetWheelScroll() const
{
	return mWheelScroll;
}

#pragma endregion

#pragma region // KEYBOARD //

void InputHandler::HandleKeyEvent(SDL_Event event)
{
	SDL_Keycode ScanCode = event.key.keysym.scancode;

	if (ScanCode > 0 && ScanCode < KEYBOARD_KEY_AMOUNT)
		mKeyCurrent[ScanCode] = event.key.state ? true : false;
}

bool InputHandler::KeyIsDown(Key index)
{
#ifdef EDITOR
	if (ImGui::GetIO().WantCaptureKeyboard) return false;
#endif // EDITOR

	return mKeyCurrent[static_cast<unsigned>(index)];
}
bool InputHandler::KeyIsUp(Key index)
{
#ifdef EDITOR
	if (ImGui::GetIO().WantCaptureKeyboard) return false;
#endif // EDITOR
	return !KeyIsDown(index);
}
bool InputHandler::KeyIsTriggered(Key index)
{
#ifdef EDITOR
	if (ImGui::GetIO().WantCaptureKeyboard) return false;
#endif // EDITOR

	if (mKeyCurrent[static_cast<unsigned>(index)] == true)
	{
		if (mKeyCurrent[static_cast<unsigned>(index)] != mKeyPrevious[static_cast<unsigned>(index)])
			return true;
	}
	return false;
}
bool InputHandler::KeyIsReleased(Key index)
{
#ifdef EDITOR
	if (ImGui::GetIO().WantCaptureKeyboard) return false;
#endif // EDITOR

	if (mKeyCurrent[static_cast<unsigned>(index)] == false)
	{
		if (mKeyCurrent[static_cast<unsigned>(index)] != mKeyPrevious[static_cast<unsigned>(index)])
			return true;
	}
	return false;
}
#pragma endregion

#pragma region // GAMEPAD //

void InputHandler::HandleAxisEvent(SDL_Event event)
{
	switch (event.caxis.axis)
	{
	case 0://X axis
		mCurrentJoyStickValue.x = event.caxis.value >= 0 ? event.caxis.value / 32767.0F : event.caxis.value / 32768.0F;
		mCurrentJoyStickValue.x = abs(mCurrentJoyStickValue.x) <= 0.2F ? 0.00F : mCurrentJoyStickValue.x;
		break;
	case 1://Y axis 
		mCurrentJoyStickValue.y = event.caxis.value >= 0 ? -event.caxis.value / 32767.0F : -event.caxis.value / 32768.0F;
		mCurrentJoyStickValue.y = abs(mCurrentJoyStickValue.y) < 0.2F ? 0.00F : mCurrentJoyStickValue.y;
		break;
	}
}
void InputHandler::HandleGamePadEvent(SDL_Event gamepad_event)
{
	switch (gamepad_event.cbutton.button)
	{
	case SDL_CONTROLLER_BUTTON_A:
		mCurrentButton[SDL_CONTROLLER_BUTTON_A] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_B:
		mCurrentButton[SDL_CONTROLLER_BUTTON_B] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_X:
		mCurrentButton[SDL_CONTROLLER_BUTTON_X] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_Y:
		mCurrentButton[SDL_CONTROLLER_BUTTON_Y] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSTICK] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
		mCurrentButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_DPAD_UP:
		mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_UP] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		mCurrentButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = gamepad_event.cbutton.state ? true : false;
		break;

	case SDL_CONTROLLER_BUTTON_START:
		mCurrentButton[SDL_CONTROLLER_BUTTON_START] = gamepad_event.cbutton.state ? true : false;
		break;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		mCurrentButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = gamepad_event.cbutton.state ? true : false;
		break;
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		mCurrentButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = gamepad_event.cbutton.state ? true : false;
		break;
	default:
		break;
	}
}
void InputHandler::HandleMouseWheel(SDL_Event event)
{
	mWheelCurrent = true;
	mWheelScroll = event.wheel.y;
}
void InputHandler::ResetPad(SDL_Event gamepad_event)
{
}

bool InputHandler::ButtonIsDown(unsigned index)
{
	return mCurrentButton[index];
}

bool InputHandler::ButtonIsUp(unsigned index)
{
	return !ButtonIsDown(index);
}

bool InputHandler::ButtonIsTriggered(unsigned index)
{
	if (mCurrentButton[index] == true)
	{
		if (mCurrentButton[index] != mPreviousButton[index])
			return true;
	}
	return false;
}

bool InputHandler::ButtonIsReleased(unsigned index)
{
	if (mCurrentButton[index] == false)
	{
		if (mCurrentButton[index] != mPreviousButton[index])
			return true;
	}
	return false;
}

bool InputHandler::CheckOther(unsigned button)
{
	if (button == SDL_CONTROLLER_BUTTON_A)
	{
		if (!ButtonIsUp(SDL_CONTROLLER_BUTTON_X))
		{
			if (lapse > offset && ButtonIsDown(SDL_CONTROLLER_BUTTON_X))
			{
				GamepadEvent double_press(SDL_CONTROLLER_BUTTON_A, 0);
				double_press.double_press = true;

				EventDisp.trigger_event(double_press);

				buttonDown.erase(button);
				buttonDown.erase(SDL_CONTROLLER_BUTTON_X);

				return true;
			}
		}
	}

	if (button == SDL_CONTROLLER_BUTTON_X)
	{
		if (!ButtonIsUp(SDL_CONTROLLER_BUTTON_A))
		{
			if (lapse > offset && ButtonIsDown(SDL_CONTROLLER_BUTTON_A))
			{
				GamepadEvent double_press(SDL_CONTROLLER_BUTTON_X, 0);
				double_press.double_press = true;
				
				EventDisp.trigger_event(double_press);

				buttonDown.erase(button);
				buttonDown.erase(SDL_CONTROLLER_BUTTON_A);

				return true;
			}
		}
	}

	return false;
}

#pragma endregion

#pragma region // MOUSE POSITIONS //

#include "Window/Window.h"

const glm::ivec2& InputHandler::RawMousePos() const
{
	return mRawMouse;
}
const glm::vec2& InputHandler::WindowMousePos() const
{
	return mWindowMouse;
}

void InputHandler::SetMousePos(glm::vec2 pos)
{
	mWindowMouse = pos;
}

void InputHandler::GetRawMouse()
{
	SDL_GetMouseState(&mRawMouse[0], &mRawMouse[1]);
	mWindowMouse = mRawMouse - WindowMgr.GetResolution() / 2;
	mWindowMouse.y *= -1;
}

#pragma endregion
