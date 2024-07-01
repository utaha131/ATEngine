#ifndef _RHI_I_SHADER_H_
#define _RHI_I_SHADER_H_
#include <string>

namespace RHI {
#undef DOMAIN
	enum class ShaderType {
		VERTEX,
		DOMAIN,
		HULL,
		GEOMETRY,
		PIXEL,
		COMPUTE,
		LIBRARY,
	};

	typedef struct ShaderDescription {
		ShaderType ShaderType;
		std::string SourcePath;
		std::string EntryPoint;
	} ShaderDescription;

	class IShader {
	public:
		virtual ~IShader() = default;
		const ShaderDescription& GetDescription() const { return m_Description; }
	protected:
		ShaderDescription m_Description;
		IShader(const ShaderDescription& description) :
			m_Description(description) {};
	};
}
#endif