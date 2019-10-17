#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL.h>

#include "pi-sim.h"


static const int xsize=XSIZE,ysize=YSIZE;

static SDL_Surface *sdl_screen=NULL;

unsigned char display[640*480*3];
unsigned char red[640*480];
unsigned char blue[640*480];
unsigned char green[640*480];

unsigned char red_palette[256];
unsigned char blue_palette[256];
unsigned char green_palette[256];


int pisim_update(void) {

	unsigned int *t_pointer;

	int x,temp;

	/* point to SDL output pixels */
	t_pointer=((Uint32 *)sdl_screen->pixels);

	for(x=0;x<xsize*ysize;x++) {

		temp=(red[x]<<16)|(green[x]<<8)|(blue[x]<<0)|0;

		t_pointer[x]=temp;
	}

	SDL_UpdateRect(sdl_screen, 0, 0, xsize, ysize);

	return 0;
}

int pisim_init(void) {

	int mode;
	int x;

	mode=SDL_SWSURFACE|SDL_HWPALETTE|SDL_HWSURFACE;

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr,
			"Couldn't initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	/* Clean up on exit */
	atexit(SDL_Quit);

	/* assume 32-bit color */
	sdl_screen = SDL_SetVideoMode(xsize, ysize, 32, mode);

	if ( sdl_screen == NULL ) {
		fprintf(stderr, "ERROR!  Couldn't set %dx%d video mode: %s\n",
			xsize,ysize,SDL_GetError());
		return -1;
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	/* Init screen */
	for(x=0;x<640*480*3;x++) display[x]=0;

	return 0;
}

int pisim_input(void) {

        SDL_Event event;
        int keypressed;


        while ( SDL_PollEvent(&event)) {

                switch(event.type) {

                case SDL_KEYDOWN:
                        keypressed=event.key.keysym.sym;
                        switch (keypressed) {

                        case SDLK_ESCAPE:
                                return 27;
                        case 'a'...'z':
                        case 'A'...'Z':
                                return keypressed;
                        case SDLK_UP:
                                return 11;
                        case SDLK_DOWN:
                                return 10;
                        case SDLK_RIGHT:
                                return 21;
                        case SDLK_LEFT:
                                return 8;
                        default:
                                printf("Unknown %d\n",keypressed);
                                return keypressed;
                        }
                        break;


                case SDL_JOYBUTTONDOWN:
                case SDL_JOYAXISMOTION:
                        printf("Joystick!\n");
                        break;

                default:
                        printf("Unknown input action!\n");
                        break;

                }
        }

        return 0;
}
