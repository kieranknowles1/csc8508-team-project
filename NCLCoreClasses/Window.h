/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

#include <string>
#include <functional>

#include "Keyboard.h"
#include "Mouse.h"

#include "Vector.h"
#include "GameTimer.h"

#include <nlohmann/json.hpp>

namespace NCL {
	class GameTimer;
	namespace Rendering {
		class RendererBase;
	};
	using namespace Rendering;

	enum class WindowEvent {
		Minimize,
		Maximize,
		Resize,
		Fullscreen,
		Windowed
	};

	enum class FullScreenState {
		None,
		Exclusive,
		Borderless
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(FullScreenState, {
		{FullScreenState::None, "None"},
		{FullScreenState::Exclusive, "Exclusive"},
		{FullScreenState::Borderless, "Borderless"}
	})

	struct WindowInitialisation {
		uint32_t width;
		uint32_t height;
		FullScreenState	fullScreen = FullScreenState::None;
		uint32_t refreshRate		= 60;

		std::string windowTitle		= "NCLGL!";
	};

	using WindowEventHandler = std::function<void(WindowEvent e, uint32_t w, uint32_t h)>;

	class Window {
	public:
		static Window* CreateGameWindow(const WindowInitialisation& init);

		bool		IsMinimised() const { return minimised;	 }

		bool		UpdateWindow();

		bool		HasInitialised()	const { return init; }

		float		GetScreenAspect()	const {
			return (float)size.x / (float)size.y;
		}

		Vector2i		GetScreenSize()		const { return size; }
		Vector2i		GetScreenPosition()	const { return position; }

		const std::string&  GetTitle()   const { return windowTitle; }
		void				SetTitle(const std::string& title) {
			windowTitle = title;
			UpdateTitle();
		};

		virtual void	LockMouseToWindow(bool lock) {};
		virtual void	ShowOSPointer(bool show) {};

		virtual void	SetWindowPosition(int x, int y) {};
		virtual void	SetFullScreen(bool state) {};
		virtual void	SetConsolePosition(int x, int y) {};
		virtual void	ShowConsole(bool state) {};

		const Keyboard* GetKeyboard() { return keyboard; }
		const Mouse* GetMouse() { return mouse; }
		const GameTimer& GetTimer() { return timer; }

		void SetWindowEventHandler(const WindowEventHandler& e) {
			eventHandler = e;
		}
		virtual ~Window();
	protected:
		Window();

		virtual void UpdateTitle() {}

		virtual bool InternalUpdate() = 0;

		WindowEventHandler eventHandler;

		bool				minimised;
		bool				init;
		Vector2i			position;
		Vector2i			size;
		Vector2i			defaultSize;

		std::string			windowTitle;

		Keyboard* keyboard;
		Mouse* mouse;

		GameTimer timer;
	};
}
