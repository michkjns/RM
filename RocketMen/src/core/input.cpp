
#include <core/input.h>
#include <common.h>
#include <core/window.h>
#include <core/debug.h>
#include <GLFW/glfw3.h>
#include <utility.h>

#include <algorithm>
#include <functional>
#include <unordered_map>

using namespace input;

const uint32_t s_numMaxControllers = 4;
const int32_t s_maxControllerButtons = 16;

/** Raw Input Data */
static ButtonState s_keyState[348];
static ButtonState s_prevMouseState[8];
static ButtonState s_mouseState[8];
static double      s_mousePosx;
static double      s_mousePosy;
static double      s_mousePosxRel;
static double      s_mousePosyRel;

struct ActionEvent
{
	ButtonState inputEvent;
	size_t hashedActionName;
};

struct ControllerState
{
	ButtonState currentButtonStates[s_maxControllerButtons];
	ButtonState prevButtonStates[s_maxControllerButtons];

	const float* axisStates;
	int32_t numAxes;

	std::unordered_map<ControllerButton, ActionEvent> buttonMap;
};

static ControllerState s_controllerState[s_numMaxControllers];
static std::unordered_map<Key, ActionEvent> s_keyMap;
static std::unordered_map<MouseButton, ActionEvent> s_mouseMap;
static ActionBuffer s_actions;

static GLFWwindow* s_glfwWindow = nullptr;

// ============================================================================
static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int glfw_action, int /*mods*/)
{
	assert(window == s_glfwWindow);
//	LOG_DEBUG("%i", key);

	if (key == GLFW_KEY_UNKNOWN)
	{
		return;
	}

	s_keyState[key] = glfw_action == GLFW_REPEAT ? ButtonState::Repeat :
		glfw_action == GLFW_PRESS ? ButtonState::Press :
		ButtonState::Release;

	auto it = s_keyMap.find(Key(key));
	if (it != s_keyMap.end())
	{
		Action action;
		action.set(it->second.hashedActionName, it->second.inputEvent);
		s_actions.insert(action);
	}
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	assert(window == s_glfwWindow);
	s_mousePosxRel = xpos - s_mousePosx;
	s_mousePosyRel = ypos - s_mousePosy;
	s_mousePosx = xpos;
	s_mousePosy = ypos;
}

static void mouseButtonCallback(GLFWwindow* window, int button, int glfw_action, int /*mods*/)
{
	assert(window == s_glfwWindow);

	const bool repeat = s_prevMouseState[button] != ButtonState::Release && glfw_action == GLFW_PRESS;
	const ButtonState state = repeat ? ButtonState::Repeat : ButtonState(glfw_action);
	s_mouseState[button] = state;
	s_prevMouseState[button] = state;

	auto it = s_mouseMap.find(MouseButton(button));
	if (it != s_mouseMap.end() && it->second.inputEvent == state)
	{
		Action action;
		action.set(it->second.hashedActionName, it->second.inputEvent);
		s_actions.insert(action);
	}
}

static void controllerCallback(int joy, int /*event*/)
{
	s_controllerState[joy] = ControllerState {};
}

static void refreshControllerStates()
{
	for (int32_t controllerId = GLFW_JOYSTICK_1; controllerId < s_numMaxControllers; controllerId++)
	{
		if(!glfwJoystickPresent(controllerId))
			continue;

		ControllerState& controllerState = s_controllerState[controllerId];
		int32_t numButtons = 0;
		const unsigned char* glfw_buttonState = glfwGetJoystickButtons(controllerId, &numButtons);
		if (numButtons != 0)
		{
			const int32_t limit = glm::min(numButtons, s_maxControllerButtons);
			for (int32_t i = 0; i < limit; i++)
			{
				if (controllerState.prevButtonStates[i] != ButtonState::Release
					&& glfw_buttonState[i] == ButtonState::Press)
				{
					controllerState.currentButtonStates[i] = ButtonState::Repeat;
				}
				else
				{
					controllerState.currentButtonStates[i] = ButtonState(glfw_buttonState[i]);
				}
				controllerState.prevButtonStates[i] = ButtonState(glfw_buttonState[i]);
			}
		}

		controllerState.axisStates = glfwGetJoystickAxes(controllerId, &controllerState.numAxes);
		//LOG_DEBUG("%d %d", controllerId, controllerState.numAxes);
	}
}

bool Input::initialize(Window* window)
{
	if (assert(window != nullptr))
	{
		s_glfwWindow = window->getGLFWwindow();
		assert(s_glfwWindow != nullptr);

		glfwSetKeyCallback(s_glfwWindow, keyCallback);
		glfwSetCursorPosCallback(s_glfwWindow, cursorPositionCallback);
		glfwSetMouseButtonCallback(s_glfwWindow, mouseButtonCallback);
		glfwSetJoystickCallback(controllerCallback);

		return true;
	}
	return false;
}

void Input::update()
{
	s_mousePosxRel = s_mousePosyRel = 0;
	s_actions.clear();
	refreshControllerStates();
}

bool Input::getKey(Key key)
{
	return s_keyState[static_cast<int32_t>(key)] == ButtonState::Press;
}

bool Input::getKeys(std::initializer_list<input::Key> keys)
{
	for (auto key : keys)
	{
		if (!s_keyState[static_cast<int32_t>(key)] == ButtonState::Press)
		{
			return false;
		}
	}

	return true;
}

bool Input::getKeyDown(Key key)
{
	return s_keyState[(int)key] == ButtonState::Repeat;
}

Vector2 Input::getMousePosition()
{
	return Vector2(s_mousePosx, s_mousePosy);
}

Vector2 Input::getMouseMovement()
{
	return Vector2(s_mousePosxRel, s_mousePosyRel);
}

bool Input::getMouse(MouseButton button)
{
	return s_mouseState[static_cast<int32_t>(button)] == ButtonState::Press;
}

bool Input::getMouseDown(MouseButton button)
{
	return s_mouseState[static_cast<int32_t>(button)] == ButtonState::Release;
}

float Input::getAxis(int32_t controllerId, int32_t axis)
{
	assert(controllerId >= 0);
	assert(controllerId < s_numMaxControllers);
	assert(axis >= 0);

	if (s_controllerState[controllerId].numAxes <= 0
		|| axis >= s_controllerState[controllerId].numAxes)
	{
		return 0.f;
	}
	return s_controllerState[controllerId].axisStates[axis];
}

void Input::getActions(int32_t controllerId, ActionBuffer& inputBuffer)
{
	assert(controllerId >= 0);
	ControllerState& controller = s_controllerState[controllerId];

	inputBuffer.insert(s_actions);

	for (auto mapping : controller.buttonMap)
	{
		if(controller.currentButtonStates[mapping.first] == mapping.second.inputEvent)
		{
			Action action;
			action.set(mapping.second.hashedActionName, mapping.second.inputEvent);
			inputBuffer.insert(action);
		}
	}	
}

void Input::mapAction(std::string name, Key key, ButtonState inputEvent)
{
	std::hash<std::string> strHash;
	s_keyMap[key] = { inputEvent, strHash(toLower(name)) };
}

void Input::mapAction(std::string name, MouseButton mouseButton, ButtonState inputEvent)
{
	std::hash<std::string> strHash;
	s_mouseMap[mouseButton] = { inputEvent, strHash(toLower(name)) };
}

void Input::mapAction(std::string name, ControllerButton controllerButton, ButtonState inputEvent, int32_t controllerId)
{
	assert(controllerId >= 0 && controllerId < s_numMaxControllers);

	std::hash<std::string> strHash;
	s_controllerState[controllerId].buttonMap[controllerButton] = { inputEvent, strHash(toLower(name)) };
}

void Input::setCursorEnabled(CursorState setEnabled)
{
	glfwSetInputMode(s_glfwWindow, GLFW_CURSOR, static_cast<int32_t>(setEnabled) + GLFW_CURSOR_NORMAL);
}

CursorState Input::getCursorEnabled()
{
	return static_cast<CursorState>(glfwGetInputMode(s_glfwWindow, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
}
