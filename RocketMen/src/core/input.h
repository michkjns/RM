
#pragma once

#include <common.h>

#include <core/input_buffer.h>
#include <core/keys.h>

class Window;
namespace input {

	bool initialize(Window* window);
	void update();

	/** @return true when the key is pressed down */
	bool getKey(Key key);

	/** @return true when the keys are pressed down */
	bool getKeys(std::initializer_list<Key> keys);

	/** @return true when the key pushed the current frame */
	bool getKeyDown(Key key);

	/** @return float2 position of the mouse cursor */
	Vector2 getMousePosition();
	
	/** @return float2 relative position of the mouse cursor */
	Vector2 getMouseMovement();

	/** @return true if button is held down */
	bool getMouse(MouseButton button);

	/** @return true if button is pressed this frame */
	bool getMouseDown(MouseButton button);

	float getAxis(ControllerId controllerId, int32_t axis);

	void getActions(ControllerId controllerId, ActionBuffer& inputBuffer,
		bool includeMouseAndKeyboard);

	/** Binds a key to an action */
	void mapAction(std::string name, Key key, 
		ButtonState eventType);

	void mapAction(std::string name, MouseButton mouseButton, 
		ButtonState eventType);

	void mapAction(std::string name, ControllerButton controllerButton,
		ButtonState eventType, const ControllerId controllerId);

	/** Enables or disables the cursor */
	void setCursorEnabled(CursorState newState);

	/** @return true if the cursor is enabled */
	CursorState getCursorEnabled();

};// namespace input
