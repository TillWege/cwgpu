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

	std::cout << "Requesting device..." << std::endl;

	WGPUDeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "Rendering Device";
	deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
	deviceDesc.requiredLimits = nullptr; // we do not require any specific limit
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	WGPUDevice device = requestDevice(adapter, &deviceDesc);

	auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);

	std::cout << "Got device: " << device << std::endl;


	// Create a queue
	WGPUQueue queue = wgpuDeviceGetQueue(device);

	// Setup Queue Callback
	auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
		std::cout << "Queued work finished with status: " << status << std::endl;
	};
	wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr /* pUserData */);

	// Create a command encoder
	WGPUCommandEncoderDescriptor encoderDesc = {};
	encoderDesc.nextInChain = nullptr;
	encoderDesc.label = "My command encoder";
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

	// Create a Command Buffer
	WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.nextInChain = nullptr;
	cmdBufferDescriptor.label = "Command buffer";
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
	wgpuCommandEncoderRelease(encoder); // release encoder after it's finished

	// Finally submit the command queue
	std::cout << "Submitting command..." << std::endl;
	wgpuQueueSubmit(queue, 1, &command);
	wgpuCommandBufferRelease(command);

	// Setup SwapChain Descriptor
	WGPUSwapChainDescriptor swapChainDesc = {};
	swapChainDesc.nextInChain = nullptr;
	swapChainDesc.width = 640;
	swapChainDesc.height = 480;

	WGPUTextureFormat swapChainFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
	swapChainDesc.format = swapChainFormat;
	swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
	swapChainDesc.presentMode = WGPUPresentMode_Fifo;

	// Create SwapChain
	WGPUSwapChain swapChain = wgpuDeviceCreateSwapChain(device, surface, &swapChainDesc);
	std::cout << "Swapchain: " << swapChain << std::endl;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	wgpuSwapChainRelease(swapChain);
	wgpuQueueRelease(queue);
	wgpuDeviceRelease(device);
	wgpuAdapterRelease(adapter);
	wgpuInstanceRelease(instance);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
