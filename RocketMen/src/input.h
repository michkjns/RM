
#pragma once

class Window;

class Input
{
public:
#include <keys.h>

	bool initialize(Window* window);
	void update();

	/** @return true when the key is pressed down */
	static bool getKey(Key key);

	/** @return true when the key pushed the current frame */
	static bool getKeyDown(Key key);

	static Input* create();
	static void destroy();
};