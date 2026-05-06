// src/main.cpp

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

int WIDTH  = 700;
int HEIGHT = 400;
int N = 10000;   // number of particles
float dt = 0.0f;




// Helper: read file into string
std::string readFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}



// Psuedo-random number
static uint32_t xorshift_state = 123456789;

inline uint32_t xorshift32() {
	uint32_t x = xorshift_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return xorshift_state = x;
}



int main(int argc, char** argv) {
	// ---------------------------
	// 1. SDL2 setup
	// ---------------------------
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	SDL_Window* window = SDL_CreateWindow("OpenCL Particles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Do NOT add SDL_RENDERER_PRESENTVSYNC because it locks the simulation FPS!

	// ---------------------------
	// 2. Initialize particles
	// ---------------------------
	try {if (argc == 2) N = std::stoi(argv[1]);}
	catch (const std::exception& e) {
		std::cout << "Invalid integer in argument 0: " << e.what() << std::endl;
	}

	std::vector<cl_float4> particles(N);
	std::vector<int> species(N);

	int x, y;
	SDL_GetWindowSize(window, &x, &y);
	const int winW = x;
	const int winH = y;

	for (int i = 0; i < N; i++) {
	    particles[i] = {
		float(rand()) / RAND_MAX * float(winW),   // random float [0, WIDTH)
		float(rand()) / RAND_MAX * float(winH),  // random float [0, HEIGHT)
		0.0f,
		0.0f
	    };
	    species[i] = rand() % 5;
	}

	// ---------------------------
	// 3. OpenCL setup
	// ---------------------------
	cl_int err;
	cl_platform_id platform;
	cl_device_id device;
	err = clGetPlatformIDs(1, &platform, nullptr);
	if (err != CL_SUCCESS) { std::cerr << "No OpenCL platform\n"; return 1; }
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &device, nullptr);
	if (err != CL_SUCCESS) err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
	if (err != CL_SUCCESS) err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, nullptr);
	if (err != CL_SUCCESS) { std::cerr << "No OpenCL device\n"; return 1; }

	cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

	cl_mem bufParticles = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float4)*N, particles.data(), &err);
	cl_mem bufSpecies = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(int)*N, species.data(), &err);
	
	std::string kernelSrc = readFile("./gpu-code/particles.cl");

	const char* src = kernelSrc.c_str();
	cl_program program = clCreateProgramWithSource(context, 1, &src, nullptr, &err);
	if (clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr) != CL_SUCCESS) {
		size_t logSize;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
		std::vector<char> log(logSize);
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
		std::cerr << "Build error:\n" << log.data() << "\n";
		return 1;
	}
	cl_kernel kernel = clCreateKernel(program, "update_particles", &err);

	// ---------------------------
	// 4. Main loop
	// ---------------------------
	bool running = true;
	Uint64 now = SDL_GetPerformanceCounter();

	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = false;
		}

		now = SDL_GetPerformanceCounter();
		static Uint64 last = 0;

		if (last != 0) dt = (float)(now - last) / SDL_GetPerformanceFrequency() / 6;


		// Run kernel
		clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufParticles);
		clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufSpecies);
		clSetKernelArg(kernel, 2, sizeof(int), &winW);
		clSetKernelArg(kernel, 3, sizeof(int), &winH);
		clSetKernelArg(kernel, 4, sizeof(float), &dt);
		clSetKernelArg(kernel, 5, sizeof(int), &N);

		size_t globalSize = (N + 1) / 2;
		clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
		clFinish(queue);

		// Read results back
		clEnqueueReadBuffer(queue, bufParticles, CL_TRUE, 0,
			sizeof(cl_float4)*N, particles.data(), 0, nullptr, nullptr);

		// Render
		SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
		SDL_RenderClear(renderer);

		// Have the loop start at a random item and iterate through the loop looping around to 0
		int rand = 0;//xorshift32() % (N + 1);
		for (int l = rand; l < N + rand; l++) {
			int i = (l + N) % N;

			SDL_Color color;
			switch ((int)species[i]) {
				case 0: color =  {255,   0,   0, 255}; break; // red
				case 1: color =  {255, 255,   0, 255}; break; // yellow
				case 2: color =  {135, 206, 235, 255}; break; // sky blue
				case 3: color =  {  0, 128,   0, 255}; break; // green
				case 4: color =  {23, 100, 255, 255};  break; // abiotic particles
				default: color = {255, 255, 255, 255}; break;
			}

			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_RenderDrawPoint(renderer, (int)particles[i].s[0], (int)particles[i].s[1]);
		}

		SDL_RenderPresent(renderer);
		last = now;
	}

	// ---------------------------
	// Cleanup
	// ---------------------------
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	clReleaseMemObject(bufParticles);
	clReleaseMemObject(bufSpecies);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	return 0;
}
