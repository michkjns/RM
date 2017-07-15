
#include <core/window.h>

#include <core/debug.h>
#include <GLFW/glfw3.h>

void onResizeCallback(GLFWwindow* window, int32_t width, int32_t height);

class Window_glfw : public Window
{
public:
	Window_glfw();
	~Window_glfw();

	bool initialize(const char* title, Vector2i size) override;
	void terminate() override;
	void setTitle(const char* title) override;
	void swapBuffers() override;
	bool pollEvents() override;
	void onResize(GLFWwindow* window, Vector2i newSize) override;

	Vector2i     getSize()   const override;
	unsigned int getWidth()  const override;
	unsigned int getHeight() const override;

	GLFWwindow* getGLFWwindow() const override;

private:
	GLFWwindow* m_glfwWindow;
	const char* m_title;
	Vector2i    m_size;
};

Window_glfw::Window_glfw() :
	m_glfwWindow(nullptr)
{
}

Window_glfw::~Window_glfw()
{
}

GLFWwindow* Window_glfw::getGLFWwindow() const
{
	return m_glfwWindow;
}

Window* Window::create()
{
	return new Window_glfw();
}

bool Window_glfw::initialize(const char* title, Vector2i size)
{
	assert(title != nullptr);
	assert(size.x > 0);
	assert(size.y > 0);

	m_size = size;

	// Initialize the library
	if (!glfwInit())
	{
		return false;
	}

	// Create a windowed mode window and its OpenGL context
	if ((m_glfwWindow = glfwCreateWindow(m_size.x, m_size.y, title, NULL, NULL)) == false)
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
	glfwSetWindowUserPointer(m_glfwWindow, this);
	glfwSetFramebufferSizeCallback(m_glfwWindow, onResizeCallback);
		
	return true;
}

void Window_glfw::terminate()
{
	LOG_INFO("Window: Terminating GLFW..");
	glfwTerminate();

	// Optional: glfwDestroyWindow(window);
}

void Window_glfw::swapBuffers()
{
	assert(m_glfwWindow);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Window::swapBuffers: OpenGL Error: %d", error);
	}

	glfwSwapBuffers(m_glfwWindow);
}

bool Window_glfw::pollEvents()
{
	glfwPollEvents();

	return (glfwWindowShouldClose(m_glfwWindow) == 1);
}

void Window_glfw::onResize(GLFWwindow* glfwWindow, Vector2i newSize)
{
	assert(glfwWindow == m_glfwWindow);

	glViewport(0, 0, newSize.x, newSize.y);
	m_size = newSize;
}

Vector2i Window_glfw::getSize() const
{
	return m_size;
}

uint32_t Window_glfw::getWidth() const
{
	return m_size.x;
}

uint32_t Window_glfw::getHeight() const
{
	return m_size.y;
}

void Window_glfw::setTitle(const char* title)
{
	glfwSetWindowTitle(m_glfwWindow, title);
}

void onResizeCallback(GLFWwindow* window, int32_t width, int32_t height)
{
	static_cast<Window*>(glfwGetWindowUserPointer(window))->onResize(window, Vector2i(width, height));
}

