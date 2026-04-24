#include <SDL.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <sstream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_WIDTH = 80;
const int GRID_HEIGHT = 60;
const int CELL_SIZE = 10;

enum class CellType {
    EMPTY,
    MANTLE,
    CRUST,
    WATER,
    ICE,
    LAVA
};

struct Cell {
    CellType type;
    float temp; // in Kelvin
    float vx, vy;
    float mass;
};

class PlanetSimulator {
public:
    PlanetSimulator() : grid(GRID_HEIGHT, std::vector<Cell>(GRID_WIDTH)) {
        initGrid();
    }

    void initGrid() {
        int cx = GRID_WIDTH / 2;
        int cy = GRID_HEIGHT / 2;
        int radius = 25;

        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                float dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));

                if (dist < radius - 5) {
                    grid[y][x] = {CellType::MANTLE, 4000.0f, 0, 0, 10.0f};
                } else if (dist < radius - 2) {
                    grid[y][x] = {CellType::CRUST, 300.0f, 0, 0, 8.0f};
                } else if (dist < radius) {
                    grid[y][x] = {CellType::WATER, 280.0f, 0, 0, 1.0f};
                } else {
                    grid[y][x] = {CellType::EMPTY, 3.0f, 0, 0, 0.0f};
                }
            }
        }
    }

    void update() {
        std::vector<std::vector<float>> nextTemp(GRID_HEIGHT, std::vector<float>(GRID_WIDTH, 0));
        std::vector<std::vector<Cell>> nextGrid = grid;

        for (int y = 1; y < GRID_HEIGHT - 1; ++y) {
            for (int x = 1; x < GRID_WIDTH - 1; ++x) {
                if (grid[y][x].type == CellType::MANTLE) {
                    if (grid[y][x].temp > 3500.0f) {
                        grid[y][x].vy -= 0.1f;
                        grid[y][x].vx += (std::rand() % 2 == 0) ? 0.05f : -0.05f;
                    } else if (grid[y][x].temp < 3000.0f) {
                        grid[y][x].vy += 0.1f;
                    }
                } else if (grid[y][x].type == CellType::CRUST && y + 1 < GRID_HEIGHT && grid[y+1][x].type == CellType::MANTLE) {
                    grid[y][x].vx += grid[y+1][x].vx * 0.1f; // Drag
                }

                // Cap velocities
                if (grid[y][x].vx > 1.0f) grid[y][x].vx = 1.0f;
                if (grid[y][x].vx < -1.0f) grid[y][x].vx = -1.0f;
                if (grid[y][x].vy > 1.0f) grid[y][x].vy = 1.0f;
                if (grid[y][x].vy < -1.0f) grid[y][x].vy = -1.0f;

                float t = grid[y][x].temp;
                float tUp = grid[y-1][x].temp;
                float tDown = grid[y+1][x].temp;
                float tLeft = grid[y][x-1].temp;
                float tRight = grid[y][x+1].temp;

                float avgNeighbors = (tUp + tDown + tLeft + tRight) / 4.0f;
                nextTemp[y][x] = t + (avgNeighbors - t) * 0.1f;
            }
        }

        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                if (y > 0 && y < GRID_HEIGHT - 1 && x > 0 && x < GRID_WIDTH - 1) {
                    // Update temp in the double buffer
                    nextGrid[y][x].temp = nextTemp[y][x];

                    // When a cell moves, it must take its temp with it and leave empty space.
                    // This is complex with a simple grid. So we just do phase changes and temp diffusion,
                    // and let movement happen slowly.

                    // Probabilistic movement for fractional velocities
                    int moveX = 0, moveY = 0;
                    if (std::abs(grid[y][x].vx) > (std::rand() % 100) / 100.0f) {
                        moveX = (grid[y][x].vx > 0) ? 1 : -1;
                    }
                    if (std::abs(grid[y][x].vy) > (std::rand() % 100) / 100.0f) {
                        moveY = (grid[y][x].vy > 0) ? 1 : -1;
                    }

                    int nx = x + moveX;
                    int ny = y + moveY;

                    if ((moveX != 0 || moveY != 0) && nx > 0 && nx < GRID_WIDTH - 1 && ny > 0 && ny < GRID_HEIGHT - 1) {
                        if (nextGrid[ny][nx].type == CellType::EMPTY) {
                            nextGrid[ny][nx] = nextGrid[y][x];
                            // Clean up old space
                            nextGrid[y][x].type = CellType::EMPTY;
                            nextGrid[y][x].temp = 3.0f; // cold vacuum
                        } else if (grid[y][x].type == CellType::CRUST && grid[ny][nx].type == CellType::MANTLE && grid[y][x].vy > 0) {
                            // Subduction
                            nextGrid[ny][nx] = nextGrid[y][x];
                            nextGrid[ny][nx].type = CellType::MANTLE;
                            // Clean up old space
                            nextGrid[y][x].type = CellType::WATER;
                            nextGrid[y][x].temp = 280.0f;
                        }
                    }
                }
            }
        }
        grid = nextGrid;

        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                if (grid[y][x].type == CellType::WATER && grid[y][x].temp < 273.15f) {
                    grid[y][x].type = CellType::ICE;
                } else if (grid[y][x].type == CellType::ICE && grid[y][x].temp > 273.15f) {
                    grid[y][x].type = CellType::WATER;
                } else if (grid[y][x].type == CellType::CRUST && grid[y][x].temp > 1500.0f) {
                    grid[y][x].type = CellType::LAVA;
                } else if (grid[y][x].type == CellType::LAVA && grid[y][x].temp < 1200.0f) {
                    grid[y][x].type = CellType::CRUST;
                }
            }
        }
    }



    void drawChar(SDL_Renderer* renderer, char c, int dx, int dy) {
        // Very basic 3x5 font representation for numbers and a few letters
        int font[128][5] = {0};

        font['0'][0]=0x7; font['0'][1]=0x5; font['0'][2]=0x5; font['0'][3]=0x5; font['0'][4]=0x7;
        font['1'][0]=0x2; font['1'][1]=0x6; font['1'][2]=0x2; font['1'][3]=0x2; font['1'][4]=0x7;
        font['2'][0]=0x7; font['2'][1]=0x1; font['2'][2]=0x7; font['2'][3]=0x4; font['2'][4]=0x7;
        font['3'][0]=0x7; font['3'][1]=0x1; font['3'][2]=0x7; font['3'][3]=0x1; font['3'][4]=0x7;
        font['4'][0]=0x5; font['4'][1]=0x5; font['4'][2]=0x7; font['4'][3]=0x1; font['4'][4]=0x1;
        font['5'][0]=0x7; font['5'][1]=0x4; font['5'][2]=0x7; font['5'][3]=0x1; font['5'][4]=0x7;
        font['6'][0]=0x7; font['6'][1]=0x4; font['6'][2]=0x7; font['6'][3]=0x5; font['6'][4]=0x7;
        font['7'][0]=0x7; font['7'][1]=0x1; font['7'][2]=0x1; font['7'][3]=0x1; font['7'][4]=0x1;
        font['8'][0]=0x7; font['8'][1]=0x5; font['8'][2]=0x7; font['8'][3]=0x5; font['8'][4]=0x7;
        font['9'][0]=0x7; font['9'][1]=0x5; font['9'][2]=0x7; font['9'][3]=0x1; font['9'][4]=0x7;
        font['%'][0]=0x5; font['%'][1]=0x1; font['%'][2]=0x2; font['%'][3]=0x4; font['%'][4]=0x5;
        font['E'][0]=0x7; font['E'][1]=0x4; font['E'][2]=0x6; font['E'][3]=0x4; font['E'][4]=0x7;
        font['S'][0]=0x7; font['S'][1]=0x4; font['S'][2]=0x7; font['S'][3]=0x1; font['S'][4]=0x7;
        font['W'][0]=0x5; font['W'][1]=0x5; font['W'][2]=0x5; font['W'][3]=0x7; font['W'][4]=0x5;
        font['M'][0]=0x5; font['M'][1]=0x7; font['M'][2]=0x5; font['M'][3]=0x5; font['M'][4]=0x5;
        font['A'][0]=0x7; font['A'][1]=0x5; font['A'][2]=0x7; font['A'][3]=0x5; font['A'][4]=0x5;
        font['R'][0]=0x7; font['R'][1]=0x5; font['R'][2]=0x7; font['R'][3]=0x6; font['R'][4]=0x5;
        font['T'][0]=0x7; font['T'][1]=0x2; font['T'][2]=0x2; font['T'][3]=0x2; font['T'][4]=0x2;
        font['H'][0]=0x5; font['H'][1]=0x5; font['H'][2]=0x7; font['H'][3]=0x5; font['H'][4]=0x5;
        font['N'][0]=0x6; font['N'][1]=0x5; font['N'][2]=0x5; font['N'][3]=0x5; font['N'][4]=0x5;
        font['O'][0]=0x7; font['O'][1]=0x5; font['O'][2]=0x5; font['O'][3]=0x5; font['O'][4]=0x7;
        font['B'][0]=0x6; font['B'][1]=0x5; font['B'][2]=0x6; font['B'][3]=0x5; font['B'][4]=0x6;
        font['L'][0]=0x4; font['L'][1]=0x4; font['L'][2]=0x4; font['L'][3]=0x4; font['L'][4]=0x7;
        font['C'][0]=0x7; font['C'][1]=0x4; font['C'][2]=0x4; font['C'][3]=0x4; font['C'][4]=0x7;
        font['I'][0]=0x7; font['I'][1]=0x2; font['I'][2]=0x2; font['I'][3]=0x2; font['I'][4]=0x7;
        font['G'][0]=0x7; font['G'][1]=0x4; font['G'][2]=0x5; font['G'][3]=0x5; font['G'][4]=0x7;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for(int y=0; y<5; y++) {
            int row = font[(int)c][y];
            for(int x=0; x<3; x++) {
                if (row & (1 << (2-x))) {
                    SDL_Rect r = {dx + x*4, dy + y*4, 4, 4};
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }
    }


    void drawString(SDL_Renderer* renderer, const std::string& str, int dx, int dy) {
        for(size_t i=0; i<str.length(); i++) {
            drawChar(renderer, toupper(str[i]), dx + i*16, dy);
        }
    }

    void render(SDL_Renderer* renderer) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                SDL_Rect rect = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};

                switch (grid[y][x].type) {
                    case CellType::EMPTY: SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); break;
                    case CellType::MANTLE: SDL_SetRenderDrawColor(renderer, 128, 64, 0, 255); break;
                    case CellType::CRUST: SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); break;
                    case CellType::WATER: SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); break;
                    case CellType::ICE: SDL_SetRenderDrawColor(renderer, 200, 255, 255, 255); break;
                    case CellType::LAVA: SDL_SetRenderDrawColor(renderer, 255, 64, 0, 255); break;
                }

                SDL_RenderFillRect(renderer, &rect);
            }
        }

        int water = 0, ice = 0, crust = 0, lava = 0;
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                switch (grid[y][x].type) {
                    case CellType::WATER: water++; break;
                    case CellType::ICE: ice++; break;
                    case CellType::CRUST: crust++; break;
                    case CellType::LAVA: lava++; break;
                    default: break;
                }
            }
        }

        int totalSurface = water + ice + crust + lava;
        if (totalSurface > 0) {
            int wWidth = (water * WINDOW_WIDTH) / totalSurface;
            int iWidth = (ice * WINDOW_WIDTH) / totalSurface;
            int cWidth = (crust * WINDOW_WIDTH) / totalSurface;
            int lWidth = WINDOW_WIDTH - wWidth - iWidth - cWidth;

            SDL_Rect wRect = {0, WINDOW_HEIGHT - 20, wWidth, 20};
            SDL_Rect iRect = {wWidth, WINDOW_HEIGHT - 20, iWidth, 20};
            SDL_Rect cRect = {wWidth + iWidth, WINDOW_HEIGHT - 20, cWidth, 20};
            SDL_Rect lRect = {wWidth + iWidth + cWidth, WINDOW_HEIGHT - 20, lWidth, 20};

            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); SDL_RenderFillRect(renderer, &wRect);
            SDL_SetRenderDrawColor(renderer, 200, 255, 255, 255); SDL_RenderFillRect(renderer, &iRect);
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); SDL_RenderFillRect(renderer, &cRect);
            SDL_SetRenderDrawColor(renderer, 255, 64, 0, 255); SDL_RenderFillRect(renderer, &lRect);

            // HUD state indicator in the corner
            SDL_Rect stateRect = {10, 10, 30, 30};
            std::string stateStr = "";
            if (ice > water * 2 && ice > crust) {
                SDL_SetRenderDrawColor(renderer, 200, 255, 255, 255); // Snowball
                stateStr = "SNOWBALL";
            }
            else if (water > ice * 2 && water > crust) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Waterworld
                stateStr = "WATERWORLD";
            }
            else if (lava > crust * 2) {
                SDL_SetRenderDrawColor(renderer, 255, 64, 0, 255); // Magma Ocean
                stateStr = "MAGMA OCEAN";
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Earth-like
                stateStr = "EARTH";
            }
            SDL_RenderFillRect(renderer, &stateRect);

            drawString(renderer, stateStr, 50, 10);

            // Draw composition numbers
            int wp = (water * 100) / totalSurface;
            int ip = (ice * 100) / totalSurface;
            int cp = (crust * 100) / totalSurface;
            int lp = (lava * 100) / totalSurface;

            drawString(renderer, std::to_string(wp) + "%", wWidth/2 - 10, WINDOW_HEIGHT - 20);
            drawString(renderer, std::to_string(ip) + "%", wWidth + iWidth/2 - 10, WINDOW_HEIGHT - 20);
            drawString(renderer, std::to_string(cp) + "%", wWidth + iWidth + cWidth/2 - 10, WINDOW_HEIGHT - 20);
            drawString(renderer, std::to_string(lp) + "%", wWidth + iWidth + cWidth + lWidth/2 - 10, WINDOW_HEIGHT - 20);
        }
    }

    void printStats() {
        int water = 0, ice = 0, crust = 0, mantle = 0, lava = 0;
        float totalMass = 0;

        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                totalMass += grid[y][x].mass;
                switch (grid[y][x].type) {
                    case CellType::WATER: water++; break;
                    case CellType::ICE: ice++; break;
                    case CellType::CRUST: crust++; break;
                    case CellType::MANTLE: mantle++; break;
                    case CellType::LAVA: lava++; break;
                    default: break;
                }
            }
        }

        int totalSurface = water + ice + crust + lava;
        if (totalSurface == 0) return;

        std::string state = "Unknown";
        if (ice > water * 2 && ice > crust) state = "Snowball";
        else if (water > ice * 2 && water > crust) state = "Waterworld";
        else if (lava > crust * 2) state = "Magma Ocean";
        else state = "Earth-like";

        // Print to console as the HUD
        std::cout << "\033[2J\033[H"; // Clear console
        std::cout << "Planet Stats:\n";
        std::cout << "Mass: " << totalMass << "\n";
        std::cout << "State: " << state << "\n";
        std::cout << "Composition (Surface):\n";
        std::cout << "  Water: " << (water * 100.0f / totalSurface) << "%\n";
        std::cout << "  Ice: " << (ice * 100.0f / totalSurface) << "%\n";
        std::cout << "  Crust: " << (crust * 100.0f / totalSurface) << "%\n";
        std::cout << "  Lava: " << (lava * 100.0f / totalSurface) << "%\n";
    }

private:
    std::vector<std::vector<Cell>> grid;
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "2D Planet Tectonics Simulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    PlanetSimulator sim;
    bool quit = false;
    SDL_Event e;

    int frameCount = 0;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        sim.update();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        sim.render(renderer);

        SDL_RenderPresent(renderer);

        if (frameCount % 60 == 0) {
            sim.printStats();
        }
        frameCount++;

        SDL_Delay(16); // ~60fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
