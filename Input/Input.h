#ifndef _AT_INPUT_H_
#define _AT_INPUT_H_
#include <mutex>

namespace AT::Input {
	struct KeyboardState {
		bool key[256] = {};
	};

	struct MouseState {
		float position_x = 0.0f;
		float position_y = 0.0f;
		float delta_x = 0.0f;
		float delta_y = 0.0f;
		bool button[3] = {};
	};

	enum INPUT_EVENT {
		INPUT_EVENT_QUIT,
		INPUT_EVENT_COUNT
	};
	void Reset();
	void SetEvent(INPUT_EVENT e, bool value);
	void SetKey(char c, bool value);
	void UpdateMouse(float x, float y, float cx, float cy);
	MouseState GetMouseState();
	bool GetKeyDown(char c);
	bool GetEvent(INPUT_EVENT e);
}
#endif