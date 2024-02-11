#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <limits>

#include <execution>
#include <algorithm>

// ---------------------------------------

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION
#include "glfw/glfw3.h"

// ---------------------------------------

#include "../Common/util.hpp"
#include "../Common/reader.hpp"

#include "btree_sort.hpp"

using std::string;
using std::vector;

using std::unique_ptr;

// ------------------------------------------------------------------------------

using TypeData = int32_t;

template<typename T> class Bounds {
public:
	static constexpr T min = std::numeric_limits<T>::min();
	static constexpr T max = std::numeric_limits<T>::max();
};
template<> class Bounds<double> {
public:
	static constexpr double min = -10000.0;
	static constexpr double max = 10000.0;
};

vector<TypeData> g_dataOriginal;
vector<TypeData> g_dataSorted;

void RenderData(vector<TypeData>& data);

// ------------------------------------------------------------------------------

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main(int argc, char** argv) {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;
	
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	GLFWwindow* window = glfwCreateWindow(1600, 900, "msort visualize", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	
	glfwSetKeyCallback(window, glfw_key_callback);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;
	
	ImGui::StyleColorsDark();
	
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	
	{
		FileReader input(argv[1], true);
		
		g_dataOriginal = input.ReadData<TypeData>();
		g_dataSorted = g_dataOriginal;	// Copy
		
		btreesort::BTreeSort btreesort(
			g_dataSorted.begin(), g_dataSorted.end(), 
			std::less<TypeData>());
		btreesort.Sort();
	}
	
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		{
			ImGui::SetNextWindowSize(ImVec2(1200, 400), ImGuiCond_Appearing);
			ImGui::Begin("Data original");
			
			RenderData(g_dataOriginal);
			
			ImGui::End();
		}
		{
			ImGui::SetNextWindowSize(ImVec2(1200, 400), ImGuiCond_Appearing);
			ImGui::Begin("Data sorted");
			
			RenderData(g_dataSorted);
			
			ImGui::End();
		}
		
		{
			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			
			glfwSwapBuffers(window);
		}
	}
	
	// ---------------------------------------
	
	/* FileReader input("data_1k_i32_r.bin", true);
	
	DataType typeDataParse = DataType::i32;
	SortType typeSort = SortType::BTreeMerge;
	
	try {
		vector<int32_t> data = input.ReadData<int32_t>();
		
		BTreeSort btreesort(data.begin(), data.end(), std::less<int32_t>());
		btreesort.Sort();
	}
	catch (const string& e) {
		printf("Fatal error-> %s", e.c_str());
	} */
	
	//printf("Done");
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}

void RenderData(vector<TypeData>& data) {
	const float cvWidth = 1100;
	const float cvMaxHeight = 250;
	
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	
	const auto winPos = ImGui::GetWindowPos();
	const auto winSize = ImGui::GetContentRegionAvail();
	
	float x = winPos.x + winSize.x / 2 - cvWidth / 2;
	float y = winPos.y + winSize.y / 2 + cvMaxHeight / 2 + 32;
	float itemW = cvWidth / data.size();
	
	const ImU32 col = ImColor(ImVec4(0.9f, 0.9f, 1.0f, 1.0f));
	
	for (auto& i : data) {
		double min = (double)Bounds<TypeData>::min;
		double max = (double)Bounds<TypeData>::max;
		double rate = ((double)i - min) / (max - min);
		float height = cvMaxHeight * rate;
		
		drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + itemW, y - height), col);
		
		x += itemW;
	}
}
