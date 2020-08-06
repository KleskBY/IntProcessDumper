#define STB_IMAGE_IMPLEMENTATION
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma comment(lib, "legacy_stdio_definitions")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw\\lib-vc2010-32\\glfw3.lib")
#pragma comment(lib, "Wtsapi32.lib")

#include <Windows.h>
#include <vector>
#include <string>
#include "glfw/include/GLFW/glfw3.h"
#include "main.h"

#include <wtsapi32.h>
#include "Dumper.h"
#include "logo.h"

struct ProcessInfo
{
    DWORD PID;
    std::string ProcName;
};
std::vector<ProcessInfo> ListOfProcesses;
ImGuiTextBuffer     Buf;
ImVector<int>       LineOffsets; 

int selected = -1;
char SearchFor[100] = "";
char FolderPath[MAX_PATH] = "";
bool SetWidthOnce = true;
bool Dumping = false;
bool Logging = true;
bool AutoScroll = true;
bool ShowHelp = false;


void AddLog(const char* fmt, ...) IM_FMTARGS(2)
{
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
        if (Buf[old_size] == '\n')
            LineOffsets.push_back(old_size + 1);
    std::cout << fmt ;

}

void LogError(std::string error)
{
    AddLog(std::string("[" + GetTime() + "] ERROR:" + error).c_str());
    MessageBoxA(NULL, error.c_str(), NULL, NULL);
}
void DumperThread()
{
    Dumping = true;

    if (Logging)
    {
        std::remove("ProcessDumper.log");
        if (!freopen("ProcessDumper.log", "w", stdout))
        {
            LogError("CAN NOT CREATE LOG FILE\n");
        }
    }

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, ListOfProcesses[selected].PID);
    if (processHandle == INVALID_HANDLE_VALUE)
    {
        Dumping = false;
        LogError("CAN NOT CREATE HANDLE\n");
    }
    else
    {
        BOOL bIsWow64 = FALSE;
        IsWow64Process(processHandle, &bIsWow64);
        if (bIsWow64 == FALSE)
        {
            Dumping = false;
            LogError("It's a 64bit process!\n");
        }
        else
        {
            MEMORY_BASIC_INFORMATION mbi;
            unsigned char* memoryAddress = nullptr;
            auto pagesScanned = 0;
            while (true)
            {
                if (!VirtualQueryEx(processHandle, memoryAddress, &mbi, 0x1000))
                {
                    Dumping = false;
                    break;
                }

                if (IsDumpableMemoryInformation(mbi))
                {
                    LPVOID pageBuffer = VirtualAlloc(nullptr, mbi.RegionSize, MEM_COMMIT, PAGE_READWRITE);
                    if (!pageBuffer)
                    {
                        Dumping = false;
                        AddLog(std::string("[" + GetTime() + "] CAN NOT VirtualAlloc\n").c_str());
                        return;
                    }

                    SIZE_T bytesRead = 0;
                    if (ReadProcessMemory(processHandle, mbi.BaseAddress, pageBuffer, mbi.RegionSize, &bytesRead))
                    {
						try
						{
							std::string Path = FolderPath;
							if (Path.size() > 2)
							{
								if (Path.back() != '\\' && Path.back() != '/') Path = Path + "\\";
							}
							else Path = "";
							Path = (Path + GetMemoryInformationFileName(mbi));

							std::ofstream fileStream(Path, std::ios::out | std::ios::binary);
							if (fileStream.bad())
							{
								Dumping = false;
								LogError("Write result to file error!\n");
							}
							fileStream.write((char*)pageBuffer, mbi.RegionSize);
							fileStream.close();
				 
							AddLog(std::string("[" + GetTime() + "] WRITING SECTION TO FILE: " + Path +"\n").c_str());
						}
                        catch (...) {}
                    }
                    else
                    {
                        Dumping = false;
                        LogError("CAN NOT ACCESS MEMORY\n");
                    }

                    VirtualFree(pageBuffer, mbi.RegionSize, MEM_FREE);
                }

                pagesScanned++;
                memoryAddress += mbi.RegionSize;
            }
            AddLog(std::string("[" + GetTime() + "] SUCCESS! FINISHED: " + std::to_string(pagesScanned) + " PAGES SCANNED!" + "\n").c_str());
            Dumping = false;
        }
    }
    if (Logging) fclose(stdout);
}

int main(int, char**)
{
    std::string randomclassname = "ProcessDumper";//random_string(random_number(4, 16));
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, randomclassname.c_str(), NULL, NULL);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* DefaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImFont* BigFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 24.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImFont* LogsFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
   
    //ImGui::StyleColorsDark();
    //ImGuiStyle& style = ImGui::GetStyle();
    //{
    //    style.Alpha = 1.f;
    //    style.WindowPadding = ImVec2(5, 5);
    //    style.WindowRounding = 0.0f;
    //    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    //    style.ChildRounding = 3.0f;
    //    style.FrameBorderSize = 1;
    //    style.FramePadding = ImVec2(4, 3);
    //    style.FrameRounding = 5;
    //    style.ItemSpacing = ImVec2(8, 0);
    //    style.ItemInnerSpacing = ImVec2(4, 4);
    //    style.TouchExtraPadding = ImVec2(0, 0);
    //    style.IndentSpacing = 21.0f;
    //    style.ColumnsMinSpacing = 6.0f;
    //    style.ScrollbarSize = 10.0f;
    //    style.ScrollbarRounding = 3.0f;
    //    style.GrabMinSize = 20.0f;
    //    style.GrabRounding = 3.0f;
    //    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    //    style.DisplayWindowPadding = ImVec2(22, 22);
    //    style.DisplaySafeAreaPadding = ImVec2(4, 4);
    //    style.AntiAliasedLines = true;
    //}
    GetMyStyle();

    int BackgroundTextureWidth = 0;
    int BackgroundTextureHeight = 0;
    GLuint LogoTexture = 0;
    bool ret = LoadTextureFromMemory(logo, sizeof(logo), &LogoTexture, &BackgroundTextureWidth, &BackgroundTextureHeight);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    int counter = 58;
    bool open = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    DWORD dwFlag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    DWORD dwFlagHelpWindow = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    std::string CurrentPath = GetCurrentPath();
    for (int i = 0; i < CurrentPath.size(); i++)
    {
        FolderPath[i] = CurrentPath[i];
        FolderPath[i + 1] = '\0';
    }

    while (!glfwWindowShouldClose(window) && open)
    {
        counter = counter + 1;
        if (counter >= 59)
        {
            counter = 0;

            ListOfProcesses.clear();
            WTS_PROCESS_INFO* pWPIs = NULL;
            DWORD dwProcCount = 0;
            if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pWPIs, &dwProcCount))
            {
                for (DWORD i = 0; i < dwProcCount; i++) //Go through all processes retrieved
                {
                    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pWPIs[i].ProcessId);
                    if (processHandle && processHandle != INVALID_HANDLE_VALUE)
                    {
                        BOOL bIsWow64 = FALSE;
                        IsWow64Process(processHandle, &bIsWow64);
                        if (bIsWow64 == TRUE) ListOfProcesses.push_back({ pWPIs[i].ProcessId, pWPIs[i].pProcessName });
                        CloseHandle(processHandle);
                    }
                }
            }
            if (pWPIs) //Free memory
            {
                WTSFreeMemory(pWPIs);
                pWPIs = NULL;
            }
        }


        glfwPollEvents();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(WindowWidth, WindowHeight), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        //ImGui::ShowDemoWindow();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::MenuItem("AutoScroll", NULL, AutoScroll)) { AutoScroll = !AutoScroll; }
                if (ImGui::MenuItem("File logging", NULL, Logging)) { Logging = !Logging; }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                ShowHelp = !ShowHelp;
                ImGui::CloseCurrentPopup();
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }


        ImGui::SetNextWindowPos(ImVec2(0, 27));
        ImGui::SetNextWindowSize(ImVec2(409, 407));
        ImGui::Begin("INT PROCESS DUMPER v1.0", &open, dwFlag);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushItemWidth(300.f);
            ImGui::Text("Processes list:");
            ImGui::InputText("Search", SearchFor, sizeof(SearchFor));

            ImGui::BeginChild("ProcessBox", ImVec2(400, 200), true);
                ImGui::Columns(2, "mycolumns"); 
                if (SetWidthOnce)
                {
                    ImGui::SetColumnWidth(0, 75.f);
                    SetWidthOnce = false;
                }
                ImGui::Separator();
                ImGui::Text("PID"); ImGui::NextColumn();
                ImGui::Text("Name"); ImGui::NextColumn();
                ImGui::Separator();
                if (ListOfProcesses.size() > 0)
                {
                    for (int i = 0; i < ListOfProcesses.size(); i++)
                    {
                        std::string procname = ListOfProcesses[i].ProcName;
                        std::string searchthing = SearchFor;
                        std::transform(procname.begin(), procname.end(), procname.begin(), ::tolower);
                        std::transform(searchthing.begin(), searchthing.end(), searchthing.begin(), ::tolower);
                        if (procname.find(searchthing) != std::string::npos)
                        {
                            if (ImGui::Selectable(std::to_string(ListOfProcesses[i].PID).c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns)) selected = i;
                            ImGui::NextColumn();
                            ImGui::Text(ListOfProcesses[i].ProcName.c_str());
                            ImGui::NextColumn();
                        }
                    }
                }
                ImGui::Columns();
            ImGui::EndChild();
            ImGui::InputText("Extract folder", FolderPath, sizeof(FolderPath));

            if (Dumping)
            {
                ImGui::Button("Dumping... Please wait.", ImVec2(400, 26));
            }
            else
            {
                if (ImGui::Button("DUMP!", ImVec2(400, 26)))
                {
                    if(selected) CreateThread(0, 0, (LPTHREAD_START_ROUTINE)DumperThread, 0, 0, 0);
                }
            }




            ImGui::BeginChild("LogBox", ImVec2(400, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::PushFont(LogsFont);

            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();

           // ImGui::TextUnformatted(buf, buf_end);
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);

            ImGui::PopFont();
            ImGui::EndChild();
            ImGui::PopItemWidth();
            ImGui::PopStyleVar();
        }
        ImGui::End();

        if (ShowHelp)
        {
            ImGui::SetNextWindowPos(ImVec2(WindowWidth/2- HelpWindowWidth /2, WindowHeight / 2 - HelpWindowHeight / 2));
            ImGui::SetNextWindowSize(ImVec2(HelpWindowWidth, HelpWindowHeight));
            ImGui::Begin("Help", &ShowHelp, dwFlagHelpWindow);
            {
                ImGui::Dummy(ImVec2(26, 0)); ImGui::SameLine(); ImGui::Image((void*)(intptr_t)LogoTexture, ImVec2(LogoWidth, LogoHeight));
            
                ImGui::PushFont(BigFont);
                ImGui::Dummy(ImVec2(18, 0)); ImGui::SameLine(); ImGui::Text("INT PROCESS DUMPER v1.0");
                ImGui::PopFont();
                std::string Date = __DATE__;
                ImGui::Dummy(ImVec2(98, 0)); ImGui::SameLine(); ImGui::Text(Date.c_str());

                ImGui::Dummy(ImVec2(0, 14));
                ImGui::Text("HOW TO USE:");
                ImGui::BulletText("Select any 32bit process.");
                ImGui::BulletText("Select folder for output\n(or leave empty for current folder).");
                ImGui::BulletText("Browse files and look for magic header.");

                ImGui::Dummy(ImVec2(0, 14));
                ImGui::Dummy(ImVec2(100, 0)); ImGui::SameLine(); 
                ImGui::PushFont(BigFont);
                ImGui::Text("LINKS:");
                ImGui::PopFont();

                if(ImGui::Button("website", ImVec2(91, 26))) ShellExecute(NULL, "open", "https://interium.ooo/", NULL, NULL, NULL);
                ImGui::SameLine();
                if(ImGui::Button("github", ImVec2(91, 26))) ShellExecute(NULL, "open", "https://github.com/KleskBY/", NULL, NULL, NULL);
                ImGui::SameLine();
                if (ImGui::Button("telegram", ImVec2(91, 26)))  ShellExecute(NULL, "open", "http://t.me/kleskby", NULL, NULL, NULL);

            }
            ImGui::End();
        }

        ImGui::EndFrame();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
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
