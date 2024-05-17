#ifndef _AT_RESOURCE_MANAGER_IMAGE_MANAGER_H_
#define _AT_RESOURCE_MANAGER_IMAGE_MANAGER_H_
#define STB_IMAGE_IMPLEMENTATION
#include <unordered_map>
#include <string>
#include <stb/stb_image.h>

namespace AT {
	class Image {
	public:
		Image(std::string& path, bool is_srgb) :
			m_is_srgb(is_srgb)
		{
			int channels = 0;
			m_data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 4);
			m_data_size = 4 * m_width * m_height;
		}

		~Image() {
			free(m_data);
		}

		unsigned char* GetBytes() const {
			return m_data;
		}

		float* GetFloats() const {
			return m_srgb;
		}

		uint64_t GetDataSize() const {
			return m_data_size;
		}

		uint32_t GetWidth() const {
			return m_width;
		}

		uint32_t GetHeight() const {
			return m_height;
		}
	private:
		int32_t m_width;
		int32_t m_height;
		unsigned char* m_data;
		float* m_srgb;
		bool m_is_srgb;
		uint64_t m_data_size;
	};

	class ImageManager {
	public:
		Image CreateResource(const std::string& name, const std::string& path) {
			/*if (m_image_cache.find(name) != m_image_cache.end()) {
				return m_image_cache[name];
			}
			else {

			}*/
		}

		void FreeResource() {

		}
	private:
		//std::unordered_map<std::string, Image> m_image_cache;
	};
}
#endif