
#include <input.h>
#include <window.h>

#include <GLFW/glfw3.h>

#include <assert.h>
#include <algorithm>

static bool s_keyState[348];
static bool s_keyStateDown[348];

Input* s_instance = nullptr;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
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

bool Input::initialize(Window* window)
{
	assert(window != nullptr);
	GLFWwindow* glfwWindow = (GLFWwindow*)window->getGLFWwindow();
	assert(glfwWindow != nullptr);

	glfwSetKeyCallback(glfwWindow, key_callback);
	return true;
}

void Input::update()
{
	std::fill(std::begin(s_keyStateDown), std::end(s_keyStateDown), false);
}

bool Input::getKey(Input::Key key)
{
	return s_keyState[(int)key];
}

bool Input::getKeyDown(Input::Key key)
{
	return s_keyStateDown[(int)key];
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
