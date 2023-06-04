#include "Window.h"
#include "System/Scene/SceneSystem.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/RenderManager/RenderManager.h"

#define WIN32_LEAN_AND_MEAN  
#include <Windows.h>

#include <iostream>
#include <GL/glew.h>
#include "imgui.h"

bool Window::CreateWindow_(std::string name, glm::ivec2 window_size, bool fullscreen)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		exit(1);
	}

	// Get the configuration form the parameters
	mbFullscreen = fullscreen;
	mWindowName = name;
	mWindowSize = window_size;

	// Get the total screen size (the actual monitor) we may need it later
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	mScreenSize.x = DM.w;
	mScreenSize.y = DM.h;

#ifndef EDITOR
	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
#endif // !EDITOR


	// Create the actual window
	if (mbFullscreen = fullscreen) 
	{
		mWindowHandle = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			mScreenSize.x, mScreenSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		mWindowHandle = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			mWindowSize.x, mWindowSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	}

	if (mWindowHandle == nullptr)
	{
		SDL_Quit();
		exit(1);
	}

	// Create the rendering context, we will be using OpenGL, we are going to use the core profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);	// OpenGL 4+
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);	// OpenGL 4.4

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);			// Use double buffer
	SDL_GL_SetSwapInterval(1); // Enable vsync

	mGLContext = SDL_GL_CreateContext(mWindowHandle);
	if (mGLContext == nullptr)
	{
		SDL_DestroyWindow(mWindowHandle);
		SDL_Quit();
		exit(1);
	}

	// Initialize OpenGL through glew
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		SDL_GL_DeleteContext(mGLContext);
		SDL_DestroyWindow(mWindowHandle);
		SDL_Quit();
		exit(1);
	}

	return true;
}

void Window::SetTemporalName(std::string _name) const
{
	if (!mWindowHandle) {
		std::cerr << "No window handle" << std::endl;
		return;
	}

	SDL_SetWindowTitle(mWindowHandle, _name.data());
}

void Window::DestroyWindow()
{
	// Finish all things from SDL and free resources
	SDL_GL_DeleteContext(mGLContext);
	SDL_DestroyWindow(mWindowHandle);
	SDL_Quit();

	mWindowHandle = nullptr;
}

void Window::ToggleFullScreen()
{
	// Save the new configuration
	mbFullscreen = !mbFullscreen;

	if (mbFullscreen)
	{
		// We do not want the window size varaible to be affected (in any case, query of the 
		// window size will be done through the GetResolution function that already handles
		// fullscreen option properly)
		auto prev_size = mWindowSize;
		ResizeWindow(mScreenSize);
		mWindowSize = prev_size;

		// First change the resolution, then set to fullscreen (so we avoid changing 
		// the resolution of the monitor)
		SDL_SetWindowFullscreen(mWindowHandle,
			!mbFullscreen ? SDL_WINDOW_SHOWN : SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		// First go out from fullscreen mode (so that when we change the resolution, it does
		// not affect the monitor's resolution)
		SDL_SetWindowFullscreen(mWindowHandle,
			!mbFullscreen ? SDL_WINDOW_SHOWN : SDL_WINDOW_FULLSCREEN);
		ResizeWindow(mWindowSize);

		// Make sure the bar above the app is always visible (even at the expense of not 
		// showing the whole window)
		SDL_SetWindowPosition(mWindowHandle, 20, 30);
	}
}

void Window::ChangeResolution(glm::ivec2 const& _size)
{
	// For now at least, we will not allow resolution changes when in fullscreen mode
	// (and when in fullscreen the screen size will be the resolution)
	if (mbFullscreen) ToggleFullScreen();

	ResizeWindow(_size);
}

void Window::ResizeWindow(glm::ivec2 const& _size)
{
	// Actually change the size 
	mWindowSize = _size;
	SDL_SetWindowSize(mWindowHandle, _size.x, _size.y);

#ifdef EDITOR
	// Notify ImGui of the change
	if (ImGui::GetCurrentContext())
		ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(_size.x), static_cast<float>(_size.y));
#endif


	// Set properly the new viewport
	glEnable(GL_SCISSOR_TEST);
	glViewport(0, 0, _size.x, _size.y);
	glScissor(0, 0, _size.x, _size.y);

	// We also need to update the frame buffers for the cameras
	auto& spaces = Scene.GetSpaces();
	std::for_each(spaces.begin(), spaces.end(), [&](auto& it)
	{
		auto& cameras = it->GetComponentsType<CameraComp>();
		std::for_each(cameras.begin(), cameras.end(), [&](auto& cam)
		{
			auto casted_cam = reinterpret_cast<CameraComp*>(cam);
			casted_cam->RegenerateBuffers();
		});
	});

	// Since the buffers have been regenerated, we need to update the bindings
	//RenderMgr.InitializeLighting();
}
