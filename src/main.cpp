#include <iostream>
#include <GLFW/glfw3.h>
#include "wgpu_utils.h"
#include <glfw3webgpu.h>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"

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
	GLFWwindow* window = glfwCreateWindow(1280, 720, "CWGPU", nullptr, nullptr);
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

	// Encode some mock commands
	wgpuCommandEncoderInsertDebugMarker(encoder, "Do one thing");
	wgpuCommandEncoderInsertDebugMarker(encoder, "Do another thing");

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
	swapChainDesc.width = 1280;
	swapChainDesc.height = 720;

	WGPUTextureFormat swapChainFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
	swapChainDesc.format = swapChainFormat;
	swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
	swapChainDesc.presentMode = WGPUPresentMode_Fifo;

	// Create SwapChain
	WGPUSwapChain swapChain = wgpuDeviceCreateSwapChain(device, surface, &swapChainDesc);
	std::cout << "Swapchain: " << swapChain << std::endl;

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplWGPU_InitInfo info = {};

    info.Device = device;
    info.RenderTargetFormat = swapChainFormat;
    info.DepthStencilFormat = WGPUTextureFormat_Undefined;

    ImGui_ImplWGPU_Init(&info);

    // Setup shaders for Pipeline
    const char* shaderSource = R"(
    @vertex
    fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
        var p = vec2f(0.0, 0.0);
        if (in_vertex_index == 0u) {
            p = vec2f(-0.5, -0.5);
        } else if (in_vertex_index == 1u) {
            p = vec2f(0.5, -0.5);
        } else if (in_vertex_index == 2u) {
            p = vec2f(-0.5, 0.5);
        } else if (in_vertex_index == 3u) {
            p = vec2f(0.5, 0.5);
        } else if (in_vertex_index == 4u) {
            p = vec2f(0.5, -0.5);
        } else if (in_vertex_index == 5u) {
            p = vec2f(-0.5, 0.5);
        }
        return vec4f(p, 0.0, 1.0);
    }

    @fragment
    fn fs_main() -> @location(0) vec4f {
        return vec4f(0.0, 0.4, 1.0, 1.0);
    }
    )";

    WGPUShaderModuleDescriptor shaderModuleDescriptor = {};

#ifdef WEBGPU_BACKEND_WGPU
    shaderModuleDescriptor.hintCount = 0;
    shaderModuleDescriptor.hints = nullptr;
#endif

    // Setup WGSL Chain
    WGPUShaderModuleWGSLDescriptor shaderModuleWgslDescriptor = {};
    shaderModuleWgslDescriptor.chain.next = nullptr;
    shaderModuleWgslDescriptor.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderModuleWgslDescriptor.code = shaderSource;

    shaderModuleDescriptor.nextInChain = &shaderModuleWgslDescriptor.chain;

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDescriptor);

    // Setup Pipeline Descriptor
    WGPURenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.nextInChain = nullptr;

    // vertex fetch shader settings
    pipelineDescriptor.vertex.bufferCount = 0;
    pipelineDescriptor.vertex.buffers = nullptr;

    // vertex shader settings
    pipelineDescriptor.vertex.module = shaderModule;
    pipelineDescriptor.vertex.entryPoint = "vs_main";
    pipelineDescriptor.vertex.constantCount = 0;
    pipelineDescriptor.vertex.constants = nullptr;

    // primitive shader settings
    pipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDescriptor.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDescriptor.primitive.cullMode = WGPUCullMode_None;

    // fragment shader settings
    WGPUFragmentState fragmentState = {};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constants = nullptr;
    fragmentState.constantCount = 0;
    fragmentState.targetCount = 1;

    // setup blend-state
    WGPUBlendState blendState = {};
    // RGB Settings
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;

    // Alpha-Settings
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;

    // setup color target
    WGPUColorTargetState  colorTargetState = {};
    colorTargetState.format = swapChainFormat;
    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = WGPUColorWriteMask_All;

    fragmentState.targets = &colorTargetState;

    pipelineDescriptor.fragment = &fragmentState;
    pipelineDescriptor.depthStencil = nullptr;
    pipelineDescriptor.layout = nullptr;

    // MSAA settings
    pipelineDescriptor.multisample.count = 1;
    pipelineDescriptor.multisample.mask = ~0u;
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;


    // Get Pipeline from Pipeline descriptor
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDescriptor);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		if(r < 1.0f) {
			r += 0.01f;
		} else if(g < 1.0f) {
			g += 0.01f;
		} else if(b < 1.0f) {
			b += 0.01f;
		} else {
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
		}

		WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
		std::cout << "nextTexture: " << nextTexture << std::endl;

		if (!nextTexture) {
			std::cerr << "Cannot acquire next swap chain texture" << std::endl;
			break;
		}

		// Create a command encoder
		encoderDesc = {};
		encoderDesc.nextInChain = nullptr;
		encoderDesc.label = "Rendering Encoder";
		encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

		// Setup Render Pass Descriptor
		WGPURenderPassDescriptor renderPassDesc = {};

		// Fill Render Pass Descriptor
		{
			WGPURenderPassColorAttachment renderPassColorAttachment = {};
			renderPassColorAttachment.view = nextTexture;
			renderPassColorAttachment.resolveTarget = nullptr;
			renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
			renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
			renderPassColorAttachment.clearValue = WGPUColor{ r, g, b, 1.0 };

			renderPassDesc.colorAttachmentCount = 1;
			renderPassDesc.colorAttachments = &renderPassColorAttachment;
			renderPassDesc.depthStencilAttachment = nullptr;
			renderPassDesc.timestampWriteCount = 0;
			renderPassDesc.timestampWrites = nullptr;
			renderPassDesc.nextInChain = nullptr;
		}

		// Submit Render Pass
		WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplWGPU_NewFrame();
//        ImGui::NewFrame();
//        ImGui::SetWindowSize(ImVec2(500, 300));
//        ImGui::ShowDemoWindow(); // Show demo window! :)
//
//        ImGui::Render();
//        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);

        wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
        wgpuRenderPassEncoderDraw(renderPass, 6, 1, 0, 0);
		wgpuRenderPassEncoderEnd(renderPass);
		wgpuRenderPassEncoderRelease(renderPass);

		// Create a Command Buffer
		cmdBufferDescriptor = {};
		cmdBufferDescriptor.nextInChain = nullptr;
		cmdBufferDescriptor.label = "Clear Buffer";
		command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
		wgpuCommandEncoderRelease(encoder); // release encoder after it's finished
		wgpuQueueSubmit(queue, 1, &command);
		wgpuCommandBufferRelease(command);


		wgpuTextureViewRelease(nextTexture);
		wgpuSwapChainPresent(swapChain);
	}

    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();


	wgpuSwapChainRelease(swapChain);
	wgpuQueueRelease(queue);
	wgpuDeviceRelease(device);
	wgpuAdapterRelease(adapter);
	wgpuInstanceRelease(instance);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
