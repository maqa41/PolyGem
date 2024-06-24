#pragma once
#include "SDL.h"
#include "SDL_ttf.h"
#include "container.h"
#include <string>
#include <utility>

namespace gui {

#define GUI_VERTICAL 1u
#define GUI_HORIZONTAL 2u
#define GUI_MAXWIN { 0, 0, 640, 640 }

	inline SDL_Color DefaultColorBG = { 36, 36, 36, SDL_ALPHA_OPAQUE };
	inline SDL_Color DefaultGUIColor = { 96, 96, 96, SDL_ALPHA_OPAQUE };
	inline SDL_Color DefaultTextColor = { 180, 180, 180, SDL_ALPHA_OPAQUE };
	inline SDL_Color DefaultPrimaryButtonColor = { 36, 180, 112, SDL_ALPHA_OPAQUE };
	inline SDL_Color DefaultDestructiveButtonColor = { 180, 64, 24, SDL_ALPHA_OPAQUE };

	struct Vector2D {
		int x, y;

		Vector2D(int xX = 0, int yY = 0) : x(xX), y(yY) { }
	};

	enum class KeyButton {
		NONE = 0,
		KEY_ESCAPE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11,
		KEY_F12, KEY_INSERT, KEY_DELETE, KEY_BACKQUOTE, KEY_EXCLAIM, KEY_AT, KEY_HASH, KEY_DOLLAR, KEY_PERCENT, KEY_CARET,
		KEY_AMPERSAND, KEY_ASTERISK, KEY_LEFTPAREN, KEY_RIGHTPAREN, KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE,
		KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACKET,
		KEY_RIGHTBRACKET, KEY_BACKSLASH, KEY_CAPSLOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K,
		KEY_L, KEY_SEMICOLON, KEY_QUOTE, KEY_ENTER, KEY_SHIFT_L, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M,
		KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_SHIFT_R, KEY_CTRL_L, KEY_MOD, KEY_ALT_L, KEY_SPACE,
		KEY_ALT_R, KEY_CTRL_R, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
		KEY_6, KEY_7, KEY_8, KEY_9, KEY_SLASH_KP, KEY_ASTERISK_KP, KEY_MINUS_KP, KEY_PLUS, KEY_DEL, KEY_ENTER_KP,
		KEY_HOME, KEY_END, KEY_PAGEUP, KEY_PAGEDOWN
	};

	enum class MouseButton {
		NONE = 0,
		MOUSE_BUTTON_LEFT, MOUSE_BUTTON_MIDDLE, MOUSE_BUTTON_RIGHT
	};
	
	class GUIEvent {
	public:
		GUIEvent() {
			for (size_t index = 0; index < SDL_NUM_SCANCODES; index++) {
				m_KeyState[index] = false;
				if (index < 4) {
					m_MouseState[index] = false;
					m_MousePressed[index] = false;
				}
			}
		}

		~GUIEvent() { }

		bool* GetQuitState() { return &m_QuitState; }
		void SetKeyState(size_t key, bool state) { m_KeyState[key] = state; }
		bool GetKeyState(size_t key) { return m_KeyState[key]; }
		void SetMouseState(size_t button, bool state) { m_MouseState[button] = state; }
		bool GetMouseState(size_t button) { return m_MouseState[button]; }
		bool* GetMousePressed(size_t button) { return &m_MousePressed[button]; }
		Vector2D* GetMousePos() { return &m_MousePos; }
		static Vector2D GetMouseCurrentPos() {
			Vector2D mouseMotionPos;
			SDL_GetMouseState(&mouseMotionPos.x, &mouseMotionPos.y);
			return mouseMotionPos;
		}

	private:
		bool m_QuitState = false;
		bool m_KeyState[SDL_NUM_SCANCODES];
		bool m_MouseState[4];
		bool m_MousePressed[4];
		Vector2D m_MousePos;
	};

	class SubWindow {

	};
	
	class Label {
	public:

		Label() { }
		Label(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG = DefaultTextColor, SDL_Color colorBG = DefaultColorBG);
		Label(const Label& other);
		Label(Label&& other) noexcept;
		Label& operator=(const Label& other);
		Label& operator=(Label&& other) noexcept;
		~Label() { SDL_DestroyTexture(m_TextTexture); }

		const char* GetText() { return m_Text.c_str(); }
		SDL_Color GetColorFG() { return m_ColorFG; }
		SDL_Color GetColorBG() const { return m_ColorBG; }
		uint8_t GetSize() const { return m_Size; }
		SDL_Rect GetRect() const { return m_Rect; }
		SDL_Texture** GetTextTexture() { return &m_TextTexture; }
		void UpdatePosition(Vector2D offset);
		void Render(SDL_Renderer* renderer, Vector2D offset = { 0, 0 });

	private:
		std::string m_Text;
		uint8_t m_Size;
		SDL_Color m_ColorFG;
		SDL_Color m_ColorBG;
		SDL_Rect m_Rect;
		SDL_Texture* m_TextTexture = nullptr;
	};

	enum class NodeState {
		NORMAL = 0x00,
		CHILD_EXPANDING = 0x01,
		CHILD_COLLAPSING = 0x02,
		HAS_CHILD_EXPANDED = 0x03,
		HAS_CHILD_COLLAPSED = 0x04,
		IS_EXPANDED = 0x05,
		IS_COLLAPSED = 0x06,
		NO_CHILD = 0x70
	};

	class LabelNode : public Label {
	public:
		LabelNode() : Label() { }
		LabelNode(uint8_t parent, SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG = DefaultTextColor, SDL_Color colorBG = DefaultColorBG)
			: Label(renderer, rect, text, size, colorFG, colorBG), m_Parent(parent) { }
		LabelNode(const LabelNode& other);
		LabelNode(LabelNode&& other) noexcept;
		LabelNode& operator=(const LabelNode& other);
		LabelNode& operator=(LabelNode&& other) noexcept;
		~LabelNode() { }

		uint8_t GetParent() const { return m_Parent; }
		void SetParent(int8_t parent) { m_Parent = parent; }
		NodeState GetState() const { return m_State; }
		void SetState(NodeState state) { m_State = state; }
		NodeState GetChildState() const { return m_ChildState; }
		void SetChildState(NodeState state) { m_ChildState = state; }

		uint8_t GetChildCount() const { return m_ChildCount; }
		void IncChild() { m_ChildCount++; }
		void DecChild() { m_ChildCount--; }
		uint8_t GetChildSetCounter() const { return m_ChildSetCounter; }
		void SetChildSetCounter() { m_ChildSetCounter = m_ChildCount; }
		void DecSetChild() { m_ChildSetCounter--; }

		static constexpr uint8_t NO_PARENT = 0xff;
		static constexpr uint8_t NULLED = 0x00;

	private:
		uint8_t m_Parent = NO_PARENT;
		NodeState m_State = NodeState::IS_EXPANDED;
		NodeState m_ChildState = NodeState::NO_CHILD;
		uint8_t m_ChildCount = NULLED;
		uint8_t m_ChildSetCounter = NULLED;
		//SDL_Texture* m_Texture = nullptr;
	};

	class Button {
	public:
		Button() { }
		Button(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG = DefaultPrimaryButtonColor, SDL_Color labelColor = DefaultTextColor);
		Button(const Button& other);
		Button(Button&& other) noexcept;
		Button& operator=(const Button& other);
		Button& operator=(Button&& other) noexcept;

		void SetState() { m_State = !m_State; }
		bool GetState() { return m_State; }
		bool& GetHovered() { return m_IsHovered; }
		SDL_Rect GetRect() { return m_Rect; }
		Label* GetLabel() { return &m_Label; }
		SDL_Color GetColorFG() { return m_ColorFG; }
		void Render(SDL_Renderer* renderer);

	private:
		bool m_IsHovered = false;
		bool m_State = false;
		Label m_Label;
		SDL_Rect m_Rect;
		SDL_Color m_ColorFG;
	};

	class Slider {
	public:
		Slider() { }
		Slider(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, float maxVal, uint8_t orientation, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor, SDL_Color colorBG = DefaultColorBG);
		Slider(const Slider& other);
		Slider(Slider&& other) noexcept;
		Slider& operator=(const Slider& other);
		Slider& operator=(Slider&& other) noexcept;

		Label* GetLabel() { return &m_Label; }
		SDL_Color GetColorFG() { return m_ColorFG; }
		SDL_Rect GetRect() { return m_Rect; }
		void SetValue(Vector2D mousePos, Vector2D offset);
		void Render(SDL_Renderer* renderer);

	private:
		Label m_Label;
		SDL_Color m_ColorFG;
		SDL_Rect m_Rect;
		float m_Value = 0.0;
		float m_MaxVal;
		uint8_t m_Orientation;
		bool m_IsHovered = false;
		bool m_State = false;
	};

	class RadioButton {
	public:
		RadioButton() { }
		RadioButton(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, uint8_t size, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor, SDL_Color colorBG = DefaultColorBG);
		RadioButton(const RadioButton& other);
		RadioButton(RadioButton&& other) noexcept;
		RadioButton& operator=(const RadioButton& other);
		RadioButton& operator=(RadioButton&& other) noexcept;

		SDL_Color GetColorFG() { return m_ColorFG; }
		int8_t& GetHovered() { return m_IsHovered; }
		void SetState(uint8_t button) { m_SetButton = button; }
		uint8_t GetState() { return m_SetButton; }
		container::ListIterator<container::List<Label>> GetLabelIterator() { return m_Labels.Begin(); }
		container::List<Label>* GetLabelList() { return &m_Labels; }
		container::ListIterator<container::List<SDL_Rect>> GetRectIterator() { return m_Rects.Begin(); }
		container::List<SDL_Rect>* GetRectList() { return &m_Rects; }
		void Render(SDL_Renderer* renderer);

	private:
		SDL_Color m_ColorFG;
		int8_t m_IsHovered = -1;
		uint8_t m_SetButton = 0;
		container::List<Label> m_Labels;
		container::List<SDL_Rect> m_Rects;
	};

	class CheckButton {
	public:
		CheckButton() { }
		CheckButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor, SDL_Color colorBG = DefaultColorBG);
		CheckButton(const CheckButton& other);
		CheckButton(CheckButton&& other) noexcept;
		CheckButton& operator=(const CheckButton& other);
		CheckButton& operator=(CheckButton&& other) noexcept;

		bool& GetHovered() { return m_IsHovered; }
		void SetState() { m_State = !m_State; }
		bool GetState() { return m_State; }
		SDL_Color GetColorFG() { return m_ColorFG; }
		SDL_Rect GetRect() { return m_Rect; }
		Label* GetLabel() { return &m_Label; }
		void Render(SDL_Renderer* renderer);

	private:
		SDL_Color m_ColorFG;
		SDL_Rect m_Rect;
		Label m_Label;
		bool m_IsHovered = false;
		bool m_State = false;
	};

	class TreeView {
	public:
		TreeView() { }
		TreeView(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, std::initializer_list<int> layers, uint8_t size, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor, SDL_Color colorBG = DefaultColorBG);
		TreeView(const TreeView& other);
		TreeView(TreeView&& other) noexcept;
		TreeView& operator=(const TreeView& other);
		TreeView& operator=(TreeView&& other) noexcept;
		~TreeView() { }

		container::ListIterator<container::List<LabelNode>> GetNodeIterator() { return m_LabelNodes.Begin(); }
		int& GetHovered() { return m_IsHovered; }
		void InsertNode(size_t index);
		void DeleteNode(size_t index);
		void ToggleNode(size_t index);
		void UpdateStates();
		void Render(SDL_Renderer* renderer);

	private:
		container::List<LabelNode> m_LabelNodes;
		int m_IsHovered = -1;
	};

	class Layer {
	public:
		Layer(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color colorBG) : m_Rect(rect), m_ColorBG(colorBG) {
			m_LayerTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, rect.w, rect.h);
		}

		void AddButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG = DefaultPrimaryButtonColor, SDL_Color labelColor = DefaultTextColor);
		void AddSlider(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, float maxVal, uint8_t orientation, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor);
		void AddRadioButton(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, uint8_t size, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor);
		void AddCheckButton(SDL_Renderer* renderer, SDL_Rect rect, const char* text, uint8_t size, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor);
		void AddTreeView(SDL_Renderer* renderer, SDL_Rect rect, std::initializer_list<const char*> labels, std::initializer_list<int> layers, uint8_t size, SDL_Color colorFG, SDL_Color labelColor = DefaultTextColor, SDL_Color colorBG = DefaultColorBG);

		Vector2D GetPosition() { return { m_Rect.x, m_Rect.y }; }
		SDL_Rect GetRect() { return m_Rect; }
		container::ListIterator<container::List<Button>> GetButtonIterator() { return m_Buttons.Begin(); }
		container::ListIterator<container::List<Slider>> GetSliderIterator() { return m_Sliders.Begin(); }
		container::ListIterator<container::List<RadioButton>> GetRadioButtonIterator() { return m_RadioButtons.Begin(); }
		container::ListIterator<container::List<CheckButton>> GetCheckButtonIterator() { return m_CheckButtons.Begin(); }
		container::ListIterator<container::List<TreeView>> GetTreeViewIterator() { return m_TreeViews.Begin(); }
		void Render(SDL_Renderer* renderer);

	private:
		SDL_Rect m_Rect;
		SDL_Color m_ColorBG;
		SDL_Texture* m_LayerTexture;
		container::List<Button> m_Buttons = container::List<Button>(2);
		container::List<Slider> m_Sliders = container::List<Slider>(2);
		container::List<RadioButton> m_RadioButtons = container::List<RadioButton>(2);
		container::List<CheckButton> m_CheckButtons = container::List<CheckButton>(2);
		container::List<TreeView> m_TreeViews = container::List<TreeView>(2);
	};

	class Frame {
	public:
		Frame(SDL_Renderer* renderer, SDL_Rect rect, SDL_Rect window = GUI_MAXWIN) : m_Rect(rect), m_Window(window) {
			m_TextureLayer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_Window.w, m_Window.h);
			m_MainWindow = SDL_GetRenderTarget(renderer);
		}

		~Frame() { }

		SDL_Rect GetRect() { return m_Rect; }
		void SetRenderTarget(SDL_Renderer* renderer) { SDL_SetRenderTarget(renderer, m_TextureLayer); }
		void UnSetRenderTarget(SDL_Renderer* renderer) { SDL_SetRenderTarget(renderer, m_MainWindow); }
		void UpdatePosition(Vector2D offset);
		void ResizeWindow(Vector2D offset);
		void Render(SDL_Renderer* renderer);

	private:
		SDL_Rect m_Rect;
		SDL_Rect m_Window;
		SDL_Texture* m_TextureLayer;
		SDL_Texture* m_MainWindow;
	};

	class MenuBar {

	};

	class Menu {
	public:
		Menu() { }

		void Render();
	private:

	};

	class Tab {

	};

	void InitializeGUIStatics(SDL_Renderer* renderer);
	void HandleGUIEvents(GUIEvent* guiEvent, Layer* layer);
	void HandleSceneEvents(GUIEvent* guiEvent, Frame* frame, void* sceneMeshRaw);
	void RetriveGUIEvents(GUIEvent* guiEvent);
	void RenderPressedKeys(SDL_Renderer* renderer, GUIEvent* guiEvent);
}