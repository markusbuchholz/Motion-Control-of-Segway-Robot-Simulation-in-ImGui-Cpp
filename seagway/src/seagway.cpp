// Markus Buchholz, 2023

#include <stdafx.hpp>
#include "imgui_helper.hpp"
#include <tuple>
#include <thread>
#include <chrono>
#include <vector>
#include <math.h>
#include <list>
#include <numeric>
#include <memory>

#include <Eigen/Dense>

//----------- system dynamic parameters --------------------

float L = 1.0;
float r = 0.5;
float l = 2.5;
float m = 500;
float M = 80;
float a = 1.0;
float g = 9.81;

float dt = 0.01;

// proportional coefficients
float kp1 = 12000.0;
float kp2 = 300.0;

// differential coefficients
float kd1 = 2500.0;
float kd2 = 10 * 200.0;

// from Jupyter variables
float A = m * (a * a + (l * l) / 12);
float B = m * a * r;
float C = (m + 3 / 2 * M) * r * r;

float D = A * C;
float E = B * B;
float F = m * g * a;

//-----------------------------------------------------------
// Torque generated by motor installed in wheel
float controlTrq(float theta, float fi, float theta_dot, float fi_dot)
{
	float control = kp1 * theta + kd1 * theta_dot + kp2 * theta_dot + kd2 * fi_dot;
	return control;
}

//-----------------------------------------------------------
// theta_dot - pendulum angular velocity
float function1(float theta, float fi, float theta_dot, float fi_dot)
{

	return fi;
}

//-----------------------------------------------------------
// theta_dot_dot - pendulum angular acceleration
float function2(float theta, float fi, float theta_dot, float fi_dot)
{

	float theta_dot_dot = (C * (F * std::sin(theta) - controlTrq(theta, fi, theta_dot, fi_dot)) - B * std::cos(theta) * (controlTrq(theta, fi, theta_dot, fi_dot) + B * (fi * fi) * std::sin(theta))) / (D - E * (std::cos(theta)) * (std::cos(theta)));

	return theta_dot_dot;
}

//-----------------------------------------------------------
// phi_dot - wheel angular velocity
float function3(float theta, float fi, float theta_dot, float fi_dot)
{

	return fi_dot;
}

//-----------------------------------------------------------
// phi_dot_dot - wheel angular acceleration
float function4(float theta, float fi, float theta_dot, float fi_dot)
{

	float phi_dot_dot = (A * (F * std::sin(theta) - controlTrq(theta, fi, theta_dot, fi_dot)) - B * std::cos(theta) * (controlTrq(theta, fi, theta_dot, fi_dot) + B * (fi * fi) * std::sin(theta))) / (D - E * (std::cos(theta)) * (std::cos(theta)));

	return phi_dot_dot;
}

//-----------------------------------------------------------

int main(int argc, char const *argv[])
{
	ImVec4 clear_color = ImVec4(0.0f / 255.0, 0.0f / 255.0, 0.0f / 255.0, 1.00f);
	ImVec4 white_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 pink_color = ImVec4(245.0f / 255.0, 5.0f / 255.0, 150.0f / 255.0, 1.00f);
	ImVec4 blue_color = ImVec4(0.0f / 255.0, 0.0f / 255.0, 2550.0f / 255.0, 1.00f);
	int flag = true;

	int w = 800;
	int h = 500;
	std::string title = "Seagway robot";
	initImgui(w, h, title);

	float k11{0.0f}, k12{0.0f}, k13{0.0f}, k14{0.0f};
	float k21{0.0f}, k22{0.0f}, k23{0.0f}, k24{0.0f};
	float k31{0.0f}, k32{0.0f}, k33{0.0f}, k34{0.0f};
	float k41{0.0f}, k42{0.0f}, k43{0.0f}, k44{0.0f};

	// init values
	float x1 = (M_PI / 180) * 70; // theta - pendulum
	float x2 = 0.0f;			  // theta_dot - pednulum
	float x3 = 0.0f;			  // phi  - wheel
	float x4 = 0.0f;			  // phi_dot -wheel
	float t = 0.0;				  // init time
	int ij = 0;

	while (!glfwWindowShouldClose(window) && flag == true)
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGuiWindowFlags window_flags = 0;
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Simulation", nullptr, window_flags);
		ImDrawList *draw_list = ImGui::GetWindowDrawList();

		k11 = function1(x1, x2, x3, x4);
		k12 = function2(x1, x2, x3, x4);
		k13 = function3(x1, x2, x3, x4);
		k14 = function4(x1, x2, x3, x4);

		k21 = function1(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);
		k22 = function2(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);
		k23 = function3(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);
		k24 = function4(x1 + dt / 2 * k11, x2 + dt / 2 * k12, x3 + dt / 2 * k13, x4 + dt / 2 * k14);

		k31 = function1(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);
		k32 = function2(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);
		k33 = function3(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);
		k34 = function4(x1 + dt / 2 * k21, x2 + dt / 2 * k22, x3 + dt / 2 * k23, x4 + dt / 2 * k24);

		k41 = function1(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);
		k42 = function2(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);
		k43 = function3(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);
		k44 = function4(x1 + dt * k31, x2 + dt * k32, x3 + dt * k33, x4 + dt * k34);

		x1 = x1 + dt / 6.0 * (k11 + 2 * k21 + 2 * k31 + k41);
		x2 = x2 + dt / 6.0 * (k12 + 2 * k22 + 2 * k32 + k42);
		x3 = x3 + dt / 6.0 * (k13 + 2 * k23 + 2 * k33 + k43);
		x4 = x4 + dt / 6.0 * (k14 + 2 * k24 + 2 * k34 + k44);

		draw_list->AddCircleFilled({(x3 * 50.0f) + 400.0f, 400.0f}, 50.0f, ImColor(pink_color));
		draw_list->AddLine({(x3 * 50.0f) + 400.0f, 400.0f}, {(x3 * 50.0f) + 400.0f + std::sin(x1) * 200.0f, 400.0f - std::cos(x1) * 200.0f}, ImColor(white_color), 10.0f);

		float px = (x3 * 50.0f) + 400.0f + std::sin(x3) * 50.0f;
		float py = 400.0f - std::cos(x3) * 50.0f;

		draw_list->AddCircleFilled({px, py}, 5.0f, ImColor(blue_color));
		draw_list->AddLine({(x3 * 50.0f) + 400.0f, 400.0f}, {px, py}, ImColor(blue_color), 3.0f);

		ImGui::End();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	termImgui();
	return 0;
}