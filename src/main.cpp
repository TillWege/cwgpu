#include <iostream>
#include <GLFW/glfw3.h>
#include "wgpu_utils.h"
#include <glfw3webgpu.h>
#include <vector>

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

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(640, 480, "CWGPU", nullptr, nullptr);
	if (!window) {
		std::cerr << "Could not open window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	std::cout << "Requesting surface..." << std::endl;
	WGPUSurface surface = glfwGetWGPUSurface(instance, window);
	std::cout << "Got surface" << surface << std::endl;

	std::cout << "Requesting adapter..." << std::endl;
	WGPURequestAdapterOptions adapterOpts = {};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface;
	WGPUAdapter adapter = requestAdapter(instance, &adapterOpts);

	std::cout << "Got adapter: " << adapter << std::endl;

	// Check features of adapter

	std::vector<WGPUFeatureName> features;

	// Call the function a first time with a null return address, just to get the entry count.
	size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

	// Allocate memory (could be a new, or a malloc() if this were a C program)
	features.resize(featureCount);

	// Call the function a second time, with a non-null return address
	wgpuAdapterEnumerateFeatures(adapter, features.data());

	std::cout << "Adapter features:" << std::endl;
	for (auto f : features) {
		std::cout << " - " << f << std::endl;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	wgpuAdapterRelease(adapter);
	wgpuInstanceRelease(instance);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
