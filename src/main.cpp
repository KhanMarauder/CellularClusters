// src/main.cpp

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

int WIDTH  = 700;
int HEIGHT = 400;
constexpr int N = 8000;   // number of particles
float dt = 0.0f;

// Helper: read file into string
std::string readFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) throw std::runtime_error("Cannot open file: " + filename);
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

// Pseudo-random number
static uint32_t xorshift_state = 123456789;

inline uint32_t xorshift32() {
	uint32_t x = xorshift_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return xorshift_state = x;
}

void SDL_RenderDrawCircle(SDL_Renderer* renderer, const int32_t centerX, const int32_t centerY, const int32_t radius) {
	int32_t x = radius;
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = tx - (radius << 1);

	while (x >= y) {
		// Draw horizontal lines to fill the circle
		SDL_RenderDrawLine(renderer, centerX - x, centerY - y, centerX + x, centerY - y);
		SDL_RenderDrawLine(renderer, centerX - x, centerY + y, centerX + x, centerY + y);
		SDL_RenderDrawLine(renderer, centerX - y, centerY - x, centerX + y, centerY - x);
		SDL_RenderDrawLine(renderer, centerX - y, centerY + x, centerX + y, centerY + x);

		if (error <= 0) {
			++y;
			error += ty;
			ty += 2;
		}
		if (error > 0) {
			--x;
			tx += 2;
			error += (tx - (radius << 1));
		}
	}
}

int main() {

	// use true randomness
	std::random_device rd;
	srand(rd());
	xorshift_state = rd();

	// ---------------------------
	// 1. SDL2 setup
	// ---------------------------
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
		return 1;
	}

	// Initialize SDL_image for PNG support
	if (int imgFlags = IMG_INIT_PNG; (IMG_Init(imgFlags) & imgFlags) != imgFlags) {
		std::cerr << "SDL_image could not initialize PNG support! IMG_Error: " << IMG_GetError() << "\n";
		SDL_Quit();
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("OpenCL Particles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!window) {
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
		IMG_Quit();
		SDL_Quit();
		return 1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Do NOT add SDL_RENDERER_PRESENTVSYNC because it locks the simulation FPS!
	if (!renderer) {
		std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
		SDL_DestroyWindow(window);
		IMG_Quit();
		SDL_Quit();
		return 1;
	}

	// ---------------------------
	// 2. Initialize particles
	// ---------------------------
	std::vector<cl_float4> particles(N);
	std::vector<int> species(N);

	int x, y;
	SDL_GetWindowSize(window, &x, &y);
	const int winW = x;
	const int winH = y;

	for (int i = 0; i < N; i++) {
	    particles[i] = {
		static_cast<float>(rand()) / RAND_MAX * static_cast<float>(winW),   // random float [0, WIDTH)
		static_cast<float>(rand()) / RAND_MAX * static_cast<float>(winH),  // random float [0, HEIGHT)
		0.0f,
		0.0f
	    };
	    species[i] = rand() % 5;
	}

	// ---------------------------
	// 3. OpenCL setup
	// ---------------------------
	cl_platform_id platform;
	cl_device_id device;
	cl_int err = clGetPlatformIDs(1, &platform, nullptr);
	if (err != CL_SUCCESS) { std::cerr << "No OpenCL platform\n"; return 1; }
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, nullptr);
	if (err != CL_SUCCESS) { std::cerr << "No OpenCL device\n"; return 1; }

	cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

	cl_mem bufParticles = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_float4)*N, particles.data(), &err);
	cl_mem bufSpecies = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(int)*N, species.data(), &err);

	// ---------------------------
	// Render the compiling screen
	// ---------------------------
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	SDL_Surface* surface = IMG_Load("./loading.png"); // Load the loading screen texture
	SDL_Texture* texture = nullptr;
	if (!surface) {
		std::cerr << "Failed to load image './loading.png'! IMG_Error: " << IMG_GetError() << "\n";
	} else {
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		if (!texture) {
			std::cerr << "Failed to create texture from surface! SDL_Error: " << SDL_GetError() << "\n";
		}
	}

	// Render the image if texture is available
	if (texture) {
		constexpr int width = 127*5;
		constexpr int height = 19*5;
		const SDL_Rect dstRect = {(WIDTH/2) - (width/2), (HEIGHT/2) - (height/2), width, height};
		if (SDL_RenderCopy(renderer, texture, nullptr, &dstRect) != 0) {
			std::cerr << "SDL_RenderCopy failed: " << SDL_GetError() << "\n";
		}
	}

	SDL_RenderPresent(renderer); // Present the screen

	SDL_Event event;
	while (SDL_PollEvent(&event)) { } // process pending events
	SDL_Delay(50); // give the OS a chance to redraw

	// Cleanup
	if (texture) SDL_DestroyTexture(texture);

	// ---------------------------

	const std::string kernelSrc = readFile("./gpu-code/particles.cl");

	const char* src = kernelSrc.c_str();
	const cl_program program = clCreateProgramWithSource(context, 1, &src, nullptr, &err);
	if (clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr) != CL_SUCCESS) {
		size_t logSize;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
		std::vector<char> log(logSize);
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
		std::cerr << "Build error:\n" << log.data() << "\n";
		return 1;
	}
	const cl_kernel kernel = clCreateKernel(program, "update_particles", &err);

	// ---------------------------
	// 4. Main loop
	// ---------------------------
	bool running = true;
	Uint64 now = SDL_GetPerformanceCounter();

	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = false;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
		}

		now = SDL_GetPerformanceCounter();
		static Uint64 last = 0;

		if (last != 0) dt = static_cast<float>(now - last) / SDL_GetPerformanceFrequency() / 6;


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
		const int rand = xorshift32() % (N + 1);
		for (int l = rand; l < N + rand; l++) {
			int i = (l + N) % N;

			SDL_Color color;
			switch (species[i]) {
				case 0: color =  {255,   0,   0, 255}; break; // red
				case 1: color =  {255, 255,   0, 255}; break; // yellow
				case 2: color =  {135, 206, 235, 255}; break; // sky blue
				case 3: color =  {  0, 128,   0, 255}; break; // green
				case 4: color =  {23, 100, 255, 255};  break; // abiotic particles
				default: color = {255, 255, 255, 255}; break;
			}

			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			int x = static_cast<int>(particles[i].s[0]);
			int y = static_cast<int>(particles[i].s[1]);
			SDL_RenderDrawCircle(renderer, x, y, 1);
		}

		SDL_RenderPresent(renderer);
		last = now;
	}

	// ---------------------------
	// Cleanup
	// ---------------------------
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();

	clReleaseMemObject(bufParticles);
	clReleaseMemObject(bufSpecies);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	return 0;
}
