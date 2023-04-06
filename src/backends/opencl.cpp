/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     06.04.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/backends/opencl.h"
#include "../../include/inn/neuron.h"
#include "../../include/inn/system.h"

#define KERNEL(name, ...) std::string name = #__VA_ARGS__

inn::ComputeBackendOpenCL::ComputeBackendOpenCL() {
#ifdef INN_OPENCL_SUPPORT
    std::vector<cl::Platform> all_platforms;
    std::vector<cl::Device> all_devices;

    cl::Platform::get(&all_platforms);

    if (all_platforms.empty()) {
        std::cerr << "No platforms found. Check OpenCL installation!" << std::endl;
        return;
    }

    cl::Platform default_platform = all_platforms[0];

    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.empty()) {
        std::cerr << "No devices found. Check OpenCL installation!" << std::endl;;
        return;
    }

    cl::Device default_device = all_devices[0];

    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    std::cout << "Using device:   " << default_device.getInfo<CL_DEVICE_NAME>() << " (CU: " << default_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << ")" << std::endl;
    std::cout << "Driver version: " << default_device.getInfo<CL_DRIVER_VERSION>() << std::endl;

    KERNEL(kernel_code,
           __kernel void inn_kernel(__global float4 *output,  __global float16 *input) {
                   int id = get_global_id(0);
                   // input.s0 - receptor x
                   // input.s1 - receptor y
                   // input.s2 - synapse x
                   // input.s3 - synapse y

                   // input.s4 - synapse gamma value
                   // input.s5 - neuron input value

                   // input.s6 - lambda
                   // input.s7 - k1
                   // input.s8 - k2

                   // vector length
                   float d = 0;
                   d += (input[id].s0-input[id].s2)*(input[id].s0-input[id].s2);
                   d += (input[id].s1-input[id].s3)*(input[id].s1-input[id].s3);
                   d = sqrt(d);

                   float ngamma = input[id].s4 + (input[id].s7*input[id].s5-input[id].s4/input[id].s8);

                   float e = input[id].s6 * exp(-input[id].s6*d);
                   float fi = input[id].s4 * e;
                   float dfi = input[id].s5 * e;
                   float nx = 0, ny = 0;
                   if (dfi > 0) {
                       float nposd = sqrt(dfi) / d;
                       nx = fabs(input[id].s0-input[id].s2) * nposd;
                       ny = fabs(input[id].s1-input[id].s3) * nposd;
                   }

                   output[get_global_id(0)] = (float4)(nx, ny, fi, 0);
           }
    );

    Context = cl::Context(CL_DEVICE_TYPE_ALL);

    cl::Program program(Context, {kernel_code.c_str(),kernel_code.length()+1});
    int status;
    if ((status = program.build({default_device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        return;
    }

    Kernel = cl::Kernel(program,"inn_kernel");
    Queue = cl::CommandQueue(Context,default_device);
    Event = new cl::Event();
#else
    std::cerr << "The OpenCL compute backend is not supported by the current build. Rebuild interfernce library with the INN_OPENCL_SUPPORT flag." << std::endl;
#endif
}

void inn::ComputeBackendOpenCL::doRegisterHost(const std::vector<void*> &objects) {
    PoolSize = objects.size();
#ifdef INN_OPENCL_SUPPORT
    InputData = new cl_float16[PoolSize];
    OutputData = new cl_float4[PoolSize];
    InputBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float16)*PoolSize);
    OutputBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float4)*PoolSize);
    Kernel.setArg(0, OutputBuffer);
    Kernel.setArg(1, InputBuffer);
#endif
}

void inn::ComputeBackendOpenCL::doWaitTarget() {
    Queue.finish();
}

void inn::ComputeBackendOpenCL::doProcess(void *object) {
    // insert data ...

    Queue.enqueueWriteBuffer(InputBuffer,CL_TRUE,0,sizeof(cl_float16)*PoolSize,InputData);
    Queue.enqueueNDRangeKernel(Kernel, cl::NullRange,cl::NDRange(PoolSize),cl::NullRange, {}, Event);
    Queue.enqueueReadBuffer(OutputBuffer,CL_TRUE,0,sizeof(cl_float4)*PoolSize,OutputData);
}
