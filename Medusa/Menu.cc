#include "Menu.h"

void Draw()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	
	ImGui::SetNextWindowSize(ImVec2(1672, 931));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (ImGui::Begin("window_name", 0, ImGuiWindowFlags_NoTitleBar))
	{

		ImGui::SetCursorPos(ImVec2(65, 153));
		ImGui::PushItemWidth(200);
		/*static int item_current1 = 0;
		const char* items1[] = { "Never", "Gonna", "Give", "You", "Up" };
		ImGui::ListBox("##", &item_current1, items1, IM_ARRAYSIZE(items1));
		ImGui::PopItemWidth();
		if (ImGui::IsItemClicked(1))
		{
			ImGui::Text(items1[item_current1]);
		}*/
		//ImGui::IsItemHovered();
		ImGui::BeginListBox("##");
		bool temp;
		ImGui::Selectable("#", &temp);
		ImGui::EndListBox();



		ImGui::SetCursorPos(ImVec2(95, 74));
		ImGui::Button("process", ImVec2(84, 36)); //remove size argument (ImVec2) to auto-resize

	}
	ImGui::End();

	ImGui::Render();
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
}