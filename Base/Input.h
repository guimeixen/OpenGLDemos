#pragma once

#include "glm\glm.hpp"

struct Key
{
	bool state;
	bool justReleased;
	bool justPressed;
};

struct Button
{
	bool state;
	bool justReleased;
	bool justPressed;
};

enum MouseButton
{
	Left,
	Right
};

class InputManager
{
public:
	InputManager();
	~InputManager();

		void Update();			// Needs to be called at the start of the frame to reset the just released state. If two requests of WasKeyReleased() are made to the same key only the first would return
								// the correct state because in the first call it would then reset the state.
		void Reset();

		int AnyKeyPressed() const;
		bool GetLastChar(unsigned char &c);
		bool IsKeyPressed(int keycode) const;
		bool WasKeyPressed(int keycode) const;
		bool WasKeyReleased(int keycode) const;
		bool WasMouseButtonReleased(int button);
		bool MouseMoved() const;
		bool IsMousePressed(int button) const;
		bool IsMouseButtonDown(int button) const;
		float GetScrollWheelY() const { return scrollWheelY; }

		void SetMousePosition(const glm::vec2 &pos);
		const glm::vec2 &GetMousePosition() const { return mousePosition; }

		void UpdateKeys(int key, int scancode, int action, int mods);
		void UpdateChar(unsigned char c);
		void SetMouseButtonState(int button, int action);
		void SetScrollWheelYOffset(float yoffset);

	private:
		Key keys[512];
		glm::vec2 mousePosition;
		bool mouseMoved;
		Button mouseButtonsState[2];		// 0 if left mouse button, 1 is right mouse button
		unsigned char lastChar;
		bool charUpdated;
		float scrollWheelY;
	};

	class Input
	{
	private:
		friend class InputManager;
	public:
		static int AnyKeyPressed() { return inputManager->AnyKeyPressed(); }
		static bool GetLastChar(unsigned char &c) { return inputManager->GetLastChar(c); }
		static bool IsKeyPressed(int keycode) { return inputManager->IsKeyPressed(keycode); }
		static bool WasKeyPressed(int keycode) { return inputManager->WasKeyPressed(keycode); }
		static bool WasKeyReleased(int keycode) { return inputManager->WasKeyReleased(keycode); }
		static bool WasMouseButtonReleased(int button) { return inputManager->WasMouseButtonReleased(button); }
		static bool IsMousePressed(int button) { return inputManager->IsMousePressed(button); }
		static bool IsMouseButtonDown(int button) { return inputManager->IsMouseButtonDown(button); }
		static bool MouseMoved() { return inputManager->MouseMoved(); }
		static const glm::vec2 &GetMousePosition() { return inputManager->GetMousePosition(); }
		static float GetScrollWheelY() { return inputManager->GetScrollWheelY(); }

		static InputManager *GetInputManager() { return inputManager; }

	private:
		static InputManager *inputManager;
	};
