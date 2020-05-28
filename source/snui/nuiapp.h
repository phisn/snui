#pragma once

#include "common.h"
#include "fastalert.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

namespace nui
{
	class App
	{
	public:
		void notifyClose()
		{
		}

		bool updateAlert(std::string& rawAlert)
		{
			return alert.parse(rawAlert);
		}

		void process()
		{
			ImGui::Begin("Alert", 0, ImGuiWindowFlags_NoResize);
			ImGui::Text("Type: %s", alert.type.c_str());
			ImGui::Text("Info: %s", alert.info.c_str());
			ImGui::End();
		}

	private:
		FastAlert alert;
	};
}
