#include "gui.h"
#include "core_functions.h"
#include <unordered_map>

#define COLOR_TO_UINT(color) (0xff000000 | ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | (uint32_t)color.b)
#define UINT_TO_COLOR(color) { (uint8_t)((color & 0x00ff0000) >> 16), (uint8_t)((color & 0x0000ff00) >> 8), (uint8_t)(color & 0x000000ff), SDL_ALPHA_OPAQUE }

static SDL_Texture* s_BlendAddTexture = nullptr;
static std::unordered_map<uint8_t, TTF_Font*> s_FontMap;
static const char* s_FontPath = "vendor/SDL2_ttf/include/font/FreeSans.ttf";

static void s_DrawCheck(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) {
	SDL_Rect targetRect = { rect.x + 5, rect.y + 5, rect.w - 10, rect.w - 10 };
	drawLineThickness(renderer, plg::Vec2(targetRect.x, targetRect.y + targetRect.h / 2), plg::Vec2(targetRect.x + targetRect.w / 2, targetRect.y + targetRect.h), 6, color);
	drawLineThickness(renderer, plg::Vec2(targetRect.x + targetRect.w / 2, targetRect.y + targetRect.h), plg::Vec2(targetRect.x + targetRect.w, targetRect.y), 6, color);
}

static bool s_CollideWith(gui::Vector2D mousePos, SDL_Rect rect, gui::Vector2D offset) {
	gui::Vector2D position{rect.x + offset.x, rect.y + offset.y};
	return (mousePos.x > position.x) && (mousePos.y > position.y) && (mousePos.x < position.x + rect.w) && (mousePos.y < position.y + rect.h);
}

void gui::InitializeGUIStatics(SDL_Renderer* renderer) {
	s_BlendAddTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 256, 256);
	SDL_SetTextureBlendMode(s_BlendAddTexture, SDL_BLENDMODE_ADD);
}

gui::Label::Label(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color colorBG) : m_Rect(rect), m_Text(text), m_Size(size), m_ColorFG(colorFG), m_ColorBG(colorBG) {
	if (s_FontMap[m_Size] == nullptr) {
		s_FontMap[m_Size] = TTF_OpenFont(s_FontPath, m_Size);
	}
	SDL_Surface* textSurface = TTF_RenderText_LCD(s_FontMap[m_Size], m_Text.c_str(), m_ColorFG, colorBG);
	m_TextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_SetTextureBlendMode(m_TextTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(m_TextTexture, SDL_ALPHA_OPAQUE);
	m_Rect.w = textSurface->w;
	m_Rect.h = textSurface->h;
	SDL_FreeSurface(textSurface);
}

gui::Label::Label(const Label& other) 
	: m_Text(other.m_Text), m_Size(other.m_Size), m_ColorFG(other.m_ColorFG),
	m_ColorBG(other.m_ColorBG), m_Rect(other.m_Rect), m_TextTexture(other.m_TextTexture) { }

gui::Label::Label(Label&& other) noexcept
	: m_Text(std::move(other.m_Text)), m_Size(other.m_Size), m_ColorFG(other.m_ColorFG),
	m_ColorBG(other.m_ColorBG), m_Rect(other.m_Rect), m_TextTexture(other.m_TextTexture) {
	other.m_TextTexture = nullptr;
}

gui::Label& gui::Label::operator=(const Label& other) {
	if (this != &other) {
		m_Text = other.m_Text;
		m_Size = other.m_Size;
		m_ColorFG = other.m_ColorFG;
		m_ColorBG = other.m_ColorBG;
		m_Rect = other.m_Rect;
		m_TextTexture = other.m_TextTexture;
	}
	return *this;
}

gui::Label& gui::Label::operator=(Label&& other) noexcept {
	if (this != &other) {
		m_Text = other.m_Text;
		m_Size = other.m_Size;
		m_ColorFG = other.m_ColorFG;
		m_ColorBG = other.m_ColorBG;
		m_Rect = other.m_Rect;
		m_TextTexture = other.m_TextTexture;
		other.m_TextTexture = nullptr;
	}
	return *this;
}

void gui::Label::updatePosition(Vector2D offset) {
	m_Rect.x += offset.x;
	m_Rect.y += offset.y;
}

void gui::Label::Render(SDL_Renderer* renderer, Vector2D offset) {
	SDL_Rect targetRect = m_Rect;
	targetRect.x += offset.x;
	targetRect.y += offset.y;
	SDL_RenderCopy(renderer, m_TextTexture, NULL, &targetRect);
}

gui::Button::Button(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) : m_Label(renderer, rect, text, size, labelColor, colorFG), m_Rect(rect), m_ColorFG(colorFG), m_ColorBG(colorBG) {
	m_Rect.w = m_Label.GetRect().w + 8;
	m_Rect.h = m_Label.GetRect().h + 8;
	m_Label.updatePosition({ 4, 4 });
}

gui::Button::Button(const Button& other) 
	: m_Label(other.m_Label), m_Rect(other.m_Rect), m_ColorFG(other.m_ColorFG), m_ColorBG(other.m_ColorBG), m_State(other.m_State) { }

gui::Button::Button(Button&& other) noexcept
	: m_Label(std::move(other.m_Label)), m_Rect(other.m_Rect), m_ColorFG(other.m_ColorFG), m_ColorBG(other.m_ColorBG), m_State(other.m_State) { }

gui::Button& gui::Button::operator=(const Button& other) {
	if (this != &other) {
		m_Label = other.m_Label;
		m_Rect = other.m_Rect;
		m_ColorFG = other.m_ColorFG;
		m_ColorBG = other.m_ColorBG;
		m_State = other.m_State;
	}
	return *this;
}

gui::Button& gui::Button::operator=(Button&& other) noexcept {
	if (this != &other) {
		m_Label = std::move(other.m_Label);
		m_Rect = other.m_Rect;
		m_ColorFG = other.m_ColorFG;
		m_ColorBG = other.m_ColorBG;
		m_State = other.m_State;
	}
	return *this;
}

void gui::Button::Render(SDL_Renderer* renderer, Vector2D offset) {
	SDL_Rect targetRect = m_Rect;
	targetRect.x += offset.x;
	targetRect.y += offset.y;
	drawRectRound(renderer, targetRect, 8, m_ColorFG);
	m_Label.Render(renderer, offset);
	if (s_CollideWith(GUIEvent::GetMouseMotionPos(), m_Rect, offset)) {
		SDL_Texture* mainTarget = SDL_GetRenderTarget(renderer);
		SDL_SetRenderTarget(renderer, s_BlendAddTexture);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		drawRectRound(renderer, { 0, 0, targetRect.w, targetRect.h }, 8, { 35, 35, 35, SDL_ALPHA_OPAQUE });
		SDL_SetRenderTarget(renderer, mainTarget);
		targetRect = { targetRect.x, targetRect.y, 256, 256 };
		SDL_RenderCopy(renderer, s_BlendAddTexture, NULL, &targetRect);
	}
}

gui::Slider::Slider(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, float maxVal, uint8_t orientation, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) : m_ColorFG(colorFG), m_Rect(rect), m_Label(renderer, rect, text, size, labelColor, colorBG), m_MaxVal(maxVal), m_Orientation(orientation) {
	m_Rect.w = (orientation == GUI_HORIZONTAL) ? 100 : size;
	m_Rect.h = (orientation == GUI_VERTICAL) ? 100 : size;
	m_Rect.x = m_Rect.x + (orientation == GUI_VERTICAL) * (m_Label.GetRect().w / 2 - m_Rect.w / 2);
	m_Rect.y = m_Rect.y + size + 4;
	m_Label.updatePosition({ (orientation == GUI_HORIZONTAL) * (50 - m_Label.GetRect().w / 2), 0 });
}

gui::Slider::Slider(const Slider& other) 
	: m_Label(other.m_Label), m_ColorFG(other.m_ColorFG), m_Rect(other.m_Rect), m_Value(other.m_Value),
	m_MaxVal(other.m_MaxVal), m_Orientation(other.m_Orientation) { }

gui::Slider::Slider(Slider&& other) noexcept 
	: m_Label(std::move(other.m_Label)), m_ColorFG(other.m_ColorFG), m_Rect(other.m_Rect), m_Value(other.m_Value),
	m_MaxVal(other.m_MaxVal), m_Orientation(other.m_Orientation) { }

gui::Slider& gui::Slider::operator=(const Slider& other) {
	if (this != &other) {
		m_Label = other.m_Label;
		m_ColorFG = other.m_ColorFG;
		m_Rect = other.m_Rect;
		m_Value = other.m_Value;
		m_MaxVal = other.m_MaxVal;
		m_Orientation = other.m_Orientation;
	}
	return *this;
}

gui::Slider& gui::Slider::operator=(Slider&& other) noexcept {
	if (this != &other) {
		m_Label = std::move(other.m_Label);
		m_ColorFG = other.m_ColorFG;
		m_Rect = other.m_Rect;
		m_Value = other.m_Value;
		m_MaxVal = other.m_MaxVal;
		m_Orientation = other.m_Orientation;
	}
	return *this;
}

void gui::Slider::SetValue(Vector2D mousePos, Vector2D offset) {
	if (m_Orientation == GUI_HORIZONTAL) {
		int val_X = mousePos.x - offset.x - m_Rect.x;
		val_X = (val_X > 0 && val_X < m_Rect.w) * val_X + m_Rect.w * (val_X >= m_Rect.w);
		m_Value = (float)val_X * m_MaxVal / (float)m_Rect.w;
		//std::cout << "Horizontal: " << m_Value << "\n";
	}
	else if (m_Orientation == GUI_VERTICAL) {
		int val_Y = mousePos.y - offset.y - m_Rect.y;
		val_Y = (val_Y > 0 && val_Y < m_Rect.h) * val_Y + m_Rect.h * (val_Y >= m_Rect.h);
		m_Value = (float)val_Y * m_MaxVal / (float)m_Rect.h;
		//std::cout << "Vertical: " << m_Value << "\n";
	}
}

void gui::Slider::Render(SDL_Renderer* renderer, Vector2D offset) {
	SDL_Rect targetRect = m_Rect;
	targetRect.x += offset.x;
	targetRect.y += offset.y;
	drawRectRound(renderer, targetRect, 3, m_Label.GetColorFG());
	m_Label.Render(renderer, offset);
	targetRect.w -= (int)(targetRect.w * (1.0f - m_Value / m_MaxVal) * (m_Orientation == GUI_HORIZONTAL));
	targetRect.h -= (int)(targetRect.h * (1.0f - m_Value / m_MaxVal) * (m_Orientation == GUI_VERTICAL));
	drawRectRound(renderer, targetRect, 3, m_ColorFG);
}

gui::RadioButton::RadioButton(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) : m_ColorFG(colorFG) {
	for (auto label: labels) {
		m_Labels.Append(Label(renderer, { rect.x, rect.y + size + 4 }, label, size, labelColor, colorBG));
	}
	for (auto iter = m_Labels.Begin(); iter < iter.end_ptr; iter++) {
		SDL_Rect rect = iter->GetRect();
		m_Rects.Append({ rect.x, rect.y + (size + 8) * ((int)m_Labels.GetSize() - (int)(iter.end_ptr - iter.operator->())), rect.w + size + 8, rect.h });
		iter->updatePosition({ size + 8, (size + 8) * ((int)m_Labels.GetSize() - (int)(iter.end_ptr - iter.operator->())) });
	}
}

gui::RadioButton::RadioButton(const RadioButton& other)
	: m_ColorFG(other.m_ColorFG), m_SetButton(other.m_SetButton), m_Labels(other.m_Labels), m_Rects(other.m_Rects) { }

gui::RadioButton::RadioButton(RadioButton&& other) noexcept
	: m_ColorFG(other.m_ColorFG), m_SetButton(other.m_SetButton), m_Labels(std::move(other.m_Labels)), m_Rects(std::move(other.m_Rects)) { }

gui::RadioButton& gui::RadioButton::operator=(const RadioButton& other) {
	if (this != &other) {
		m_ColorFG = other.m_ColorFG;
		m_SetButton = other.m_SetButton;
		m_Labels = other.m_Labels;
		m_Rects = other.m_Rects;
	}
	return *this;
}

gui::RadioButton& gui::RadioButton::operator=(RadioButton&& other) noexcept {
	if (this != &other) {
		m_ColorFG = other.m_ColorFG;
		m_SetButton = other.m_SetButton;
		m_Labels = std::move(other.m_Labels);
		m_Rects = std::move(other.m_Rects);
	}
	return *this;
}

void gui::RadioButton::Render(SDL_Renderer* renderer, Vector2D offset) {
	auto iter_Label = m_Labels.Begin();
	auto iter_Rect = m_Rects.Begin();
	while (iter_Label < iter_Label.end_ptr) {
		Vector2D center{ iter_Rect->x + iter_Rect->h / 2 + offset.x, iter_Rect->y + iter_Rect->h / 2 + offset.y };
		iter_Label->Render(renderer, offset);
		SDL_Color targetColor = m_ColorFG;
		if (s_CollideWith(GUIEvent::GetMouseMotionPos(), *iter_Rect, offset)) {
			targetColor.r += 35 * (255 - targetColor.r > 34);
			targetColor.g += 35 * (255 - targetColor.g > 34);
			targetColor.b += 35 * (255 - targetColor.b > 34);
		}
		drawCircleFilled(renderer, center.x, center.y, iter_Rect->h / 3, targetColor);
		drawCircleFilled(renderer, center.x, center.y, iter_Rect->h / 3 - 2, iter_Label->GetColorBG());
		if (iter_Label.operator->() - iter_Label.GetBegin() == m_SetButton) {
			drawCircleFilled(renderer, center.x, center.y, iter_Rect->h / 3 - 4, targetColor);
		}
		iter_Label++;
		iter_Rect++;
	}
}

gui::CheckButton::CheckButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) : m_ColorFG(colorFG), m_Rect(rect), m_Label(renderer, rect, text, size, labelColor, colorBG) {
	m_Rect.w = m_Label.GetRect().w + size + 8;
	m_Rect.h = m_Label.GetRect().h;
	m_Label.updatePosition({ size + 8, 0 });
}

gui::CheckButton::CheckButton(const CheckButton& other)
	: m_ColorFG(other.m_ColorFG), m_Rect(other.m_Rect), m_Label(other.m_Label), m_State(other.m_State) { }

gui::CheckButton::CheckButton(CheckButton&& other) noexcept
	: m_ColorFG(other.m_ColorFG), m_Rect(other.m_Rect), m_Label(std::move(other.m_Label)), m_State(other.m_State) { }

gui::CheckButton& gui::CheckButton::operator=(const CheckButton& other) {
	if (this != &other) {
		m_ColorFG = other.m_ColorFG;
		m_Rect = other.m_Rect;
		m_Label = other.m_Label;
		m_State = other.m_State;
	}
	return *this;
}
gui::CheckButton& gui::CheckButton::operator=(CheckButton&& other) noexcept {
	if (this != &other) {
		m_ColorFG = other.m_ColorFG;
		m_Rect = other.m_Rect;
		m_Label = std::move(other.m_Label);
		m_State = other.m_State;
	}
	return *this;
}

void gui::CheckButton::Render(SDL_Renderer* renderer, Vector2D offset) {
	SDL_Rect targetRect = m_Rect;
	targetRect.x += offset.x;
	targetRect.y += offset.y;
	SDL_Color targetColor = m_ColorFG;
	if (s_CollideWith(GUIEvent::GetMouseMotionPos(), m_Rect, offset)) {
		targetColor.r += 35 * (255 - targetColor.r > 34);
		targetColor.g += 35 * (255 - targetColor.g > 34);
		targetColor.b += 35 * (255 - targetColor.b > 34);
	}
	if (m_State) {
		drawRectRound(renderer, { targetRect.x, targetRect.y, targetRect.h, targetRect.h }, 4, targetColor);
		s_DrawCheck(renderer, { targetRect.x, targetRect.y, targetRect.h, targetRect.h }, m_Label.GetColorBG());
	}
	else {
		drawRectRound(renderer, { targetRect.x, targetRect.y, targetRect.h, targetRect.h }, 4, targetColor);
		drawRectRound(renderer, { targetRect.x + 2, targetRect.y + 2, targetRect.h - 4, targetRect.h - 4 }, 4, m_Label.GetColorBG());
	}
	m_Label.Render(renderer, offset);
}

void gui::Layer::AddButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) {
	m_Buttons.EmplaceBack(renderer, rect, text, size, colorFG, labelColor, colorBG);
}

void gui::Layer::AddSlider(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, float maxVal, uint8_t orientation, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) {
	m_Sliders.EmplaceBack(renderer, rect, text, size, maxVal, orientation, colorFG, labelColor, colorBG);
}

void gui::Layer::AddRadioButton(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) {
	m_RadioButtons.EmplaceBack(renderer, rect, labels, size, colorFG, labelColor, colorBG);
}

void gui::Layer::AddCheckButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) {
	m_CheckButtons.EmplaceBack(renderer, rect, text, size, colorFG, labelColor, colorBG);
}

void gui::Layer::Render(SDL_Renderer* renderer, Vector2D offset) {
	for (auto it_Button = this->GetButtonIterator(); it_Button < it_Button.end_ptr; it_Button++) {
		it_Button->Render(renderer, offset);
	}
	for (auto it_Slider = this->GetSliderIterator(); it_Slider < it_Slider.end_ptr; it_Slider++) {
		it_Slider->Render(renderer, offset);
	}
	for (auto it_ChekcButton = this->GetCheckButtonIterator(); it_ChekcButton < it_ChekcButton.end_ptr; it_ChekcButton++) {
		it_ChekcButton->Render(renderer, offset);
	}
	for (auto it_RadioButton = this->GetRadioButtonIterator(); it_RadioButton < it_RadioButton.end_ptr; it_RadioButton++) {
		it_RadioButton->Render(renderer, offset);
	}
}

void gui::HandleGUIEvents(GUIEvent* guiEvent, Layer* layer) {
	if (*guiEvent->getMousePressed(SDL_BUTTON_LEFT)) {
		for (auto it_Button = layer->GetButtonIterator(); it_Button < it_Button.end_ptr; it_Button++) {
			if (s_CollideWith(*guiEvent->GetMousePos(), it_Button->GetRect(), layer->GetPosition())) {
				it_Button->SetState();
			}
		}
		for (auto it_ChekcButton = layer->GetCheckButtonIterator(); it_ChekcButton < it_ChekcButton.end_ptr; it_ChekcButton++) {
			if (s_CollideWith(*guiEvent->GetMousePos(), it_ChekcButton->GetRect(), layer->GetPosition())) {
				it_ChekcButton->SetState();
			}
		}
		for (auto it_RadioButton = layer->GetRadioButtonIterator(); it_RadioButton < it_RadioButton.end_ptr; it_RadioButton++) {
			for (auto it_Rect = it_RadioButton->GetRectIterator(); it_Rect < it_Rect.end_ptr; it_Rect++) {
				if (s_CollideWith(*guiEvent->GetMousePos(), *it_Rect, layer->GetPosition())) {
					it_RadioButton->SetState((uint8_t)(it_Rect.operator->() - it_Rect.GetBegin()));
				}
			}
		}
		*guiEvent->getMousePressed(SDL_BUTTON_LEFT) = false;
	}
	if (guiEvent->GetMouseState(SDL_BUTTON_LEFT)) {
		for (auto it_Slider = layer->GetSliderIterator(); it_Slider < it_Slider.end_ptr; it_Slider++) {
			if (s_CollideWith(*guiEvent->GetMousePos(), it_Slider->GetRect(), layer->GetPosition())) {
				it_Slider->SetValue(guiEvent->GetMouseMotionPos(), layer->GetPosition());
			}
		}
	}
}

void gui::RetriveGUIEvents(GUIEvent* guiEvent) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			*guiEvent->GetQuitState() = true;
			break;
		case SDL_KEYDOWN:
			guiEvent->SetKeyState(event.key.keysym.scancode);
			break;
		case SDL_KEYUP:
			guiEvent->SetKeyState(event.key.keysym.scancode);
			break;
		case SDL_MOUSEBUTTONDOWN:
			guiEvent->SetMouseState(event.button.button);
			SDL_GetMouseState(&guiEvent->GetMousePos()->x, &guiEvent->GetMousePos()->y);
			*guiEvent->getMousePressed(event.button.button) = true;
			break;
		case SDL_MOUSEBUTTONUP:
			guiEvent->SetMouseState(event.button.button);
			break;
		default:
			break;
		}
	}
}