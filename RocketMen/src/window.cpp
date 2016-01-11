
#include "includes.h"
#include "window.h"

#include <assert.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

class Window_impl : public Window
{
public:
	Window_impl();
	~Window_impl();

	bool initialize(unsigned int width, unsigned int height) override;
	void terminate() override;
	void swapBuffers() override;
	bool pollEvents() override;

	unsigned int getWidth() const override;
	unsigned int getHeight() const override;


private:
	GLFWwindow* m_glfwWindow;
	unsigned int m_width;
	unsigned int m_height;
};

Window_impl::Window_impl()
	: m_glfwWindow(nullptr)
	, m_width(600)
	, m_height(480)
{
}

Window_impl::~Window_impl()
{
}

Window* Window::create()
{
	return new Window_impl();
}

bool Window_impl::initialize(unsigned int width, unsigned int height)
{
	assert(width > 0);
	assert(height > 0);

	m_width = width;
	m_height = height;

	// Initialize the library
	if (!glfwInit())
	{
		return false;
	}

	// Create a windowed mode window and its OpenGL context
	m_glfwWindow = glfwCreateWindow(m_width, m_height, "Rocket Men", NULL, NULL);
	if (!m_glfwWindow)
	{
		glfwTerminate();
		LOG_ERROR("Window: Failed to initialize glfwWindow");
		return false;
	}

	// Make the window's context current
	glfwMakeContextCurrent(m_glfwWindow);
	
	return true;
}

void Window_impl::terminate()
{
	LOG_INFO("Window: Terminating GLFW..");
	glfwTerminate();
}

void Window_impl::swapBuffers()
{
	assert(m_glfwWindow);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Window::swapBuffers: OpenGL Error: %d", error);
	}

	glfwSwapBuffers(m_glfwWindow);
}

bool Window_impl::pollEvents()
{
	glfwPollEvents();

	bool closeWindow = (glfwWindowShouldClose(m_glfwWindow) == 1);
	if (closeWindow)
	{
		LOG_DEBUG("Window: GLFW says the window is closing..");
	}
	return closeWindow;
}

unsigned int Window_impl::getWidth() const
{
	return m_width;
}

unsigned int Window_impl::getHeight() const
{
	return m_height;
}
