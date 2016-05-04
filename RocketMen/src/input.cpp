
#include <input.h>
#include <window.h>

#include <GLFW/glfw3.h>

#include <assert.h>
#include <algorithm>

using namespace input;

static bool s_keyState[348];
static bool s_keyStateDown[348];

static bool s_mouseState[8];
static bool s_mouseStateDown[8];

static double s_mousePosx;
static double s_mousePosy;

Input* s_instance = nullptr;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == -1) key = 347;
	if (action == GLFW_PRESS)
	{
		s_keyState[key]     = true;
		s_keyStateDown[key] = true;
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
	GLFWwindow* glfwWindow = (GLFWwindow*)window->getGLFWwindow();
	assert(glfwWindow != nullptr);

	glfwSetKeyCallback(glfwWindow, key_callback);
	glfwSetCursorPosCallback(glfwWindow, cursor_position_callback);
	glfwSetMouseButtonCallback(glfwWindow, mouse_button_callback);

	//glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return true;
}

void Input::update()
{
	std::fill(std::begin(s_keyStateDown),   std::end(s_keyStateDown),   false);
	std::fill(std::begin(s_mouseStateDown), std::end(s_mouseStateDown), false);
}

bool Input::getKey(Input::Key key)
{
	return s_keyState[(int)key];
}

bool Input::getKeyDown(Input::Key key)
{
	return s_keyStateDown[(int)key];
}

float2 Input::getMousePosition()
{
	return float2(s_mousePosx, s_mousePosy);
}

bool Input::getMouse(MouseButton button)
{
	return s_mouseState[(int)button];
}

bool Input::getMouseDown(MouseButton button)
{
	return s_mouseStateDown[(int)button];
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
