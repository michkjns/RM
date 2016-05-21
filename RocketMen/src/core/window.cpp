
#include <includes.h>
#include <core/window.h>

#include <assert.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

void onResize(GLFWwindow* window, int32_t width, int32_t height);

class Window_impl : public Window
{
public:
	Window_impl();
	~Window_impl();

	bool initialize(uint32_t width, uint32_t height) override;
	void terminate()   override;
	void swapBuffers() override;
	bool pollEvents()  override;

	unsigned int getWidth()  const override;
	unsigned int getHeight() const override;


	//int32_t getFrameWidth()  const override;

	void* getGLFWwindow()    const override;

private:
	GLFWwindow*  m_glfwWindow;
	uint32_t     m_width;
	uint32_t     m_height;
};

Window_impl::Window_impl() :
	m_glfwWindow(nullptr),
	m_width(600),
	m_height(480)
{
}

Window_impl::~Window_impl()
{
}

void* Window_impl::getGLFWwindow() const
{
	return (void*)m_glfwWindow;
}

//int32_t Window::getFrameWidth() const
//{
//	return int32_t();
//}
//
//int32_t Window::getFrameHeight() const
//{
//	return int32_t();
//}

Window* Window::create()
{
	return new Window_impl();
}

bool Window_impl::initialize(uint32_t width, uint32_t height)
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

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		LOG_ERROR("Renderer: Failed to initialize GLEW");
		glfwTerminate();
		exit(-1);
	}

	glfwSetFramebufferSizeCallback(m_glfwWindow, onResize);
		
	return true;
}

void Window_impl::terminate()
{
	LOG_INFO("Window: Terminating GLFW..");
	glfwTerminate();

	// Optional: glfwDestroyWindow(window);
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

uint32_t Window_impl::getWidth() const
{
	return m_width;
}

uint32_t Window_impl::getHeight() const
{
	return m_height;
}

void onResize(GLFWwindow* window, int32_t width, int32_t height)
{
	glViewport(0, 0, width, height);
}
