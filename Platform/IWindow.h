#ifndef _AT_I_WINDOW_H_
#define _AT_I_WINDOW_H_
#include <stdint.h>
#include <string>

namespace AT {
	
	typedef struct WindowDescription {
		std::string Title;
		uint32_t Width;
		uint32_t Height;
	} WindowDescription;

	typedef struct WindowEvent {

	};

	class IWindow {
	public:
		virtual void PollEvent(WindowEvent& event) const = 0;
		virtual void* GetNative() const = 0;
		uint32_t GetWidth() const {
			return m_description.Width;
		}

		uint32_t GetHeight() const{
			return m_description.Height;
		}
	protected:
		IWindow(const WindowDescription& description) :
			m_description(description) {}
		WindowDescription m_description;
	};
}
#endif