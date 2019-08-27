/* Written by Erik Nick
 *
 * main.c
 * 
 */
 
#include "gba.h"
#include <stdio.h>
#include <stdlib.h>

void sync()
{
	while(REG_DISPLAY_VCOUNT >= 160);
	while(REG_DISPLAY_VCOUNT < 160);
}

uint16 makeColor(uint8 r, uint8 g, uint8 b)
{
	return (r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10);
}

uint32 clamp(int value, int min, int max)
{
	return (value < min ? min : (value > max ? max : value));
}

void drawRect(struct Rect rect, uint16 color)
{
	for(int y = 0; y < rect.h; y++)
	{
		for(int x = 0; x < rect.w; x++)
		{
			SCREENBUFFER[(rect.y + y) * SCREEN_WIDTH + rect.x + x] = color;
		}
	}
}

void createSegment(uint8 dir, int tileSize)
{
	int i;
	int horizSpeed, vertSpeed;
	
	if(dir == DIR_DOWN)
	{
		horizSpeed = 0; 
		vertSpeed = 1;
	}
	else if(dir == DIR_UP)
	{
		horizSpeed = 0; 
		vertSpeed = -1;
	}
	else if(dir == DIR_RIGHT)
	{
		horizSpeed = 1; 
		vertSpeed = 0;
	}
	else if(dir == DIR_LEFT)
	{
		horizSpeed = -1; 
		vertSpeed = 0;
	}
	
	lastSegment++;
	segments[lastSegment].x = segments[lastSegment-1].x - horizSpeed * tileSize;
	segments[lastSegment].y = segments[lastSegment-1].y - vertSpeed * tileSize;
	segments[lastSegment].w = segments[lastSegment].h = tileSize - 2;
	segments[lastSegment].used = 1;
}

int main()
{
	REG_DISPLAY = VIDEOMODE | BGMODE;
	
	int tileSize = 8;
	
	int frameCounter = 0;
	
	int score = 0;
	uint8 dir = DIR_RIGHT;
	
	int widthInTiles = SCREEN_WIDTH/tileSize;
	int heightInTiles = SCREEN_HEIGHT/tileSize;
	
	int startX = widthInTiles/2;
	int startY = heightInTiles/2;
	
	segments[0].x = startX * tileSize + 1;
	segments[0].y = startY * tileSize + 1;
	segments[0].w = segments[0].h = tileSize - 2;
	segments[0].used = 1;
	
	food.x = rand()%30 * tileSize+1;
	food.y = rand()%20 * tileSize+1;
	food.w = food.h = tileSize - 3;
	
	createSegment(dir, tileSize);
	createSegment(dir, tileSize);
	
	bool aDown = false;

	while(1)
	{
		sync();
		
		//wall collision
		if (segments[0].x + segments[0].w >= SCREEN_WIDTH || segments[0].x <= 0 || 
				segments[0].y + segments[0].h >= SCREEN_HEIGHT || segments[0].y < 0)
		{		
			lastSegment = 0;
			blank.x = blank.y = 0;
			blank.h = 160;
			blank.w = 240;
			drawRect(blank, makeColor(0,0,0));
			
			for (int i = 1; i < MAX_LENGTH; i++)
			{
				segments[i].used = false;
			}
			exit(1);
			
			segments[0].x = startX * tileSize + 1;
			segments[0].y = startY * tileSize + 1;
			segments[0].w = segments[0].h = tileSize - 2;
			segments[0].used = 1; 
		}
		
		//snake collision (colliding with itself
		for (int i = 1; i < MAX_LENGTH; i++)
		{
			if ((segments[0].x + segments[0].w >= segments[i].x && segments[0].x + segments[0].w <= segments[i].x + segments[i].w)
				&& (segments[0].y >= segments[i].y && segments[0].y <= segments[i].y + segments[i].h))
				{
					lastSegment = 0;
					blank.x = blank.y = 0;
					blank.h = 160;
					blank.w = 240; 
					drawRect(blank, makeColor(0,0,0));
					
					for (int i = 1; i < MAX_LENGTH; i++)
					{
						segments[i].used = false;
					}
					exit(1);
				}
		}
		
		if(!((REG_KEY_INPUT) & DOWN))
		{
			if(dir != DIR_UP)
			{
				dir = DIR_DOWN;
			}
		}
		if(!((REG_KEY_INPUT) & UP))
		{
			if(dir != DIR_DOWN)
			{
				dir = DIR_UP;
			}
		}
		if(!((REG_KEY_INPUT) & RIGHT))
		{
			if(dir != DIR_LEFT)
			{
				dir = DIR_RIGHT;
			}
		}
		if(!((REG_KEY_INPUT) & LEFT))
		{
			if(dir != DIR_RIGHT)
			{
				dir = DIR_LEFT;	
			}
		}
		
		if(!((REG_KEY_INPUT) & A) && !aDown)
		{
			createSegment(dir, tileSize);
			aDown = true;
		}
		else if(((REG_KEY_INPUT) & A))
		{
			aDown = false;
		}
		
		if(frameCounter % 10 == 0)
		{
			frameCounter = 0;
			int i;

			drawRect(food, makeColor(0x1f,0x00,0x00));
			
			//eat the food
			if (segments[0].x == food.x && segments[0].y == food.y)
			{
				food.x = rand()%30 * tileSize+1;
				food.y = rand()%20 * tileSize+1;
				food.w = food.h = tileSize - 3;
				score++;
				createSegment(dir, tileSize);
				
				//gradually speed up as you eat more food
				frameCounter += 25;

			}
			drawRect(segments[lastSegment], makeColor(0,0,0));
			
			
			for(i = lastSegment; i >= 0; i--)
			{
				if(i != 0)
				{
					segments[i].x = segments[i-1].x;
					segments[i].y = segments[i-1].y;
				}
				else
				{
					if(dir == DIR_DOWN)
						segments[i].y += tileSize;
					else if(dir == DIR_UP)
						segments[i].y -= tileSize;
					else if(dir == DIR_RIGHT)
						segments[i].x += tileSize;
					else if(dir == DIR_LEFT)
						segments[i].x -= tileSize;
					drawRect(food, makeColor(0x1f,0,0x1f));
					drawRect(segments[i], makeColor(0x1f, 0x00, 0x00));
				}

			}
		}
		frameCounter++;
	}
	return 0;
}