#include "Common.h"
#include "configuration.h"
#include "nui.h"
#include "nuiapp.h"

#include "glfw3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui.h"

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

#include <fstream>
#include <future>
#include <iostream>
#include <thread>

namespace
{
	std::thread nuiThread;
	GLFWwindow* window;

	std::mutex mtx;
	std::unique_lock<std::mutex> lock{ mtx };
	std::condition_variable condition;

	nui::App app;
}

namespace nui
{
	static void glfw_error_callback(int error, const char* description)
	{
		std::cerr << "glfw failed (" << error << "): " << description;
		exit(-507);
	}

	void snui_main(std::promise<bool>* initPromise)
	{
		glfwSetErrorCallback(glfw_error_callback);

		if (!glfwInit())
		{
			initPromise->set_value(false);
			return;
		}

		window = glfwCreateWindow(
			400,
			200,
			SNUI_APP_NAME,
			NULL,
			NULL);

		if (window == NULL)
		{
			initPromise->set_value(false);
			return;
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		if (!ImGui_ImplGlfw_InitForOpenGL(window, true) ||
			!ImGui_ImplOpenGL2_Init())
		{
			initPromise->set_value(false);
			return;
		}

		// init promise is invalid from now on
		initPromise->set_value(true);

		while (true)
		{
			glfwHideWindow(window);
			condition.wait(lock);
			glfwShowWindow(window);

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

			glfwSetWindowShouldClose(window, GLFW_FALSE);
		}

		ImGui_ImplOpenGL2_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool initialize()
	{
		std::promise<bool> initPromise;
		std::future<bool> initResult = initPromise.get_future();
		nuiThread = std::thread(snui_main, &initPromise);
		
		initResult.wait();
		return initResult.get();
	}

	bool readAlertFile(std::string& line)
	{
		std::ifstream alertFile(Configuration::alertPath);

		if (!alertFile)
		{
			std::cerr << "Open alert file failed: " << strerror(errno) << std::endl;
			return false;
		}

		if (!alertFile.seekg(-3, std::ios_base::end))
		{
			// ignore empty file
			return true;
		}

		while (true)
		{
			char c;
			if (!alertFile.get(c))
				break;

			if (alertFile.tellg() <= 1)
				return true;

			if (c == '\n')
				break;

			alertFile.seekg(-2, std::ios_base::cur);
		}
		
		if (!alertFile)
		{
			std::cerr << "Reverse read alert file failed: " << strerror(errno) << std::endl;
			return false;
		}

		if (!std::getline(alertFile, line))
		{
			std::cerr << "Read alert file failed: " << strerror(errno) << std::endl;
			return false;
		}

		return true;
	}

	bool notify()
	{
		std::string lastAlertLine;
		if (!readAlertFile(lastAlertLine))
			return false;

		// ignore manuall changes by user
		if (!app.updateAlert(lastAlertLine))
			return true;

		condition.notify_one();
		return true;
	}
}
