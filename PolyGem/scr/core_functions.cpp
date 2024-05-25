#include "core_functions.h"
#include <math.h>
#include <string>

static SDL_Color s_GetPixelColor(const SDL_Surface* surface, const int X, const int Y) {
	const uint8_t Bpp = surface->format->BytesPerPixel;
	uint8_t* pixel = (uint8_t*)surface->pixels + Y * surface->pitch + X * Bpp;
	uint32_t pixelData = *(uint32_t*)pixel;
	return { (uint8_t)((pixelData & 0x00ff0000) >> 16), (uint8_t)((pixelData & 0x0000ff00) >> 8), (uint8_t)(pixelData & 0x000000ff), SDL_ALPHA_OPAQUE };
}

static uint32_t s_GetRawPixelColor(const SDL_Surface* surface, const int X, const int Y) {
	const uint8_t Bpp = surface->format->BytesPerPixel;
	uint8_t* pixel = (uint8_t*)surface->pixels + Y * surface->pitch + X * Bpp;
	return (*(uint32_t*)pixel);
}

static void s_SetPixelColor(const SDL_Surface* surface, const int X, const int Y, SDL_Color color) {
	const uint8_t Bpp = surface->format->BytesPerPixel;
	uint8_t* pixel = (uint8_t*)surface->pixels + Y * surface->pitch + X * Bpp;
	uint32_t* pixelData = (uint32_t*)pixel;
	*pixelData = (*pixelData & 0xff000000) | ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | ((uint32_t)color.b);
}

static void s_SetRawPixelColor(const SDL_Surface* surface, const int X, const int Y, const uint32_t color) {
	const uint8_t Bpp = surface->format->BytesPerPixel;
	uint8_t* pixel = (uint8_t*)surface->pixels + Y * surface->pitch + X * Bpp;
	uint32_t* pixelData = (uint32_t*)pixel;
	*pixelData = color;
}

SDL_Texture* LoadTexture(std::string path, SDL_Renderer* renderer, SDL_Rect* rect = NULL) {
	SDL_Texture* newText = NULL;
	SDL_Surface* loadedSurf = IMG_Load(path.c_str());
	if (loadedSurf == NULL) {
		std::cout << "Unable to load the image!" << SDL_GetError() << std::endl;
	}
	else {
		if (rect) {
			rect->h = loadedSurf->h;
			rect->w = loadedSurf->w;
		}
		newText = SDL_CreateTextureFromSurface(renderer, loadedSurf);
		SDL_FreeSurface(loadedSurf);
	}
	return newText;
}

SDL_Surface* SDL_CreateSurfaceFromTexture(SDL_Renderer* renderer, SDL_Texture* texture) {
	SDL_Texture* target = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, texture);
	int width, height;
	SDL_QueryTexture(texture, NULL, NULL, &width, &height);
	SDL_Surface* newSurface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_RenderReadPixels(renderer, NULL, newSurface->format->format, newSurface->pixels, newSurface->pitch);
	SDL_SetRenderTarget(renderer, target);
	return newSurface;
}

SDL_Surface* blurSurface(SDL_Surface* surface, uint8_t kernel) {
	int halfKernel = kernel / 2;
	int _halfKernel = halfKernel + 1;
	float invKernel = 1.0f / std::powf((float)kernel, 2);
	SDL_Surface* blurredSurface = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0);
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			float r = 0.0f, g = 0.0f, b = 0.0f;
			for (int dy = -halfKernel; dy < _halfKernel; dy++) {
				int posY = y + dy;
				for (int dx = -halfKernel; dx < _halfKernel; dx++) {
					int posX = x + dx;
					if (posX < 0 || posX > surface->w || posY < 0 || posY > surface->h)
						continue;
					SDL_Color prevColor = s_GetPixelColor(surface, posX, posY);
					r += prevColor.r;
					g += prevColor.g;
					b += prevColor.b;
				}
			}
			r = r * invKernel;
			g = g * invKernel;
			b = b * invKernel;
			s_SetPixelColor(blurredSurface, x, y, { (uint8_t)r, (uint8_t)g, (uint8_t)b, SDL_ALPHA_OPAQUE });
		}
	}
	SDL_FreeSurface(surface);
	return blurredSurface;
}

SDL_Surface* blurSurfaceMagic(SDL_Surface* surface, uint8_t kernel, uint8_t power) {
	int halfKernel = kernel / 2;
	int _halfKernel = halfKernel + 1;
	
	float sumKernel = 0.0f;
	for (int dy = -halfKernel; dy < _halfKernel; dy++)
		for (int dx = -halfKernel; dx < _halfKernel; dx++)
			sumKernel += std::powf(kernel - std::abs(dx) - std::abs(dy), power);
	float invKernel = 1.0f / sumKernel;

	SDL_Surface* blurredSurface = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0);
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			float r = 0.0f, g = 0.0f, b = 0.0f;
			for (int dy = -halfKernel; dy < _halfKernel; dy++) {
				int posY = y + dy;
				for (int dx = -halfKernel; dx < _halfKernel; dx++) {
					int posX = x + dx;
					if (posX < 0 || posX > surface->w || posY < 0 || posY > surface->h)
						continue;
					SDL_Color prevColor = s_GetPixelColor(surface, posX, posY);
					float magicVal = std::powf(kernel - std::abs(dx) - std::abs(dy), power);
					r += prevColor.r * magicVal;
					g += prevColor.g * magicVal;
					b += prevColor.b * magicVal;
				}
			}
			r = r * invKernel;
			g = g * invKernel;
			b = b * invKernel;
			s_SetPixelColor(blurredSurface, x, y, { (uint8_t)r, (uint8_t)g, (uint8_t)b, SDL_ALPHA_OPAQUE });
		}
	}
	SDL_FreeSurface(surface);
	return blurredSurface;
}

SDL_Surface* upsample2x(SDL_Surface* surface) {
	SDL_Surface* scaledSurface = SDL_CreateRGBSurface(0, surface->w * 2, surface->h * 2, 32, 0, 0, 0, 0);
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			uint32_t P = s_GetRawPixelColor(surface, x, y);
			uint32_t A = s_GetRawPixelColor(surface, x, y - (y > 0));
			uint32_t B = s_GetRawPixelColor(surface, x + (x < surface->w - 1), y);
			uint32_t C = s_GetRawPixelColor(surface, x - (x > 0), y);
			uint32_t D = s_GetRawPixelColor(surface, x, y + (x < surface->h - 1));

			int dx = 2 * x, dy = 2 * y;
			s_SetRawPixelColor(scaledSurface, dx, dy, P);
			s_SetRawPixelColor(scaledSurface, dx + 1, dy, P);
			s_SetRawPixelColor(scaledSurface, dx, dy + 1, P);
			s_SetRawPixelColor(scaledSurface, dx + 1, dy + 1, P);
			if (C == A) {
				s_SetRawPixelColor(scaledSurface, dx, dy, A);
			}
			if (A == B) {
				s_SetRawPixelColor(scaledSurface, dx + 1, dy, B);
			}
			if (D == C) {
				s_SetRawPixelColor(scaledSurface, dx, dy + 1, C);
			}
			if (B == D) {
				s_SetRawPixelColor(scaledSurface, dx + 1, dy + 1, D);
			}
		}
	}
	return scaledSurface;
}

SDL_Surface* downsample2x(SDL_Surface* surface) {
	SDL_Surface* sampledSurface = SDL_CreateRGBSurface(0, surface->w / 2, surface->h / 2, 32, 0, 0, 0, 0);
	for (int y = 0; y < sampledSurface->h; y++) {
		for (int x = 0; x < sampledSurface->w; x++) {
			int dx = 2 * x, dy = 2 * y;
			SDL_Color A = s_GetPixelColor(surface, dx, dy);
			SDL_Color B = s_GetPixelColor(surface, dx + 1, dy);
			SDL_Color C = s_GetPixelColor(surface, dx, dy + 1);
			SDL_Color D = s_GetPixelColor(surface, dx + 1, dy + 1);

			SDL_Color P = {(A.r + B.r + C.r + D.r) / 4, (A.g + B.g + C.g + D.g) / 4, (A.b + B.b + C.b + D.b) / 4, SDL_ALPHA_OPAQUE };
			s_SetPixelColor(sampledSurface, x, y, P);
		}
	}
	return sampledSurface;
}

void drawRawPolygon(SDL_Renderer* renderer, std::initializer_list<plg::Vec2> vertex_list, SDL_Color color) {
	container::List<plg::Vec2> vertices(vertex_list);
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	for (int i = 0; i < vertices.GetCapacity(); i++) {
		plg::Vec2 vertex_s = vertices[i];
		plg::Vec2 vertex_e = vertices[(i + 1) % vertices.GetCapacity()];
		SDL_RenderDrawLine(renderer, (int)vertex_s.x, (int)vertex_s.y, (int)vertex_e.x, (int)vertex_e.y);
	}
}

void drawPolygon(SDL_Renderer* renderer, std::initializer_list<plg::Vec2> vertex_list, SDL_Color color) {
	container::List<plg::Vec2> vertices(vertex_list);
	plg::Vec2 topVertex = vertices[0];
	for (int i = 1; i < vertices.GetCapacity() - 1; i++) {
		plg::Vec2 vertex = vertices[i];
		plg::Vec2 _vertex = vertices[i + 1];
		SDL_Vertex vert[3];
		vert[0].position.x = topVertex.x;
		vert[0].position.y = topVertex.y;
		vert[0].color = color;

		vert[1].position.x = vertex.x;
		vert[1].position.y = vertex.y;
		vert[1].color = color;
		
		vert[2].position.x = _vertex.x;
		vert[2].position.y = _vertex.y;
		vert[2].color = color;
		
		SDL_RenderGeometry(renderer, NULL, vert, 3, NULL, 0);
	}
}

void drawLineThickness(SDL_Renderer* renderer, plg::Vec2 start, plg::Vec2 end, int thickness, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	int halfThick = thickness / 2;
	plg::Vec2 offset = end - start;
	int x_state = offset.x / std::abs(offset.x);
	int y_state = offset.y / std::abs(offset.y);
	float step_x, step_y, limit;
	if (std::abs(offset.x) > std::abs(offset.y)) {
		step_x = std::abs(offset.x / offset.y) * x_state;
		step_y = y_state;
		limit = offset.y;
	}
	else {
		step_x = x_state;
		step_y = std::abs(offset.y / offset.x) * y_state;
		limit = offset.x;
	}

	for (int i = 0; i < (int)limit; i++) {
		drawCircleFilled(renderer, start.x + step_x * i, start.y + step_y * i, halfThick, color);
	}
}

void drawArc(SDL_Renderer* renderer, int x, int y, int radius1, int radius2, double angle_start, double angle_stop, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	double aStep;
	double a;
	int x_last, x_next, y_last, y_next;

	if (radius1 < radius2) {
		if (radius1 < 1.0e-4) {
			aStep = 1.0;
		}
		else {
			aStep = asin(2.0 / radius1);
		}
	}
	else {
		if (radius2 < 1.0e-4) {
			aStep = 1.0;
		}
		else {
			aStep = asin(2.0 / radius2);
		}
	}

	if (aStep < 0.05) {
		aStep = 0.05;
	}

	x_last = (int)(x + cos(angle_start) * radius1);
	y_last = (int)(y - sin(angle_start) * radius2);
	for (a = angle_start + aStep; a < aStep + angle_stop; a += aStep) {
		x_next = (int)(x + cos(std::min(a, angle_stop)) * radius1);
		y_next = (int)(y - sin(std::min(a, angle_stop)) * radius2);
		SDL_RenderDrawLine(renderer, x_last, y_last, x_next, y_next);
		x_last = x_next;
		y_last = y_next;
	}
}

void drawCircle(SDL_Renderer* renderer, int x0, int y0, int radius, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		if (f >= 0) {
			SDL_RenderDrawPoint(renderer, x0 - x, y0 + y - 1);
			SDL_RenderDrawPoint(renderer, x0 + x - 1, y0 + y - 1);
			SDL_RenderDrawPoint(renderer, x0 - x, y0 - y);
			SDL_RenderDrawPoint(renderer, x0 + x - 1, y0 - y);
		}
		SDL_RenderDrawPoint(renderer, x0 - y, y0 + x - 1);
		SDL_RenderDrawPoint(renderer, x0 + y - 1, y0 + x - 1);
		SDL_RenderDrawPoint(renderer, x0 - y, y0 - x);
		SDL_RenderDrawPoint(renderer, x0 + y - 1, y0 - x);
	}
}

void drawCircleFilled(SDL_Renderer* renderer, int x0, int y0, int radius, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		if (f >= 0) {
			SDL_RenderDrawLine(renderer, x0 - x, y0 + y - 1, x0 + x - 1, y0 + y - 1);
			SDL_RenderDrawLine(renderer, x0 - x, y0 - y, x0 + x - 1, y0 - y);
		}
		SDL_RenderDrawLine(renderer, x0 - y, y0 + x - 1, x0 + y - 1, y0 + x - 1);
		SDL_RenderDrawLine(renderer, x0 - y, y0 - x, x0 + y - 1, y0 - x);
	}
}

void drawEllipseFilled(SDL_Renderer* renderer, int x0, int y0, int width, int height, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	long long dx, dy, x, y;
	int x_offset, y_offset;
	double d1, d2;
	if (width == 1) {
		SDL_RenderDrawLine(renderer, x0, y0, x0, y0 + height - 1);
		return;
	}
	if (height == 1) {
		SDL_RenderDrawLine(renderer, x0, y0, x0 + width - 1, y0);
		return;
	}
	x0 = x0 + width / 2;
	y0 = y0 + height / 2;
	x_offset = (width + 1) % 2;
	y_offset = (height + 1) % 2;
	width = width / 2;
	height = height / 2;
	x = 0;
	y = height;
	d1 = (height * height) - (width * width * height) + (0.25 * width * width);
	dx = 2 * height * height * x;
	dy = 2 * width * width * y;
	while (dx < dy) {
		SDL_RenderDrawLine(renderer, x0 - (int)x, y0 - (int)y, x0 + (int)x - x_offset, y0 - (int)y);
		SDL_RenderDrawLine(renderer, x0 - (int)x, y0 + (int)y - y_offset, x0 + (int)x - x_offset, y0 + (int)y - y_offset);
		if (d1 < 0) {
			x++;
			dx = dx + (2 * height * height);
			d1 = d1 + dx + (height * height);
		}
		else {
			x++;
			y--;
			dx = dx + (2 * height * height);
			dy = dy - (2 * width * width);
			d1 = d1 + dx - dy + (height * height);
		}
	}
	d2 = (((double)height * height) * ((x + 0.5) * (x + 0.5))) + (((double)width * width) * ((y - 1) * (y - 1))) - ((double)width * width * height * height);
	while (y >= 0) {
		SDL_RenderDrawLine(renderer, x0 - (int)x, y0 - (int)y, x0 + (int)x - x_offset, y0 - (int)y);
		SDL_RenderDrawLine(renderer, x0 - (int)x, y0 + (int)y - y_offset, x0 + (int)x - x_offset, y0 + (int)y - y_offset);
		if (d2 > 0) {
			y--;
			dy = dy - (2 * width * width);
			d2 = d2 + (width * width) - dy;
		}
		else {
			y--;
			x++;
			dx = dx + (2 * height * height);
			dy = dy - (2 * width * width);
			d2 = d2 + dx - dy + (width * width);
		}
	}
}

void drawEllipseThickness(SDL_Renderer* renderer, int x0, int y0, int width, int height, int thickness, SDL_Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	long long dx, dy, dx_inner, dy_inner, x, y, x_inner, y_inner;
	int line, x_offset, y_offset;
	double d1, d2, d1_inner, d2_inner = 0;
	x0 = x0 + width / 2;
	y0 = y0 + height / 2;
	x_offset = (width + 1) % 2;
	y_offset = (height + 1) % 2;
	width = width / 2;
	height = height / 2;
	line = 1;
	x = 0;
	y = height;
	x_inner = 0;
	y_inner = height - thickness;
	d1 = (height * height) - (width * width * height) + (0.25 * width * width);
	d1_inner = ((height - thickness) * (height - thickness)) - ((width - thickness) * (width - thickness) * (height - thickness)) + (0.25 * (width - thickness) * (width - thickness));
	dx = 2 * height * height * x;
	dy = 2 * width * width * y;
	dx_inner = 2 * (height - thickness) * (height - thickness) * x_inner;
	dy_inner = 2 * (width - thickness) * (width - thickness) * y_inner;
	while (dx < dy) {
		if (line) {
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 - (int)y, x0 + (int)x - x_offset, y0 - (int)y);
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 + (int)y - y_offset, x0 + (int)x - x_offset, y0 + (int)y - y_offset);
		}
		else {
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 - (int)y, x0 - (int)x_inner, y0 - (int)y);
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 + (int)y - y_offset, x0 - (int)x_inner, y0 + (int)y - y_offset);
			SDL_RenderDrawLine(renderer, x0 + (int)x - x_offset, y0 - (int)y, x0 + (int)x_inner - x_offset, y0 - (int)y);
			SDL_RenderDrawLine(renderer, x0 + (int)x - x_offset, y0 + (int)y - y_offset, x0 + (int)x_inner - x_offset, y0 + (int)y - y_offset);
		}
		if (d1 < 0) {
			x++;
			dx = dx + (2 * height * height);
			d1 = d1 + dx + (height * height);
		}
		else {
			x++;
			y--;
			dx = dx + (2 * height * height);
			dy = dy - (2 * width * width);
			d1 = d1 + dx - dy + (height * height);
			if (line && y < height - thickness) {
				line = 0;
			}
			if (!line) {
				if (dx_inner < dy_inner) {
					while (d1_inner < 0) {
						x_inner++;
						dx_inner = dx_inner + (2 * (height - thickness) * (height - thickness));
						d1_inner = d1_inner + dx_inner + ((height - thickness) * (height - thickness));
					}
					x_inner++;
					y_inner--;
					dx_inner = dx_inner + (2 * (height - thickness) * (height - thickness));
					dy_inner = dy_inner - (2 * (width - thickness) * (width - thickness));
					d1_inner = d1_inner + dx_inner - dy_inner + ((height - thickness) * (height - thickness));
				}
			}
		}
	}
	d2 = (((double)height * height) * ((x + 0.5) * (x + 0.5))) + (((double)width * width) * ((y - 1) * (y - 1))) - ((double)width * width * height * height);
	while (y >= 0) {
		if (line) {
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 - (int)y, x0 + (int)x - x_offset, y0 - (int)y);
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 + (int)y - y_offset, x0 + (int)x - x_offset, y0 + (int)y - y_offset);
		}
		else {
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 - (int)y, x0 - (int)x_inner, y0 - (int)y);
			SDL_RenderDrawLine(renderer, x0 - (int)x, y0 + (int)y - y_offset, x0 - (int)x_inner, y0 + (int)y - y_offset);
			SDL_RenderDrawLine(renderer, x0 + (int)x - x_offset, y0 - (int)y, x0 + (int)x_inner - x_offset, y0 - (int)y);
			SDL_RenderDrawLine(renderer, x0 + (int)x - x_offset, y0 + (int)y - y_offset, x0 + (int)x_inner - x_offset, y0 + (int)y - y_offset);
		}
		if (d2 > 0) {
			y--;
			dy = dy - (2 * width * width);
			d2 = d2 + (width * width) - dy;
		}
		else {
			y--;
			x++;
			dx = dx + (2 * height * height);
			dy = dy - (2 * width * width);
			d2 = d2 + dx - dy + (width * width);
		}
		if (line && y < height - thickness) {
			line = 0;
		}
		if (!line) {
			if (dx_inner < dy_inner) {
				while (d1_inner < 0) {
					x_inner++;
					dx_inner = dx_inner + (2 * (height - thickness) * (height - thickness));
					d1_inner = d1_inner + dx_inner + ((height - thickness) * (height - thickness));
				}
				x_inner++;
				y_inner--;
				dx_inner = dx_inner + (2 * (height - thickness) * (height - thickness));
				dy_inner = dy_inner - (2 * (width - thickness) * (width - thickness));
				d1_inner = d1_inner + dx_inner - dy_inner + ((height - thickness) * (height - thickness));
			}
			else if (y_inner >= 0) {
				if (d2_inner == 0) {
					d2_inner = ((((double)height - thickness) * (height - thickness)) * ((x_inner + 0.5) * (x_inner + 0.5))) + ((((double)width - thickness) * (width - thickness)) * ((y_inner - 1) * (y_inner - 1))) - (((double)width - thickness) * (width - thickness) * (height - thickness) * (height - thickness));
				}
				if (d2_inner > 0) {
					y_inner--;
					dy_inner = dy_inner - (2 * (width - thickness) * (width - thickness));
					d2_inner = d2_inner + ((width - thickness) * (width - thickness)) - dy_inner;
				}
				else {
					y_inner--;
					x_inner++;
					dx_inner = dx_inner + (2 * (height - thickness) * (height - thickness));
					dy_inner = dy_inner - (2 * (width - thickness) * (width - thickness));
					d2_inner = d2_inner + dx_inner - dy_inner + ((width - thickness) * (width - thickness));
				}
			}
		}
	}
}

void drawRectRound(SDL_Renderer* renderer, SDL_Rect rect, int radius, SDL_Color color) {
	if (2 * radius > rect.h) {
		radius = rect.h / 2 - 1;
	}
	if (2 * radius > rect.w) {
		radius = rect.w / 2 - 1;
	}
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;
	
	int top_left_x = rect.x + radius;
	int top_right_x = rect.x + rect.w - radius;
	int bottom_left_x = rect.x + radius;
	int bottom_right_x = rect.x + rect.w - radius;
	int top_left_y = rect.y + radius;
	int top_right_y = rect.y + radius;
	int bottom_left_y = rect.y + rect.h - radius;
	int bottom_right_y = rect.y + rect.h - radius;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		if (f >= 0) {
			SDL_RenderDrawLine(renderer, bottom_left_x - x, bottom_left_y + y - 1, bottom_right_x + x - 1, bottom_right_y + y - 1);
			SDL_RenderDrawLine(renderer, top_left_x - x, top_left_y - y, top_right_x + x - 1, top_right_y - y);
		}
		SDL_RenderDrawLine(renderer, bottom_left_x - y, bottom_left_y + x - 1, bottom_right_x + y - 1, bottom_right_y + x - 1);
		SDL_RenderDrawLine(renderer, top_left_x - y, top_left_y - x, top_right_x + y - 1, top_right_y - x);
	}
	rect.y += radius;
	rect.h -= 2 * radius;
	SDL_RenderFillRect(renderer, &rect);
}