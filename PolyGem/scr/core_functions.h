#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "core.h"

SDL_Texture* LoadTexture(std::string path, SDL_Renderer* renderer, SDL_Rect* rect);
SDL_Surface* SDL_CreateSurfaceFromTexture(SDL_Renderer* renderer, SDL_Texture* texture);
SDL_Surface* blurSurface(SDL_Surface* surface, uint8_t kernel);
SDL_Surface* blurSurfaceMagic(SDL_Surface* surface, uint8_t kernel, uint8_t power);
SDL_Surface* upsample2x(SDL_Surface* surface);
SDL_Surface* downsample2x(SDL_Surface* surface);

void drawLineThickness(SDL_Renderer* renderer, plg::Vec2 start, plg::Vec2 end, int thickness, SDL_Color color);
void drawArc(SDL_Renderer* renderer, int x, int y, int radius1, int radius2, double angle_start, double angle_stop, SDL_Color color);
void drawCircle(SDL_Renderer* renderer, int x0, int y0, int radius, SDL_Color color);
void drawCircleFilled(SDL_Renderer* renderer, int x0, int y0, int radius, SDL_Color color);
void drawEllipseFilled(SDL_Renderer* renderer, int x0, int y0, int width, int height, SDL_Color color);
void drawEllipseThickness(SDL_Renderer* renderer, int x0, int y0, int width, int height, int thickness, SDL_Color color);
void drawRectRound(SDL_Renderer* renderer, SDL_Rect rect, int radius, SDL_Color color);
void drawRawPolygon(SDL_Renderer* renderer, std::initializer_list<plg::Vec2> vertex_list, SDL_Color color);
void drawPolygon(SDL_Renderer* renderer, std::initializer_list<plg::Vec2> vertex_list, SDL_Color color);