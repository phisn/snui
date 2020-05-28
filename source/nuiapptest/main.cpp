#include "common.h"
#include "snui/nuiapp.h"

#include "glfw3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui.h"

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
	std::cout << "glfw error [" << description << "] (" << error << ")" << std::endl;
	exit(-1);
}

int main()
{
	std::string alert = "01/01-00:00:00.999999  [**] [122:5:1] (portscan) TCP Filtered Portscan [**] [Classification: Attempted Information Leak] [Priority: 2] {PROTO:255} 8.8.8.8 -> 8.8.4.4";
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
	{
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(
		//////////////////////////////////////////////
		// window size:
		//     x
		//     y
		//////////////////////////////////////////////
		400,
		200,
		SNUI_APP_NAME,
		NULL,
		NULL);

	if (window == NULL)
	{
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	if (!ImGui_ImplGlfw_InitForOpenGL(window, true) ||
		!ImGui_ImplOpenGL2_Init())
	{
		return 1;
	}

	nui::App app;
	app.updateAlert(alert);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		app.process();

		ImGui::Render();

		int displayWidth, displayHeight;
		glfwGetFramebufferSize(window,
			&displayWidth,
			&displayHeight);
		glViewport(0, 0, displayWidth, displayHeight);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
