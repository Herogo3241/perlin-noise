#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600
#define GRID_SIZE 50
#define SCALE 50.0f

#define M_PI 3.14159265358979323846




// Linear interpolation
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Fade function to smooth the interpolation
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Generate random gradient vectors
void generate_gradients(float gradients[][2], int size) {
    for (int i = 0; i < size; i++) {
        float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        gradients[i][0] = cos(angle);
        gradients[i][1] = sin(angle);
    }
}

// Dot product of gradient and distance vectors
float dot_grid_gradient(float gradients[][2], int grid_x, int grid_y, float x, float y) {
    float dx = x - (float)grid_x;
    float dy = y - (float)grid_y;
    return dx * gradients[grid_x + grid_y * GRID_SIZE][0] + dy * gradients[grid_x + grid_y * GRID_SIZE][1];
}

// Perlin noise function
float perlin_noise(float gradients[][2], float x, float y) {
    int x0 = (int)floor(x);
    int x1 = x0 + 1;
    int y0 = (int)floor(y);
    int y1 = y0 + 1;

    float sx = fade(x - (float)x0);
    float sy = fade(y - (float)y0);

    float n0 = dot_grid_gradient(gradients, x0, y0, x, y);
    float n1 = dot_grid_gradient(gradients, x1, y0, x, y);
    float ix0 = lerp(n0, n1, sx);

    n0 = dot_grid_gradient(gradients, x0, y1, x, y);
    n1 = dot_grid_gradient(gradients, x1, y1, x, y);
    float ix1 = lerp(n0, n1, sx);

    return lerp(ix0, ix1, sy);
}

// Map noise to a gradient of colors
void map_to_color(float noise, Uint8 *r, Uint8 *g, Uint8 *b) {
    if (noise < -0.5f) { // Water
        *r = 0;
        *g = 0;
        *b = (Uint8)(128 + 127 * (noise + 0.5f));
    } else if (noise < 0.0f) { // Shoreline
        *r = (Uint8)(128 * (noise + 0.5f));
        *g = (Uint8)(128 + 127 * (noise + 0.5f));
        *b = (Uint8)(255 * (noise + 0.5f));
    } else if (noise < 0.5f) { // Grassland
        *r = (Uint8)(64 * noise);
        *g = (Uint8)(128 + 127 * noise);
        *b = (Uint8)(64 * noise);
    } else { // Mountains
        *r = (Uint8)(128 + 127 * (noise - 0.5f));
        *g = (Uint8)(128 + 127 * (noise - 0.5f));
        *b = (Uint8)(128 + 127 * (noise - 0.5f));
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Perlin Noise - Textured", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    float gradients[GRID_SIZE * GRID_SIZE][2];
    generate_gradients(gradients, GRID_SIZE * GRID_SIZE);

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float nx = (float)x / SCALE;
                float ny = (float)y / SCALE;
                float noise = perlin_noise(gradients, nx, ny);

                Uint8 r, g, b;
                map_to_color(noise, &r, &g, &b);
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
