#pragma once
#include "glfw/include/GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl2.h"
#include "stb_image.h"

#include <random>
#include <time.h>

int WindowWidth = 409;
int WindowHeight = 407+27;
int HelpWindowWidth = 300;
int HelpWindowHeight = 300;
int LogoWidth = 220;
int LogoHeight = 40;

////////////////////////////////////////////
///////////////// Utils ////////////////////
////////////////////////////////////////////

#pragma region Utils

static std::string random_string(int length)
{
	std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	std::random_device rd;
	std::mt19937 generator(rd());
	std::shuffle(str.begin(), str.end(), generator);
	return str.substr(0, length);    // assumes 32 < number of characters in str         
}

static DWORDLONG random_number(DWORDLONG min, DWORDLONG max)
{
	srand(time(0));
	DWORDLONG out = min + rand() % (max - min + 1);
	return out;
}

std::string GetCurrentPath() 
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

std::string GetTime()
{
	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
	std::string str(buffer);
	return str;
}
#pragma endregion


////////////////////////////////////////////
////////////// OpenGL Shit /////////////////
////////////////////////////////////////////

#pragma region OpenGLShit

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL) return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

bool LoadTextureFromMemory(unsigned char* MappedFile, int FileSize, GLuint* out_texture, int* out_width, int* out_height)
{
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory(MappedFile, FileSize, &image_width, &image_height, NULL, 4);

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}


void GetMyStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	{
		style.Alpha = 1.f;
		style.WindowPadding = ImVec2(5, 5);
		style.WindowRounding = 0.0f;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.ChildRounding = 3.0f;
		style.FrameBorderSize = 1;
		style.FramePadding = ImVec2(4, 3);
		style.FrameRounding = 5;
		style.ItemSpacing = ImVec2(8, 0);
		style.ItemInnerSpacing = ImVec2(4, 4);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 10.0f;
		style.ScrollbarRounding = 3.0f;
		style.GrabMinSize = 20.0f;
		style.GrabRounding = 3.0f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.DisplayWindowPadding = ImVec2(22, 22);
		style.DisplaySafeAreaPadding = ImVec2(4, 4);
		style.AntiAliasedLines = true;

		// Setup style
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);						//Цвет текста
		colors[ImGuiCol_TextDisabled] = ImColor(53, 53, 53, 255);
		colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 255);						// Задний фон окна
		colors[ImGuiCol_ChildBg] = ImColor(15, 15, 15, 255);						// Задний фон "ребенка" (Child)
		colors[ImGuiCol_PopupBg] = ImColor(25, 25, 25, 255);
		colors[ImGuiCol_Border] = ImColor(36, 35, 35, 255);							//Обводка прямоуголника приложения и айтемом
		colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 200);						//Тень от обводки
		colors[ImGuiCol_FrameBg] = ImColor(20, 20, 20, 255);						// Задний фон слайдеров, кнопок и прочей херни
		colors[ImGuiCol_FrameBgHovered] = ImColor(140, 0, 0, 255);					// Когда наведен курсор
		colors[ImGuiCol_FrameBgActive] = ImColor(255, 0, 0, 175);					//Когда нажато
		colors[ImGuiCol_TitleBg] = ImColor(15, 15, 15, 255);						//Верхняя полоска с название программы когда окно неактивно
		colors[ImGuiCol_TitleBgActive] = ImColor(20, 20, 20, 255);					//Когда активно
		colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 255);
		colors[ImGuiCol_MenuBarBg] = ImColor(25, 25, 25, 255);
		colors[ImGuiCol_ScrollbarBg] = ImColor(20, 20, 20, 255);					//Ползунок(ScrollBar) задний фон
		colors[ImGuiCol_ScrollbarGrab] = ImColor(242, 0, 0, 232);					//Ползунок(ScrollBar) при нажатии??
		colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(241, 0, 0, 174);			//Ползунок(ScrollBar) при наведении	
		colors[ImGuiCol_ScrollbarGrabActive] = ImColor(255, 0, 0, 133);				//Ползунок(ScrollBar) при перетягивании	
		colors[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 253);						//Галочка (CheckBox)	//ImVec4(0.8f, 0.15f, 0.15f, 1.00f);  //CheckBox 
		colors[ImGuiCol_SliderGrab] = ImColor(246, 0, 0, 255);						// Слайдер когда нажимаем?
		colors[ImGuiCol_SliderGrabActive] = ImColor(255, 0, 0, 175);				// Слайдер когда перетягиваем
		colors[ImGuiCol_Button] = ImColor(20, 20, 20, 249);							// Кнопка 
		colors[ImGuiCol_ButtonHovered] = ImColor(140, 0, 0, 223);					// Кнопка при наведении
		colors[ImGuiCol_ButtonActive] = ImColor(0, 0, 0, 168);				        // Кнопка при нажамтии
		colors[ImGuiCol_Header] = ImColor(255, 0, 0, 175);							// Выбранный айтем из комбобокса
		colors[ImGuiCol_HeaderHovered] = ImColor(244, 0, 0, 223);
		colors[ImGuiCol_HeaderActive] = ImColor(255, 0, 0, 255);
		colors[ImGuiCol_Separator] = ImColor(0, 0, 0, 78);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	}
}

#pragma endregion