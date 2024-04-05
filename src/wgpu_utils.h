//
// Created by tillw on 05/04/2024.
//

#ifndef WGPU_UTILS_H
#define WGPU_UTILS_H

#include <webgpu/webgpu.h>

WGPUAdapter requestAdapter(WGPUInstance instance, WGPURequestAdapterOptions const * options);

WGPUDevice requestDevice(WGPUAdapter adapter, WGPUDeviceDescriptor const * descriptor);

#endif //WGPU_UTILS_H
