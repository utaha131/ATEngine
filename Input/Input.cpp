#include "Input.h"

namespace AT::Input {
	static std::mutex lock;
	bool EventState[INPUT_EVENT_COUNT];

	KeyboardState keyboard;
	MouseState mouse;

	void Reset() {
		std::scoped_lock locker(lock);
		mouse.delta_x = 0;
		mouse.delta_y = 0;
	}

	void SetEvent(INPUT_EVENT e, bool value) {
		std::scoped_lock locker(lock);
		EventState[e] = value;
	}

	bool GetEvent(INPUT_EVENT e) {
		std::scoped_lock locker(lock);
		return EventState[e];
	}

	void SetKey(char c, bool value) {
		std::scoped_lock locker(lock);
		keyboard.key[c] = value;
	}

	bool GetKeyDown(char c) {
		std::scoped_lock locker(lock);
		return keyboard.key[c];
	}

	void UpdateMouse(float x, float y, float cx, float cy) {
		std::scoped_lock locker(lock);
		mouse.delta_x += x;
		mouse.delta_y += y;
	}

	MouseState GetMouseState() {
		std::scoped_lock locker(lock);
		return mouse;
	}
}