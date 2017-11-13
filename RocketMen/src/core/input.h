
#pragma once

#include <common.h>

#include <core/input_buffer.h>
#include <core/keys.h>

class Window;

class Input
{
public:
	static bool initialize(Window* window);
	static void update();

	/** @return true when the key is pressed down */
	static bool getKey(input::Key key);

	/** @return true when the keys are pressed down */
	static bool getKeys(std::initializer_list<input::Key> keys);

	/** @return true when the key pushed the current frame */
	static bool getKeyDown(input::Key key);

	/** @return float2 position of the mouse cursor */
	static Vector2 getMousePosition();
	
	/** @return float2 relative position of the mouse cursor */
	static Vector2 getMouseMovement();

	/** @return true if button is held down */
	static bool getMouse(input::MouseButton button);

	/** @return true if button is pressed this frame */
	static bool getMouseDown(input::MouseButton button);

	static float getAxis(int32_t controllerId, int32_t axis);

	static void getActions(int32_t controllerId, ActionBuffer& inputBuffer);

	/** Binds a key to an action */
	static void mapAction(std::string name, input::Key key, 
		input::ButtonState eventType);

	static void mapAction(std::string name, input::MouseButton mouseButton, 
		input::ButtonState eventType);

	static void mapAction(std::string name, input::ControllerButton controllerButton,
		input::ButtonState eventType, int32_t controllerId);

	/** Enables or disables the cursor */
	static void setCursorEnabled(input::CursorState newState);

	/** @return true if the cursor is enabled */
	static input::CursorState getCursorEnabled();
};
