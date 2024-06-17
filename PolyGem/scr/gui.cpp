#include "gui.h"
#include "core_scene.h"
#include "core_functions.h"
#include <unordered_map>
#include <unordered_set>

#define D_PI 0.0174532925199432957692369076849
#define COLOR_TO_UINT(color) (0xff000000 | ((uint32_t)color.r << 16) | ((uint32_t)color.g << 8) | (uint32_t)color.b)
#define UINT_TO_COLOR(color) { (uint8_t)((color & 0x00ff0000) >> 16), (uint8_t)((color & 0x0000ff00) >> 8), (uint8_t)(color & 0x000000ff), SDL_ALPHA_OPAQUE }

static SDL_Texture* s_BlendAddTexture = nullptr;
static SDL_Texture* s_GlobalLayerTexture = nullptr;
static std::unordered_map<uint8_t, TTF_Font*> s_FontMap;
static const char* s_FontPath = "vendor/SDL2_ttf/include/font/FreeSans.ttf";

static void s_DrawCheck(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) {
	SDL_Rect targetRect = { rect.x + 4, rect.y + 4, rect.w - 8, rect.w - 8 };
	drawLineThickness(renderer, plg::Vec2(targetRect.x, targetRect.y + targetRect.h / 2), plg::Vec2(targetRect.x + targetRect.w / 2, targetRect.y + targetRect.h), 6, color);
	drawLineThickness(renderer, plg::Vec2(targetRect.x + targetRect.w / 2, targetRect.y + targetRect.h), plg::Vec2(targetRect.x + targetRect.w, targetRect.y), 6, color);
}

static bool s_CollideWith(gui::Vector2D mousePos, SDL_Rect rect, gui::Vector2D offset = { 0, 0 }) {
	gui::Vector2D position{rect.x + offset.x, rect.y + offset.y};
	return (mousePos.x > position.x) && (mousePos.y > position.y) && (mousePos.x < position.x + rect.w) && (mousePos.y < position.y + rect.h);
}

void gui::InitializeGUIStatics(SDL_Renderer* renderer) {
	s_BlendAddTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 256, 256);
	s_GlobalLayerTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 256, 256);
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
	SDL_QueryTexture(m_TextTexture, NULL, NULL, &m_Rect.w, &m_Rect.h);
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

void gui::Label::UpdatePosition(Vector2D offset) {
	m_Rect.x += offset.x;
	m_Rect.y += offset.y;
}

void gui::Label::Render(SDL_Renderer* renderer, Vector2D offset) {
	SDL_Rect offsetRect = { m_Rect.x + offset.x, m_Rect.y + offset.y, m_Rect.w, m_Rect.h };
	SDL_RenderCopy(renderer, m_TextTexture, NULL, &offsetRect);
}

gui::LabelNode::LabelNode(const LabelNode& other)
	: Label(other), m_Parent(other.m_Parent), m_State(other.m_State), m_ChildState(other.m_ChildState), m_ChildCount(other.m_ChildCount), m_ChildSetCounter(other.m_ChildSetCounter) { }

gui::LabelNode::LabelNode(LabelNode&& other) noexcept 
	: Label(std::move(other)), m_Parent(other.m_Parent), m_State(other.m_State), m_ChildState(other.m_ChildState), m_ChildCount(other.m_ChildCount), m_ChildSetCounter(other.m_ChildSetCounter) { }

gui::LabelNode& gui::LabelNode::operator=(const LabelNode& other) {
	if (this != &other) {
		Label::operator=(other);
		m_Parent = other.m_Parent;
		m_State = other.m_State;
		m_ChildState = other.m_ChildState;
		m_ChildCount = other.m_ChildCount;
		m_ChildSetCounter = other.m_ChildSetCounter;
	}
	return *this;
}

gui::LabelNode& gui::LabelNode::operator=(LabelNode&& other) noexcept {
	if (this != &other) {
		Label::operator=(std::move(other));
		m_Parent = other.m_Parent;
		m_State = other.m_State;
		m_ChildState = other.m_ChildState;
		m_ChildCount = other.m_ChildCount;
		m_ChildSetCounter = other.m_ChildSetCounter;
	}
	return *this;
}

gui::Button::Button(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor) : m_Label(renderer, rect, text, size, labelColor, colorFG), m_Rect(rect), m_ColorFG(colorFG) {
	m_Rect.w = m_Label.GetRect().w + 8;
	m_Rect.h = m_Label.GetRect().h + 8;
	m_Label.UpdatePosition({ 4, 4 });
}

gui::Button::Button(const Button& other) 
	: m_Label(other.m_Label), m_Rect(other.m_Rect), m_ColorFG(other.m_ColorFG), m_State(other.m_State) { }

gui::Button::Button(Button&& other) noexcept
	: m_Label(std::move(other.m_Label)), m_Rect(other.m_Rect), m_ColorFG(other.m_ColorFG), m_State(other.m_State) { }

gui::Button& gui::Button::operator=(const Button& other) {
	if (this != &other) {
		m_Label = other.m_Label;
		m_Rect = other.m_Rect;
		m_ColorFG = other.m_ColorFG;
		m_State = other.m_State;
	}
	return *this;
}

gui::Button& gui::Button::operator=(Button&& other) noexcept {
	if (this != &other) {
		m_Label = std::move(other.m_Label);
		m_Rect = other.m_Rect;
		m_ColorFG = other.m_ColorFG;
		m_State = other.m_State;
	}
	return *this;
}

void gui::Button::Render(SDL_Renderer* renderer) {
	SDL_Rect targetRect = m_Rect;
	drawRectRound(renderer, targetRect, 8, m_ColorFG);
	m_Label.Render(renderer);
	if (m_IsHovered) {
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
	m_Rect.w = (orientation == GUI_HORIZONTAL) ? 100 : size / 2;
	m_Rect.h = (orientation == GUI_VERTICAL) ? 100 : size / 2;
	m_Rect.x = m_Rect.x + (orientation == GUI_VERTICAL) * (m_Label.GetRect().w / 2 - m_Rect.w / 2);
	m_Rect.y = m_Rect.y + m_Label.GetRect().h;
	m_Label.UpdatePosition({ (orientation == GUI_HORIZONTAL) * (50 - m_Label.GetRect().w / 2), 0 });
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
	m_State = true;
	if (m_Orientation == GUI_HORIZONTAL) {
		int val_X = mousePos.x - offset.x - m_Rect.x;
		val_X = (val_X > 0 && val_X < m_Rect.w) * val_X + m_Rect.w * (val_X >= m_Rect.w);
		m_Value = (float)val_X * m_MaxVal / (float)m_Rect.w;
	}
	else if (m_Orientation == GUI_VERTICAL) {
		int val_Y = mousePos.y - offset.y - m_Rect.y;
		val_Y = (val_Y > 0 && val_Y < m_Rect.h) * val_Y + m_Rect.h * (val_Y >= m_Rect.h);
		m_Value = (float)val_Y * m_MaxVal / (float)m_Rect.h;
	}
}

void gui::Slider::Render(SDL_Renderer* renderer) {
	SDL_Rect targetRect = m_Rect;
	drawRectRound(renderer, targetRect, 3, m_Label.GetColorFG());
	m_Label.Render(renderer);
	targetRect.w -= (int)(targetRect.w * (1.0f - m_Value / m_MaxVal) * (m_Orientation == GUI_HORIZONTAL));
	targetRect.h -= (int)(targetRect.h * (1.0f - m_Value / m_MaxVal) * (m_Orientation == GUI_VERTICAL));
	drawRectRound(renderer, targetRect, 3, m_ColorFG);
	if (m_State) {
		if (m_Orientation == GUI_HORIZONTAL) {
			targetRect = { targetRect.x + targetRect.w - targetRect.h / 2, targetRect.y - targetRect.h / 2, targetRect.h, targetRect.h * 2 };
		}
		else {
			targetRect = { targetRect.x - targetRect.w / 2, targetRect.y + targetRect.h - targetRect.w / 2, targetRect.w * 2, targetRect.w };
		}
		drawRectRound(renderer, targetRect, 3, m_ColorFG);
	}
	m_State = false;
}

gui::RadioButton::RadioButton(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) : m_ColorFG(colorFG) {
	for (auto label: labels) {
		m_Labels.Append(Label(renderer, { rect.x, rect.y}, label, size, labelColor, colorBG));
	}
	for (auto iter = m_Labels.Begin(); iter < iter.end_ptr; iter++) {
		SDL_Rect rect = iter->GetRect();
		m_Rects.Append({ rect.x, rect.y + iter->GetRect().h * (int)iter.GetIndex(), rect.w + rect.h, rect.h});
		iter->UpdatePosition({ rect.h, iter->GetRect().h * (int)iter.GetIndex() });
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

void gui::RadioButton::Render(SDL_Renderer* renderer) {
	auto iter_Label = m_Labels.Begin();
	auto iter_Rect = m_Rects.Begin();
	while (iter_Label < iter_Label.end_ptr) {
		Vector2D center{ iter_Rect->x + iter_Rect->h / 2, iter_Rect->y + iter_Rect->h / 2 };
		iter_Label->Render(renderer);
		SDL_Color targetColor = m_ColorFG;
		if (m_IsHovered == iter_Label.GetIndex()) {
			targetColor.r += 35 * (255 - targetColor.r > 34);
			targetColor.g += 35 * (255 - targetColor.g > 34);
			targetColor.b += 35 * (255 - targetColor.b > 34);
		}
		drawCircleFilled(renderer, center.x, center.y, iter_Rect->h / 3, targetColor);
		drawCircleFilled(renderer, center.x, center.y, iter_Rect->h / 3 - 1, iter_Label->GetColorBG());
		if (iter_Label.GetIndex() == m_SetButton) {
			drawCircleFilled(renderer, center.x, center.y, iter_Rect->h / 3 - 3, targetColor);
		}
		iter_Label++;
		iter_Rect++;
	}
}

gui::CheckButton::CheckButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) : m_ColorFG(colorFG), m_Rect(rect), m_Label(renderer, rect, text, size, labelColor, colorBG) {
	m_Rect.w = m_Label.GetRect().w + m_Label.GetRect().h;
	m_Rect.h = m_Label.GetRect().h;
	m_Label.UpdatePosition({ m_Label.GetRect().h, 0 });
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

void gui::CheckButton::Render(SDL_Renderer* renderer) {
	m_Label.Render(renderer);
	SDL_Rect targetRect = { m_Rect.x + 2, m_Rect.y + 2, m_Rect.h - 4, m_Rect.h - 4 };
	SDL_Color targetColor = m_ColorFG;
	if (m_IsHovered) {
		targetColor.r += 35 * (255 - targetColor.r > 34);
		targetColor.g += 35 * (255 - targetColor.g > 34);
		targetColor.b += 35 * (255 - targetColor.b > 34);
	}
	if (m_State) {
		drawRectRound(renderer, targetRect, 4, targetColor);
		s_DrawCheck(renderer, targetRect, m_Label.GetColorBG());
	}
	else {
		drawRectRound(renderer, targetRect, 4, targetColor);
		drawRectRound(renderer, { targetRect.x + 1, targetRect.y + 1, targetRect.h - 2, targetRect.h - 2 }, 4, m_Label.GetColorBG());
	}
}

gui::TreeView::TreeView(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, std::initializer_list<int> layers, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) {
	uint8_t currParentIndex = LabelNode::NO_PARENT;
	int prevDepth = 0, depthDiff = 0, depthIterCount = 0;
	
	auto label_iter = labels.begin();
	auto layer_iter = layers.begin();
	LabelNode* currParent;
	for (; label_iter != labels.end();) {
		if (prevDepth != *layer_iter) {
			depthDiff = std::abs(*layer_iter - prevDepth);
			if (*layer_iter < prevDepth) {
				while (depthIterCount < depthDiff && currParentIndex != LabelNode::NO_PARENT) {
					currParentIndex = m_LabelNodes[currParentIndex].GetParent();
					depthIterCount++;
				}
				depthIterCount = 0;
			}
			else if (*layer_iter > prevDepth) {
				currParentIndex = m_LabelNodes.GetSize() - 1;
			}
		}
		prevDepth = *layer_iter;
		m_LabelNodes.Append(LabelNode(currParentIndex, renderer, rect, *label_iter, size, labelColor, colorBG));
		LabelNode* curr = &m_LabelNodes[m_LabelNodes.GetSize() - 1];
		if (currParentIndex != LabelNode::NO_PARENT) {
			currParent = &m_LabelNodes[currParentIndex];
			currParent->IncChild();
			currParent->SetChildState(NodeState::HAS_CHILD_EXPANDED);
			curr->UpdatePosition({ (*layer_iter + 1) * curr->GetRect().h, 0 });
		}
		else {
			curr->UpdatePosition({ (*layer_iter + 1) * curr->GetRect().h, 0 });
		}
		label_iter++;
		layer_iter++;
	}
}

gui::TreeView::TreeView(const TreeView& other) : m_LabelNodes(other.m_LabelNodes) { }

gui::TreeView::TreeView(TreeView&& other) noexcept : m_LabelNodes(std::move(other.m_LabelNodes)) { }

gui::TreeView& gui::TreeView::operator=(const TreeView& other) {
	if (this != &other){
		m_LabelNodes = other.m_LabelNodes;
	}
	return *this;
}

gui::TreeView& gui::TreeView::operator=(TreeView&& other) noexcept {
	if (this != &other){
		m_LabelNodes = std::move(other.m_LabelNodes);
	}
	return *this;
}

void gui::TreeView::ToggleNode(size_t index) {
	LabelNode* node = &m_LabelNodes[index];
	if (node->GetChildState() == NodeState::HAS_CHILD_EXPANDED) {
		node->SetChildState(NodeState::CHILD_COLLAPSING);
		node->SetChildSetCounter();
	}
	else if (node->GetChildState() == NodeState::HAS_CHILD_COLLAPSED) {
		node->SetChildState(NodeState::CHILD_EXPANDING);
		node->SetChildSetCounter();
	}
	UpdateStates();
}

void gui::TreeView::UpdateStates() {
	for (auto it_Node = m_LabelNodes.Begin(); it_Node < it_Node.end_ptr; it_Node++) {
		if (it_Node->GetParent() != LabelNode::NO_PARENT) {
			LabelNode* parent = &m_LabelNodes[it_Node->GetParent()];
			if (parent->GetChildState() == NodeState::CHILD_EXPANDING) {
				it_Node->SetState(NodeState::IS_EXPANDED);
				parent->DecSetChild();
				if (it_Node->GetChildState() != NodeState::NO_CHILD) {
					it_Node->SetChildState(NodeState::CHILD_EXPANDING);
					it_Node->SetChildSetCounter();
				}
				if (parent->GetChildSetCounter() == 0) {
					parent->SetChildState(NodeState::HAS_CHILD_EXPANDED);
				}
			}
			else if (parent->GetChildState() == NodeState::CHILD_COLLAPSING) {
				it_Node->SetState(NodeState::IS_COLLAPSED);
				parent->DecSetChild();
				if (it_Node->GetChildState() != NodeState::NO_CHILD) {
					it_Node->SetChildState(NodeState::CHILD_COLLAPSING);
					it_Node->SetChildSetCounter();
				}
				if (parent->GetChildSetCounter() == 0) {
					parent->SetChildState(NodeState::HAS_CHILD_COLLAPSED);
				}
			}
		}
	}
}

void gui::TreeView::Render(SDL_Renderer* renderer) {
	int verticalOffset = 0;
	auto it_Node = m_LabelNodes.Begin();
	int h = it_Node->GetRect().h;
	for (; it_Node < it_Node.end_ptr; it_Node++) {
		if (it_Node->GetState() == NodeState::IS_EXPANDED) {
			SDL_Rect nodeRect = it_Node->GetRect();
			it_Node->Render(renderer, { (int)(it_Node->GetSize() * 1.5), verticalOffset});
			SDL_Color targetColor = it_Node->GetColorFG();
			if (it_Node.GetIndex() == m_IsHovered) {
				targetColor.r += 35 * (255 - targetColor.r > 34);
				targetColor.g += 35 * (255 - targetColor.g > 34);
				targetColor.b += 35 * (255 - targetColor.b > 34);
			}
			if (it_Node->GetChildState() == NodeState::HAS_CHILD_COLLAPSED) {
				SDL_Point points[4];
				points[0] = { nodeRect.x, nodeRect.y + verticalOffset };
				points[1] = { nodeRect.x + (int)(h * 0.7072f) , nodeRect.y + verticalOffset + (int)(h * 0.5) };
				points[2] = { nodeRect.x, nodeRect.y + verticalOffset + h };
				points[3] = points[0];
				SDL_SetRenderDrawColor(renderer, targetColor.r, targetColor.g, targetColor.b, targetColor.a);
				SDL_RenderDrawLines(renderer, points, 4);
			}
			else if (it_Node->GetChildState() == NodeState::HAS_CHILD_EXPANDED) {
				SDL_Vertex vertices[3];
				vertices[0].position = { (float)(nodeRect.x), (float)(nodeRect.y + h + verticalOffset) };
				vertices[0].color = targetColor;
				vertices[1].position = { (float)(nodeRect.x + 0.7072f * h), (float)(nodeRect.y + h + verticalOffset) };
				vertices[1].color = targetColor;
				vertices[2].position = { (float)(nodeRect.x + 0.7072f * h), (float)(nodeRect.y + 0.2928 * h + verticalOffset) };
				vertices[2].color = targetColor;
				SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);
			}
			verticalOffset += h;
		}
	}
}

void gui::Layer::AddButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor) {
	m_Buttons.EmplaceBack(renderer, rect, text, size, colorFG, labelColor);
}

void gui::Layer::AddSlider(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, float maxVal, uint8_t orientation, SDL_Color colorFG, SDL_Color labelColor) {
	m_Sliders.EmplaceBack(renderer, rect, text, size, maxVal, orientation, colorFG, labelColor, m_ColorBG);
}

void gui::Layer::AddRadioButton(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, uint8_t size, SDL_Color colorFG, SDL_Color labelColor) {
	m_RadioButtons.EmplaceBack(renderer, rect, labels, size, colorFG, labelColor, m_ColorBG);
}

void gui::Layer::AddCheckButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor) {
	m_CheckButtons.EmplaceBack(renderer, rect, text, size, colorFG, labelColor, m_ColorBG);
}

void gui::Layer::AddTreeView(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, std::initializer_list<int> layers, uint8_t size, SDL_Color colorFG, SDL_Color labelColor, SDL_Color colorBG) {
	m_TreeViews.EmplaceBack(renderer, rect, labels, layers, size, colorFG, labelColor, colorBG);
}

void gui::Layer::Render(SDL_Renderer* renderer) {
	SDL_Texture* mainTarget = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, m_LayerTexture);
	SDL_SetRenderDrawColor(renderer, m_ColorBG.r, m_ColorBG.g, m_ColorBG.b, m_ColorBG.a);
	SDL_RenderClear(renderer);
	for (auto it_Button = GetButtonIterator(); it_Button < it_Button.end_ptr; it_Button++) {
		it_Button->GetHovered() = s_CollideWith(GUIEvent::GetMouseCurrentPos(), it_Button->GetRect(), { m_Rect.x, m_Rect.y });
		it_Button->Render(renderer);
	}
	for (auto it_Slider = GetSliderIterator(); it_Slider < it_Slider.end_ptr; it_Slider++) {
		it_Slider->Render(renderer);
	}
	for (auto it_CheckButton = GetCheckButtonIterator(); it_CheckButton < it_CheckButton.end_ptr; it_CheckButton++) {
		it_CheckButton->GetHovered() = s_CollideWith(GUIEvent::GetMouseCurrentPos(), it_CheckButton->GetRect(), { m_Rect.x, m_Rect.y });
		it_CheckButton->Render(renderer);
	}
	for (auto it_RadioButton = GetRadioButtonIterator(); it_RadioButton < it_RadioButton.end_ptr; it_RadioButton++) {
		it_RadioButton->GetHovered() = -1;
		for (auto it_RectRB = it_RadioButton->GetRectIterator(); it_RectRB < it_RectRB.end_ptr; it_RectRB++) {
			if (s_CollideWith(GUIEvent::GetMouseCurrentPos(), *it_RectRB, { m_Rect.x, m_Rect.y })) {
				it_RadioButton->GetHovered() = it_RectRB.GetIndex();
				break;
			}
		}
		it_RadioButton->Render(renderer);
	}
	int verticalOffset = 0;
	for (auto it_TreeView = GetTreeViewIterator(); it_TreeView < it_TreeView.end_ptr; it_TreeView++) {
		it_TreeView->GetHovered() = -1;
		for (auto it_Node = it_TreeView->GetNodeIterator(); it_Node < it_Node.end_ptr; it_Node++) {
			SDL_Rect rect = it_Node->GetRect();
			rect.w = rect.h;
			rect.y += verticalOffset;
			if (it_Node->GetState() == NodeState::IS_COLLAPSED) {
				continue;
			}
			if (s_CollideWith(GUIEvent::GetMouseCurrentPos(), rect, { m_Rect.x, m_Rect.y })) {
				it_TreeView->GetHovered() = it_Node.GetIndex();
				break;
			}
			verticalOffset += rect.h;
		}
		it_TreeView->Render(renderer);
	}
	SDL_SetRenderDrawColor(renderer, DefaultGUIColor.r, DefaultGUIColor.g, DefaultGUIColor.b, DefaultGUIColor.a);
	SDL_RenderDrawLine(renderer, 0, 6, 0, m_Rect.h - 7);
	SDL_RenderDrawLine(renderer, m_Rect.w - 1, 7, m_Rect.w - 1, m_Rect.h - 7);
	SDL_RenderDrawLine(renderer, 6, 0, m_Rect.w - 7, 0);
	SDL_RenderDrawLine(renderer, 6, m_Rect.h - 1, m_Rect.w - 7, m_Rect.h - 1);
	drawArc(renderer, 7, 7, 8, 8, 90 * D_PI, 180 * D_PI, DefaultGUIColor);
	drawArc(renderer, m_Rect.w - 8, 7, 8, 8, 0, 90 * D_PI, DefaultGUIColor);
	drawArc(renderer, 7, m_Rect.h - 8, 8, 8, 180 * D_PI, 270 * D_PI, DefaultGUIColor);
	drawArc(renderer, m_Rect.w - 8, m_Rect.h - 8, 8, 8, 270 * D_PI, 360 * D_PI, DefaultGUIColor);
	SDL_SetRenderTarget(renderer, mainTarget);
	SDL_RenderCopy(renderer, m_LayerTexture, NULL, &m_Rect);
}

void gui::Frame::UpdatePosition(Vector2D offset) {
	m_Rect.x += offset.x;
	m_Rect.y += offset.y;
}

void gui::Frame::Render(SDL_Renderer* renderer) {
	SDL_Rect rect = { 0, 0, m_Rect.w, m_Rect.h };
	SDL_RenderCopy(renderer, m_TextureLayer, &rect, &m_Rect);
	SetRenderTarget(renderer);
	SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
	SDL_RenderClear(renderer);
	UnSetRenderTarget(renderer);
}

void gui::HandleGUIEvents(GUIEvent* guiEvent, Layer* layer) {
	if (!s_CollideWith(*guiEvent->GetMousePos(), layer->GetRect()))
		return;
	if (*guiEvent->getMousePressed(SDL_BUTTON_LEFT)) {
		bool handled = false;
		for (auto it_Button = layer->GetButtonIterator(); it_Button < it_Button.end_ptr && !handled; it_Button++) {
			if (s_CollideWith(*guiEvent->GetMousePos(), it_Button->GetRect(), layer->GetPosition())) {
				it_Button->SetState();
				*guiEvent->getMousePressed(SDL_BUTTON_LEFT) = false;
				handled = true;
			}
		}
		for (auto it_ChekcButton = layer->GetCheckButtonIterator(); it_ChekcButton < it_ChekcButton.end_ptr && !handled; it_ChekcButton++) {
			if (s_CollideWith(*guiEvent->GetMousePos(), it_ChekcButton->GetRect(), layer->GetPosition())) {
				it_ChekcButton->SetState();
				*guiEvent->getMousePressed(SDL_BUTTON_LEFT) = false;
				handled = true;
			}
		}
		for (auto it_RadioButton = layer->GetRadioButtonIterator(); it_RadioButton < it_RadioButton.end_ptr && !handled; it_RadioButton++) {
			for (auto it_Rect = it_RadioButton->GetRectIterator(); it_Rect < it_Rect.end_ptr; it_Rect++) {
				if (s_CollideWith(*guiEvent->GetMousePos(), *it_Rect, layer->GetPosition())) {
					it_RadioButton->SetState(it_Rect.GetIndex());
					*guiEvent->getMousePressed(SDL_BUTTON_LEFT) = false;
					handled = true;
					break;
				}
			}
		}
		for (auto it_TreeView = layer->GetTreeViewIterator(); it_TreeView < it_TreeView.end_ptr && !handled; it_TreeView++) {
			int verticalOffset = 0;
			for (auto it_Node = it_TreeView->GetNodeIterator(); it_Node < it_Node.end_ptr; it_Node++) {
				SDL_Rect rect = it_Node->GetRect();
				rect.w = rect.h;
				rect.y += verticalOffset;
				if (it_Node->GetState() == NodeState::IS_COLLAPSED) {
					continue;
				}
				if (s_CollideWith(*guiEvent->GetMousePos(), rect, layer->GetPosition())) {
					it_TreeView->ToggleNode(it_Node.GetIndex());
					*guiEvent->getMousePressed(SDL_BUTTON_LEFT) = false;
					handled = true;
					break;
				}
				verticalOffset += rect.h;
			}
		}
	}
	if (guiEvent->GetMouseState(SDL_BUTTON_LEFT)) {
		for (auto it_Slider = layer->GetSliderIterator(); it_Slider < it_Slider.end_ptr; it_Slider++) {
			if (s_CollideWith(*guiEvent->GetMousePos(), it_Slider->GetRect(), layer->GetPosition())) {
				it_Slider->SetValue(guiEvent->GetMouseCurrentPos(), layer->GetPosition());
			}
		}
	}
}

void gui::HandleSceneEvents(GUIEvent* guiEvent, Frame* frame, void* sceneMeshRaw) {
	container::List<plg::Mesh>* scene = (container::List<plg::Mesh>*)sceneMeshRaw;
	std::unordered_set<int> indexSet;
	int meshID = plg::sceneMeshData.GetMeshID();
	if (guiEvent->GetKeyState(SDL_SCANCODE_SPACE) && meshID != plg::sceneMeshData.NULLMESH && !plg::sceneMeshData.IsCleared()) {
		Vector2D mousePos = guiEvent->GetMouseCurrentPos();
		plg::Vec2 offset(mousePos.x - frame->GetRect().x, mousePos.y - frame->GetRect().y);
  		if (plg::sceneMeshData.GetMode() == plg::MeshMode::PLG_VERTEX) {
			auto vertex_it = plg::sceneMeshData.GetVertexIter();
			offset = offset - scene->operator[](meshID).GetVertexList()->operator[](*vertex_it);
			for (; vertex_it < vertex_it.end_ptr; vertex_it++) {
				indexSet.insert(*vertex_it);
			}
		}
		else if (plg::sceneMeshData.GetMode() == plg::MeshMode::PLG_EDGE) {
			auto edge_it = plg::sceneMeshData.GetEdgeIter();
			plg::Edge init_edge = scene->operator[](meshID).GetEdgeList()->operator[](*edge_it);
			offset = offset - scene->operator[](meshID).GetEdgeCenter(init_edge);
			for (; edge_it < edge_it.end_ptr; edge_it++) {
				plg::Edge edge = scene->operator[](meshID).GetEdgeList()->operator[](*edge_it);
				indexSet.insert(edge.m_Start);
				indexSet.insert(edge.m_End);
			}
		}
		else if (plg::sceneMeshData.GetMode() == plg::MeshMode::PLG_FACE) {
			auto face_it = plg::sceneMeshData.GetFaceIter();
			plg::Face init_face = scene->operator[](meshID).GetFaceList()->operator[](*face_it);
			offset = offset - scene->operator[](meshID).GetFaceCenter(init_face);
			for (; face_it < face_it.end_ptr; face_it++) {
				plg::Face face = scene->operator[](meshID).GetFaceList()->operator[](*face_it);
				indexSet.insert(face.m_Vert1);
				indexSet.insert(face.m_Vert2);
				indexSet.insert(face.m_Vert3);
			}
		}
		for (auto index = indexSet.begin(); index != indexSet.end(); index++) {
			scene->operator[](meshID).GetVertexList()->operator[](*index).AddVec(offset);
		}
	}
	if (!s_CollideWith(*guiEvent->GetMousePos(), frame->GetRect()))
		return;
	if (*guiEvent->getMousePressed(SDL_BUTTON_LEFT)) {
		if (!(guiEvent->GetKeyState(SDL_SCANCODE_LSHIFT) || guiEvent->GetKeyState(SDL_SCANCODE_RSHIFT))) {
			plg::sceneMeshData.Clear();
		}
		Vector2D mousePos = *guiEvent->GetMousePos();
		bool state = false;
		if (plg::sceneMeshData.GetMode() == plg::MeshMode::PLG_VERTEX) {
			state = plg::sceneMeshData.SetVertex(&(scene->operator[](meshID)), plg::Vec2(mousePos.x - frame->GetRect().x, mousePos.y - frame->GetRect().y));
		}
		else if (plg::sceneMeshData.GetMode() == plg::MeshMode::PLG_EDGE) {
			state = plg::sceneMeshData.SetEdge(&(scene->operator[](meshID)), plg::Vec2(mousePos.x - frame->GetRect().x, mousePos.y - frame->GetRect().y));
		}
		else if (plg::sceneMeshData.GetMode() == plg::MeshMode::PLG_FACE) {
			state = plg::sceneMeshData.SetFace(&(scene->operator[](meshID)), plg::Vec2(mousePos.x - frame->GetRect().x, mousePos.y - frame->GetRect().y));
		}
		if (!state && !(guiEvent->GetKeyState(SDL_SCANCODE_LSHIFT) || guiEvent->GetKeyState(SDL_SCANCODE_RSHIFT))) {
			plg::sceneMeshData.Clear();
		}
		*guiEvent->getMousePressed(SDL_BUTTON_LEFT) = false;
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
			guiEvent->SetKeyState(event.key.keysym.scancode, true);
			break;
		case SDL_KEYUP:
			guiEvent->SetKeyState(event.key.keysym.scancode, false);
			break;
		case SDL_MOUSEBUTTONDOWN:
			guiEvent->SetMouseState(event.button.button, true);
			SDL_GetMouseState(&guiEvent->GetMousePos()->x, &guiEvent->GetMousePos()->y);
			*guiEvent->getMousePressed(event.button.button) = true;
			break;
		case SDL_MOUSEBUTTONUP:
			guiEvent->SetMouseState(event.button.button, false);
			*guiEvent->getMousePressed(event.button.button) = false;
			break;
		default:
			break;
		}
	}
}