#include <SDL2/SDL.h>
#include <time.h>

#define SPREAD (map.w + pole->w + 80)
#define RANDOM_HEIGHT (rand() % (int)(map.h - map.ground - pole->gap - 120) + 60)
#define INITIAL_VEL -11.1f

enum {
	ALIVE,
	DEAD,
} gamestate = DEAD;

typedef struct {
	const float w;
	const float h;
	const float ground;
} Map;
Map map = { 480, 600, 80 };

typedef struct {
	float x;
	float y;
	const float size;
	float vel;
	float frame;
} Player;

typedef struct {
	float x[2];
	float y[2];
	const float w;
	const float gap;
} Pole;

typedef struct Textures {
	SDL_Texture *back;
	SDL_Texture *pole;
	SDL_Texture *bird;
} Textures;

void
game_new(Player *player, Pole *pole)
{
	gamestate = ALIVE;
	player->y = (map.h - map.ground) / 2;
	player->vel = INITIAL_VEL;
	pole->x[0] = SPREAD + SPREAD / 2 - pole->w;
	pole->x[1] = SPREAD - pole->w;
	pole->y[0] = RANDOM_HEIGHT;
	pole->y[1] = RANDOM_HEIGHT;
}

void
game_end()
{
	gamestate = DEAD;
}

void
upd_pole(Player *player, Pole *pole, int i)
{
	if (player->x + player->size >= pole->x[i] && player->x <= pole->x[i] + pole->w &&
			(player->y <= pole->y[i] || player->y + player->size >= pole->y[i] + pole->gap))
		game_end();

	pole->x[i] -= 5;

	if (pole->x[i] <= -pole->w)
	{
		pole->x[i] = SPREAD - pole->w;
		pole->y[i] = RANDOM_HEIGHT;
	}
}

void
upd(Player *player, Pole *pole)
{
	if (gamestate != ALIVE)
		return;

	player->y += player->vel;
	player->vel += 0.61;

	if (player->vel > 10.0f)
		player->frame = 0;
	else
		player->frame -= (player->vel - 10.0f) * 0.03f;

	if (player->y > map.h - map.ground - player->size)
		game_end();

	for (int i = 0; i < 2; i++)
		upd_pole(player, pole, i);
}

void
draw(SDL_Renderer *renderer, Textures textures, Player player, Pole pole)
{
	SDL_Rect dest = {0, 0, map.w, map.h};
	SDL_RenderCopy(renderer, textures.back, NULL, &dest);

	for (int i = 0; i < 2; i++)
	{
		SDL_Rect pole_topd = { pole.x[i], pole.y[i] - map.h, pole.w, map.h };
		SDL_RenderCopy(renderer, textures.pole, NULL, &pole_topd);

		int lower = pole.y[i] + pole.gap;
		SDL_Rect pole_bots = { 0, 0, 86, map.h - lower - map.ground };
		SDL_Rect pole_botd = { pole.x[i], lower, pole.w, pole_bots.h };
		SDL_RenderCopy(renderer, textures.pole, &pole_bots, &pole_botd);
	}

	SDL_Rect dbird = {player.x, player.y, player.size, player.size};
	SDL_RenderCopy(renderer, textures.bird, NULL, &dbird);

	SDL_RenderPresent(renderer);
}

void
init(SDL_Window **window, SDL_Renderer **renderer, Textures *textures)
{
	SDL_Init(SDL_INIT_VIDEO);

	*window = SDL_CreateWindow("clappy", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, map.w, map.h, SDL_WINDOW_SHOWN);

	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_PRESENTVSYNC);

	SDL_Surface *surface;

	surface = SDL_LoadBMP("assets/back.bmp");
	textures->back = SDL_CreateTextureFromSurface(*renderer, surface);
	SDL_FreeSurface(surface);

	surface = SDL_LoadBMP("assets/pole.bmp");
	textures->pole = SDL_CreateTextureFromSurface(*renderer, surface);
	SDL_FreeSurface(surface);

	surface = SDL_LoadBMP("assets/bird.bmp");
	textures->bird = SDL_CreateTextureFromSurface(*renderer, surface);
	SDL_FreeSurface(surface);
}

void
deinit(SDL_Window **window, SDL_Renderer **renderer, Textures *textures)
{
	SDL_DestroyTexture(textures->back);
	SDL_DestroyTexture(textures->pole);
	SDL_DestroyTexture(textures->bird);

	SDL_DestroyRenderer(*renderer);

	SDL_DestroyWindow(*window);

	SDL_Quit();
}

int
main()
{
	srand(time(NULL));

	SDL_Window *window;
	SDL_Renderer *renderer;
	Textures textures;
	init(&window, &renderer, &textures);

	Player player = { 80, (map.h - map.ground) / 2, 50, 0, 0 };

	Pole pole = { {map.w, map.w}, {0, 0}, 86, 220 };

	for (;;)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
			{
				case SDL_QUIT:
					goto quit;
				case SDL_KEYDOWN:
					if (gamestate == ALIVE)
					{
						player.vel = INITIAL_VEL;
						player.frame += 1.0f;
					}
					else
						game_new(&player, &pole);
			}

		upd(&player, &pole);
		draw(renderer, textures, player, pole);
		SDL_Delay(1000 / 60);
	}

quit:
	deinit(&window, &renderer, &textures);
}
