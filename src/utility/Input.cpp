#include "Input.h"

void Input::HandleEvent(SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_EventType::SDL_KEYDOWN:
    case SDL_EventType::SDL_KEYUP:
        HandleKeyEvent(event->key);
        break;

    case SDL_EventType::SDL_MOUSEBUTTONDOWN:
    case SDL_EventType::SDL_MOUSEBUTTONUP:
        HandleMouseButtonEvent(event->button);
        break;

    case SDL_EventType::SDL_MOUSEMOTION:
        HandleMouseMoveEvent(event->motion);
        break;

    case SDL_EventType::SDL_MOUSEWHEEL:
    default:
        break;
    }
}

void Input::NewFrame()
{
    _mouseButtonPressed = false;
    _mouseMoved = false;
    _keysDownThisFrame.clear();
}

void Input::HandleKeyEvent(const SDL_KeyboardEvent& event)
{
    if (event.type == SDL_EventType::SDL_KEYDOWN)
    {
        //Add to per-frame list if the key isn't already down
        //That way KeyPressed() will only return true on one frame until the key is released
        if(!KeyDown((SDL_KeyCode)event.keysym.sym))
            _keysDownThisFrame.push_back((SDL_KeyCode)event.keysym.sym);

        _keyDownStates.insert_or_assign((SDL_KeyCode)event.keysym.sym, true);
    }
    if (event.type == SDL_EventType::SDL_KEYUP)
    {
        _keyDownStates.insert_or_assign((SDL_KeyCode)event.keysym.sym, false);
    }

    _shiftDown = KeyDown(SDL_KeyCode::SDLK_LSHIFT) || KeyDown(SDL_KeyCode::SDLK_RSHIFT);
    _controlDown = KeyDown(SDL_KeyCode::SDLK_LCTRL) || KeyDown(SDL_KeyCode::SDLK_RCTRL);
    _altDown = KeyDown(SDL_KeyCode::SDLK_LALT) || KeyDown(SDL_KeyCode::SDLK_RALT);
    _capsLockDown = KeyDown(SDL_KeyCode::SDLK_CAPSLOCK);
    _numLockDown = KeyDown(SDL_KeyCode::SDLK_CAPSLOCK);
    _scrollLockDown = KeyDown(SDL_KeyCode::SDLK_SCROLLLOCK);
}

void Input::HandleMouseButtonEvent(const SDL_MouseButtonEvent& event)
{
    if (event.type == SDL_EventType::SDL_MOUSEBUTTONDOWN)
    {
        _leftMouseButtonDown = (event.button & SDL_BUTTON_LMASK) != 0;
        _rightMouseButtonDown = (event.button & SDL_BUTTON_RMASK) != 0;
        _middleMouseButtonDown = (event.button & SDL_BUTTON_MMASK) != 0;
    }
    if (event.type == SDL_EventType::SDL_MOUSEBUTTONUP)
    {
        _leftMouseButtonDown = !((event.button & SDL_BUTTON_LMASK) != 0);
        _rightMouseButtonDown = !((event.button & SDL_BUTTON_RMASK) != 0);
        _middleMouseButtonDown = !((event.button & SDL_BUTTON_MMASK) != 0);
    }
}

void Input::HandleMouseMoveEvent(const SDL_MouseMotionEvent& event)
{
    _mousePosX = event.x;
    _mousePosY = event.y;
    _mouseDeltaX = _mousePosX - _lastMouseX;
    _mouseDeltaY = _mousePosY - _lastMouseY;
    _lastMouseX = _mousePosX;
    _lastMouseY = _mousePosY;

    if (abs(_mouseDeltaX) > 0 || abs(_mouseDeltaY) > 0)
        _mouseMoved = true;
}

bool Input::KeyDown(SDL_KeyCode keycode)
{
    //First search for the keycode. The map doesn't add a keycode until that key is pressed
    auto result = _keyDownStates.find(keycode);
    if (result == _keyDownStates.end())
        return false; //If not found then the key was never pressed and isn't down
    else
        return result->second; //If found return key state
}

bool Input::KeyPressed(SDL_KeyCode keycode)
{
    for (SDL_KeyCode key : _keysDownThisFrame)
        if (key == keycode)
            return true;

    return false;
}

bool Input::ShiftDown()
{
    return _shiftDown;
}

bool Input::ControlDown()
{
    return _controlDown;
}

bool Input::AltDown()
{
    return _altDown;
}

bool Input::CapsLockDown()
{
    return _capsLockDown;
}

bool Input::NumLockDown()
{
    return _numLockDown;
}

bool Input::ScrollLockDown()
{
    return _scrollLockDown;
}