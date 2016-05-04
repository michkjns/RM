
#pragma once

#include <includes.h>
#include <keys.h>

class Window;

class Input
{
public:

	bool initialize(Window* window);
	void update();

	/** @return true when the key is pressed down */
	static bool getKey(input::Key key);

	/** @return true when the key pushed the current frame */
	static bool getKeyDown(input::Key key);

	/** @return float2 position of the mouse cursor */
	static float2 getMousePosition();
	
	/** @return true if button is held down */
	static bool getMouse(input::MouseButton button);

	/** @return true if button is pressed this frame */
	static bool getMouseDown(input::MouseButton button);

	static Input* create();
	static void destroy();
};