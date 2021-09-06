#pragma once
#include "Typedefs.h"
#include <SDL.h>
#include <unordered_map>

//Stores current keyboard and mouse input state. Can be queried by any code to react to input.
class Input
{
public:
	//Called by the main loop when it polls events
    void HandleEvent(SDL_Event* event);
	//Reset per-frame state
	void EndFrame();

	//Returns true if the key is down
	bool KeyDown(SDL_KeyCode keycode);
	//Returns true if the key was pressed this frame. Use for non-repeating keypress behavior.
	bool KeyPressed(SDL_KeyCode keycode);
	//Shortcut key states
	bool ShiftDown();
	bool ControlDown();
	bool AltDown();
	bool CapsLockDown();
	bool NumLockDown();
	bool ScrollLockDown();

private:
	void HandleKeyEvent(const SDL_KeyboardEvent& event);
	void HandleMouseButtonEvent(const SDL_MouseButtonEvent& event);
	void HandleMouseMoveEvent(const SDL_MouseMotionEvent& event);

	//Key states. Have to use a map instead of a sized array since some SDL_KeyCode enums have huge values like 1073742049.
    std::unordered_map<SDL_KeyCode, bool> _keyDownStates;
	//Keys that were pressed this frame. Cleared at the end of every frame.
	std::vector<SDL_KeyCode> _keysDownThisFrame;

	//Shorthand key states. For keys that have left/right these are true if either side is down.
	bool _shiftDown = false;
	bool _controlDown = false;
	bool _altDown = false;
	bool _capsLockDown = false;
	bool _numLockDown = false;
	bool _scrollLockDown = false;

	//Mouse state
	bool _leftMouseButtonDown = false;
	bool _rightMouseButtonDown = false;
	bool _middleMouseButtonDown = false;
	i32 _lastMouseX = 0;
	i32 _lastMouseY = 0;
	i32 _mousePosX = 0;
	i32 _mousePosY = 0;
	i32 _mouseDeltaX = 0;
	i32 _mouseDeltaY = 0;
	bool _mouseButtonPressed = false;
	bool _mouseMoved = false;
};