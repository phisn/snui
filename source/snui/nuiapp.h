#pragma once

#include "FastAlert.h"

#include "imgui/imgui.h"

namespace nui
{
	class App
	{
	public:
		void process()
		{
			ImGui::Begin("Alert", 0, ImGuiWindowFlags_NoResize);
			ImGui::Text("Type: %s", alert.type.c_str());
			ImGui::Text("Info: %s", alert.info.c_str());
			ImGui::End();
		}

		bool updateAlert(std::string& rawAlert)
		{
			return alert.parse(rawAlert);
		}

	private:
		FastAlert alert;
	};
}
