#include <iostream>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

int main (int, char**) {
	WGPUInstanceDescriptor desc = {};
	desc.nextInChain = nullptr;

	WGPUInstance instance = wgpuCreateInstance(&desc);

	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
		return 1;
	}

	std::cout << "WGPU instance: " << instance << std::endl;

	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW!" << std::endl;
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "CWGPU", NULL, NULL);
	if (!window) {
		std::cerr << "Could not open window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	wgpuInstanceRelease(instance);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
