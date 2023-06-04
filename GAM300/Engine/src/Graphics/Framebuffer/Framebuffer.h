#pragma once

#include "Graphics/GLEnums.h"

using GLuint = unsigned;

namespace GFX {

	class Framebuffer
	{
	public:
		enum class TextureType {
			Texture2D, CubeMap
		};
		enum class AttachmentType {
			Color, Depth
		};

		// Shadow map creation
		void GenerateDepthBuffer(Framebuffer::TextureType _type, const glm::ivec2& _size);
		void GenerateColorBuffer(const glm::ivec2& _size, int _attachments);

		// Cleanup
		~Framebuffer() { Shutdown(); }
		void Shutdown();

		// Gettors
		[[nodiscard]] GLuint GetHandle() const;
		[[nodiscard]] GLuint GetDepthTexture() const;
		[[nodiscard]] GLuint GetColorTexture(int _attachment) const;
		[[nodiscard]] bool IsGenerated() const;
		[[nodiscard]] glm::ivec2 GetSize() const;
		[[nodiscard]] float GetAspectRatio() const;
		[[nodiscard]] Framebuffer::TextureType    GetTextureType() const;
		[[nodiscard]] Framebuffer::AttachmentType GetAttachmentType() const;

		void  BindTexture() const;

	private:
		// Some helpers
		void AttachDepth(Framebuffer::TextureType _type, const glm::ivec2& _size);
		void AttachColor(const glm::ivec2& _size, int _attachments);

		// Variables
		bool				mbGenerated = false; 
		GLuint				mHandle = 0;
		GLuint				mDepthTexture = 0;
		GLuint				mRenderbuffer = 0;
		glm::ivec2			mSize{};
		float				mAspectRatio = 1.0f;
		std::vector<GLuint>	mColor = {};

		Framebuffer::TextureType		mTextureType = Framebuffer::TextureType::Texture2D;
		Framebuffer::AttachmentType		mAttachments = Framebuffer::AttachmentType::Color;
	};

	class GBuffer
	{
	public:
		void Initialize(const glm::ivec2& _size);
		void Shutdown();
		GLuint GenerateTexture(const glm::ivec2& _size, bool high_precision);

		[[nodiscard]] GLuint GetHandle() const { return mHandle; }

		GLuint mHandle = 0;
		GLuint mPositionBuffer = 0;
		GLuint mNormalBuffer = 0;
		GLuint mSpecularAlbedo = 0;
		GLuint mDepth = 0;
		
		bool mbGenerated = false;
		glm::ivec2 mSize{};
	};
}
