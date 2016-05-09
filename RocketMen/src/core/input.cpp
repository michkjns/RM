
#include <core/input.h>
#include <includes.h>
#include <core/window.h>

#include <GLFW/glfw3.h>

#include <assert.h>
#include <algorithm>
#include <functional>
#include <unordered_map>

using namespace input;

/** Raw Input Data */
static bool s_keyState[348];
static bool s_keyStateDown[348];
static bool s_mouseState[8];
static bool s_mouseStateDown[8];
static double s_mousePosx;
static double s_mousePosy;


static int32_t s_actionCount;

static ActionBuffer s_actionBuffer;
static std::vector<Action> s_events;
static std::unordered_map<Key, size_t> s_map;

GLFWwindow* s_glfwWindow = nullptr;
Input*      s_instance = nullptr;

//==========================================================================
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == -1) key = 347;
	if (action == GLFW_PRESS)
	{
		s_keyState[key]     = true;
		s_keyStateDown[key] = true;

		auto it = s_map.find(Key(key));
		if (it != s_map.end())
		{
			Action action;
			action.set(it->second, true);
			s_actionBuffer.insert(action);
		}
	}
	else if (action == GLFW_RELEASE)
	{
		s_keyState[key]     = false;
		s_keyStateDown[key] = false;
	}
	else if (action == GLFW_REPEAT)
	{
		s_keyState[key]     = true;
		s_keyStateDown[key] = false;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	s_mousePosx = xpos;
	s_mousePosy = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		s_mouseState[button]     = true;
		s_mouseStateDown[button] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		s_mouseState[button]     = false;
		s_mouseStateDown[button] = false;
	}
}

bool Input::initialize(Window* window)
{
	assert(window != nullptr);
	s_glfwWindow = (GLFWwindow*)window->getGLFWwindow();
	assert(s_glfwWindow != nullptr);

	glfwSetKeyCallback(s_glfwWindow, key_callback);
	glfwSetCursorPosCallback(s_glfwWindow, cursor_position_callback);
	glfwSetMouseButtonCallback(s_glfwWindow, mouse_button_callback);

	return true;
}

void Input::update()
{
	std::fill(std::begin(s_keyStateDown),   std::end(s_keyStateDown),   false);
	std::fill(std::begin(s_mouseStateDown), std::end(s_mouseStateDown), false);

}

bool Input::getKey(Key key)
{
	return s_keyState[static_cast<int32_t>(key)];
}

bool Input::getKeys(std::initializer_list<input::Key> keys)
{
	for (auto key : keys)
	{
		if (!s_keyState[static_cast<int32_t>(key)])
		{
			return false;
		}
	}

	return true;
}

bool Input::getKeyDown(Key key)
{
	return s_keyStateDown[(int)key];
}

float2 Input::getMousePosition()
{
	return float2(s_mousePosx, s_mousePosy);
}

bool Input::getMouse(MouseButton button)
{
	return s_mouseState[static_cast<int32_t>(button)];
}

bool Input::getMouseDown(MouseButton button)
{
	return s_mouseStateDown[static_cast<int32_t>(button)];
}

void Input::mapAction(std::string name, Key key)
{
	std::hash<std::string> strHash;
	s_map[key] = strHash(toLower(name));

//	LOG_DEBUG("Action mapped: %s - %i", name, static_cast<int32_t>(key));
}

void Input::setCursorEnabled(CursorState setEnabled)
{
	glfwSetInputMode(s_glfwWindow, GLFW_CURSOR, static_cast<int32_t>(setEnabled) + GLFW_CURSOR_NORMAL);
}

CursorState Input::getCursorEnabled()
{
	return static_cast<CursorState>(glfwGetInputMode(s_glfwWindow, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
}

ActionBuffer& Input::getActions()
{
	return s_actionBuffer;
}

Input* Input::create()
{
	if (s_instance == nullptr)
		s_instance = new Input();

	return s_instance;
}

void Input::destroy()
{
	if (s_instance)
		delete s_instance;
}
