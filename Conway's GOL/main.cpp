#include <SDL.h>
#include <memory>
#include <ctime>
#include <thread>
#include <chrono>

// SDL Window and Surface for pixel manipultion
SDL_Window* window = NULL;
SDL_Surface* surface = NULL;

// Width and height of cell in pixels
unsigned int CELL_SIZE = 5;

unsigned CELLMAP_WIDTH = 200;
unsigned CELLMAP_HEIGHT = 200;

unsigned int SCREEN_WIDTH = CELLMAP_WIDTH * CELL_SIZE;
unsigned int SCREEN_HEIGHT = CELLMAP_HEIGHT * CELL_SIZE;

class CellMap
{
public:
	CellMap(unsigned int width, unsigned int height);
	~CellMap();
	void SetCell(unsigned int x, unsigned int y);
	void ClearCell(unsigned int x, unsigned int y);
	void Init();
	int CellState(unsigned int x, unsigned int y);
	void NextGen();
private:
	unsigned char* cells;
	unsigned char* temp_cells;
	unsigned int w, h;
	unsigned int length;
};

void DrawCell(unsigned int x, unsigned int y, unsigned int color)
{
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * CELL_SIZE * SCREEN_WIDTH + x * CELL_SIZE) * 4;

	for (unsigned int i = 0; i < CELL_SIZE; i++)
	{
		for (unsigned j = 0; j < CELL_SIZE; j++)
		{
			*(pixel_ptr + j * 4) = color;
			*(pixel_ptr + j * 4 + 1) = color;
			*(pixel_ptr + j * 4 + 2) = color;
		}
		pixel_ptr += SCREEN_WIDTH * 4;
	}
}

CellMap::CellMap(unsigned int width, unsigned int height) : w(width), h(height)
{
	length = w * h;
	cells = new unsigned char[length];
	temp_cells = new unsigned char[length];
	memset(cells, 0, length);
}

CellMap::~CellMap()
{
	delete[] cells;
	delete[] temp_cells;
}

void CellMap::SetCell(unsigned int x, unsigned int y)
{
	unsigned char* cell_ptr = cells + (y * w) + x;
	int xleft, xright, yabove, ybelow;

	*(cell_ptr) |= 0x01; // Set first bit as 1, 'on'

	if (x == 0)
		xleft = static_cast<int>(w) - 1;
	else
		xleft = -1;

	if (x == (w - 1))
		xright = -(static_cast<int>(w) - 1);
	else
		xright = 1;

	if (y == 0)
		yabove = static_cast<int>(length) - static_cast<int>(w);
	else
		yabove = -static_cast<int>(w);

	if (y == (h - 1))
		ybelow = -(static_cast<int>(length) - static_cast<int>(w));
	else
		ybelow = static_cast<int>(w);

	*(cell_ptr + yabove + xleft) += 0x02;
	*(cell_ptr + yabove) += 0x02;
	*(cell_ptr + yabove + xright) += 0x02;
	*(cell_ptr + xleft) += 0x02;
	*(cell_ptr + xright) += 0x02;
	*(cell_ptr + ybelow + xleft) += 0x02;
	*(cell_ptr + ybelow) += 0x02;
	*(cell_ptr + yabove + xright) += 0x02;
}

void CellMap::ClearCell(unsigned int x, unsigned int y)
{
	unsigned char* cell_ptr = cells + (y * w) + x;
	int xleft, xright, yabove, ybelow;

	*(cell_ptr) &= ~0x01; // Set first bit as 0, 'off'

	if (x == 0)
		xleft = static_cast<int>(w) - 1;
	else
		xleft = -1;

	if (x == (w - 1))
		xright = -(static_cast<int>(w) - 1);
	else
		xright = 1;

	if (y == 0)
		yabove = static_cast<int>(length) - static_cast<int>(w);
	else
		yabove = -static_cast<int>(w);

	if (y == (h - 1))
		ybelow = -(static_cast<int>(length) - static_cast<int>(w));
	else
		ybelow = static_cast<int>(w);

	*(cell_ptr + yabove + xleft) -= 0x02;
	*(cell_ptr + yabove) -= 0x02;
	*(cell_ptr + yabove + xright) -= 0x02;
	*(cell_ptr + xleft) -= 0x02;
	*(cell_ptr + xright) -= 0x02;
	*(cell_ptr + ybelow + xleft) -= 0x02;
	*(cell_ptr + ybelow) -= 0x02;
	*(cell_ptr + yabove + xright) -= 0x02;
}

int CellMap::CellState(unsigned int x, unsigned int y)
{
	unsigned char* cell_ptr = cells + (y * w) + x;

	return *cell_ptr & 0x01;
}

void CellMap::Init()
{
	unsigned int seed = (unsigned)time(NULL);

	srand(seed);

	unsigned int x, y;

	for (int i = 0; i < length * 0.5; i++)
	{
		x = rand() % (w - 1);
		y = rand() % (h - 1);

		if (!CellState(x, y))
			SetCell(x, y);
	}
}

void CellMap::NextGen()
{
	unsigned int x, y, live_neighbours;
	unsigned char* cell_ptr;

	memcpy(temp_cells, cells, length);

	cell_ptr = temp_cells;

	for (unsigned int y = 0; y < h; y++)
	{
		x = 0;
		do
		{
			// Skipping
			while (*cell_ptr == 0)
			{
				cell_ptr++;

				if (++x >= w) goto NextRow;
			}

			live_neighbours = *cell_ptr >> 1;
			if (*cell_ptr & 0x01)
			{
				if ((live_neighbours != 2) && (live_neighbours != 3))
				{
					ClearCell(x, y);
					DrawCell(x, y, 0x00);
				}
			}
			else
			{
				if (live_neighbours == 3)
				{
					SetCell(x, y);
					DrawCell(x, y, 0xFF);
				}
			}

			cell_ptr++;

		} while (++x < w);

	NextRow:;
	}
}

int main(int argc, char* argv[])
{
	// SDL Boilerplate
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(
		"Conway's Game of Life",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);
	surface = SDL_GetWindowSurface(window);

	// SDL Event Handler
	SDL_Event e;

	CellMap map(CELLMAP_WIDTH, CELLMAP_HEIGHT);
	map.Init();

	// Rendering Loop
	bool quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
			if (e.type == SDL_QUIT) quit = true;

		map.NextGen();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		// Update frame buffer
		SDL_UpdateWindowSurface(window);
	}

	// Clean up SDL
	SDL_DestroyWindow(window);
	// Quit SDL
	SDL_Quit();

	return 0;
}