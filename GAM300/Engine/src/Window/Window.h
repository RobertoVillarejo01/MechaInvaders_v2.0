#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "Utilities/Singleton.h"
#include "GameStateManager/MenuManager/MenuManager.h"

/**
 * Simple window class, it holds a SDL window and initializes it and destroys it
 */
class Window
{
	MAKE_SINGLETON(Window)

public:
	~Window() { DestroyWindow(); }
	bool CreateWindow_(std::string name, glm::ivec2 window_size, bool fullscreen);
	void SetTemporalName(std::string _name) const;

	void ToggleFullScreen();
	void ChangeResolution(glm::ivec2 const& _size);

	// Some gettors
	const glm::ivec2&	GetResolution() const { return mbFullscreen ? mScreenSize : mWindowSize; };
	SDL_Window*			GetHandle() const { return mWindowHandle; };
	SDL_GLContext		GetContext() const { return mGLContext; };
	const std::string&  GetName() const { return mWindowName; }


private:
	/* Clean the resources for the OpenGL context and the window */
	void DestroyWindow();


	void ResizeWindow(glm::ivec2 const& _size);

	SDL_Window*		mWindowHandle = nullptr;
	SDL_GLContext	mGLContext = {};

	std::string		mWindowName = {};
	glm::ivec2		mWindowSize = {};
	glm::ivec2		mScreenSize = {};

	// SHOULD NOT MODIFY THIS DIRECTLY
	bool			mbFullscreen = false;

	friend class MenuManager;
};

#define WindowMgr (Window::Instance())