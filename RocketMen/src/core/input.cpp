
#include <core/input.h>
#include <common.h>
#include <core/window.h>
#include <core/debug.h>
#include <GLFW/glfw3.h>
#include <utility/utility.h>

#include <algorithm>
#include <functional>
#include <unordered_map>

using namespace input;

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

static ControllerState s_controllerState[NumSupportedControllers];
static std::unordered_map<Key, ActionEvent> s_keyMap;
static std::unordered_map<MouseButton, ActionEvent> s_mouseMap;
static ActionBuffer s_mouseAndKeyboardActions;

static GLFWwindow* s_glfwWindow = nullptr;

// ============================================================================
static void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int glfw_action, int /*mods*/)
{
	ASSERT(window == s_glfwWindow, "callback came from another glswf window instance");

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
		s_mouseAndKeyboardActions.insert(action);
	}
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	ASSERT(window == s_glfwWindow, "Callback came from another glswf window instance");
	s_mousePosxRel = xpos - s_mousePosx;
	s_mousePosyRel = ypos - s_mousePosy;
	s_mousePosx = xpos;
	s_mousePosy = ypos;
}

static void mouseButtonCallback(GLFWwindow* window, int button, int glfw_action, int /*mods*/)
{
	ASSERT(window == s_glfwWindow, "Callback came from another glswf window instance");

	const bool repeat = s_prevMouseState[button] != ButtonState::Release && glfw_action == GLFW_PRESS;
	const ButtonState state = repeat ? ButtonState::Repeat : ButtonState(glfw_action);
	s_mouseState[button] = state;
	s_prevMouseState[button] = state;

	auto it = s_mouseMap.find(MouseButton(button));
	if (it != s_mouseMap.end() && it->second.inputEvent == state)
	{
		Action action;
		action.set(it->second.hashedActionName, it->second.inputEvent);
		s_mouseAndKeyboardActions.insert(action);
	}
}

static void controllerCallback(int joy, int /*event*/)
{
	s_controllerState[joy] = ControllerState {};
}

static void refreshControllerStates()
{
	for (ControllerId controllerId = GLFW_JOYSTICK_1; controllerId < input::NumSupportedControllers; controllerId++)
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

bool input::initialize(Window* window)
{
	ASSERT(window != nullptr);
	s_glfwWindow = window->getGLFWwindow();
	ASSERT(s_glfwWindow != nullptr);

	glfwSetKeyCallback(s_glfwWindow, keyCallback);
	glfwSetCursorPosCallback(s_glfwWindow, cursorPositionCallback);
	glfwSetMouseButtonCallback(s_glfwWindow, mouseButtonCallback);
	glfwSetJoystickCallback(controllerCallback);

	return true;
}

void input::update()
{
	s_mousePosxRel = s_mousePosyRel = 0;
	s_mouseAndKeyboardActions.clear();
	refreshControllerStates();
}

bool input::getKey(Key key)
{
	return s_keyState[static_cast<int32_t>(key)] == ButtonState::Press;
}

bool input::getKeys(std::initializer_list<input::Key> keys)
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

bool input::getKeyDown(Key key)
{
	return s_keyState[(int)key] == ButtonState::Repeat;
}

Vector2 input::getMousePosition()
{
	return Vector2(s_mousePosx, s_mousePosy);
}

Vector2 input::getMouseMovement()
{
	return Vector2(s_mousePosxRel, s_mousePosyRel);
}

bool input::getMouse(MouseButton button)
{
	return s_mouseState[static_cast<int32_t>(button)] == ButtonState::Press;
}

bool input::getMouseDown(MouseButton button)
{
	return s_mouseState[static_cast<int32_t>(button)] == ButtonState::Release;
}

float input::getAxis(ControllerId controllerId, int32_t axis)
{
	ASSERT(controllerId >= 0, "controllerId cannot be negative");
	ASSERT(controllerId < input::NumSupportedControllers, "controllerId exceeds maxControllers");
	ASSERT(axis >= 0, "axis cannot be negativ");

	if (s_controllerState[controllerId].numAxes <= 0
		|| axis >= s_controllerState[controllerId].numAxes)
	{
		return 0.f;
	}
	return s_controllerState[controllerId].axisStates[axis];
}

void input::getActions(ControllerId controllerId, ActionBuffer& inputBuffer, bool includeMouseAndKeyboard)
{
	ASSERT(controllerId >= 0, "controllerId cannot be negative");
	if (controllerId != Controller::MouseAndKeyboard)
	{
		ControllerState& controller = s_controllerState[controllerId];
		for (auto mapping : controller.buttonMap)
		{
			if (controller.currentButtonStates[mapping.first] == mapping.second.inputEvent)
			{
				Action action;
				action.set(mapping.second.hashedActionName, mapping.second.inputEvent);
				inputBuffer.insert(action);
			}
		}
	}
	if (includeMouseAndKeyboard || controllerId == Controller::MouseAndKeyboard)
	{
		inputBuffer.insert(s_mouseAndKeyboardActions);
	}
}

void input::mapAction(std::string name, Key key, ButtonState inputEvent)
{
	std::hash<std::string> strHash;
	s_keyMap[key] = { inputEvent, strHash(toLower(name)) };
}

void input::mapAction(std::string name, MouseButton mouseButton, ButtonState inputEvent)
{
	std::hash<std::string> strHash;
	s_mouseMap[mouseButton] = { inputEvent, strHash(toLower(name)) };
}

void input::mapAction(std::string name, ControllerButton controllerButton, ButtonState inputEvent, const ControllerId controllerId)
{
	ASSERT(controllerId != Controller::MouseAndKeyboard, "Cannot map ControllerButtons to the MouseAndKeyboard controller");
	ASSERT(controllerId >= 0 && controllerId < input::NumSupportedControllers, "Invalid controllerId");

	std::hash<std::string> strHash;
	s_controllerState[controllerId].buttonMap[controllerButton] = { inputEvent, strHash(toLower(name)) };
}

void input::setCursorEnabled(CursorState setEnabled)
{
	glfwSetInputMode(s_glfwWindow, GLFW_CURSOR, static_cast<int32_t>(setEnabled) + GLFW_CURSOR_NORMAL);
}

CursorState input::getCursorEnabled()
{
	return static_cast<CursorState>(glfwGetInputMode(s_glfwWindow, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
}
