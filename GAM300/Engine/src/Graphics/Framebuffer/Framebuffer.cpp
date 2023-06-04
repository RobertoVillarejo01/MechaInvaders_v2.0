#include "Graphics/Lighting/Light.h"
#include "Framebuffer.h"

#include <GL/glew.h>
#include <GL/GL.h>

namespace GFX {

	/**
	 * Generate a framebuffer for only depth testing purposes. Either a 2D texture or a CubeMap
	 */
	void GFX::Framebuffer::GenerateDepthBuffer(Framebuffer::TextureType _type, 
		const glm::ivec2& _size)
	{
		// First of all, delete the already existing shadow map if needed
		if (mbGenerated) Shutdown();

		// Set the given properties
		mTextureType = _type;
		mAttachments = Framebuffer::AttachmentType::Depth;
		mSize = _size;
		mAspectRatio = ((float)_size.x) / ((float)_size.y);

		// Create the frame buffer that will hold the texture we will be drwaing onto. 
		glGenFramebuffers(1, &mHandle);

		// Depth = Single channel of depth, no color nor stencil
		AttachDepth(_type, _size);

		// Security check that everything went fine
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Unable to generate framebuffer" << std::endl;
			Shutdown();
			return;
		}

		// We are done, unbind everything just to be safe
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Save that the shadow map is valid and generated
		mbGenerated = true;
	}

	void Framebuffer::GenerateColorBuffer(const glm::ivec2& _size, int _attachments)
	{
		// Tampoco nos calentemos
		assert(_attachments <= 4);

		// First of all, delete the already existing shadow map if needed
		if (mbGenerated) Shutdown();

		// Set the given properties
		mTextureType = Framebuffer::TextureType::Texture2D;
		mAttachments = Framebuffer::AttachmentType::Color;
		mSize = _size;
		mAspectRatio = ((float)_size.x) / ((float)_size.y);

		// Create the frame buffer that will hold the texture we will be drwaing onto. 
		glGenFramebuffers(1, &mHandle);

		// Color = N channels of color + RenderBuffer for the depth
		AttachColor(_size, _attachments);

		// Security check that everything went fine
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Unable to generate framebuffer" << std::endl;
			Shutdown();
			return;
		}

		// We are done, unbind everything just to be safe
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Save that the shadow map is valid and generated
		mbGenerated = true;
	}

	void Framebuffer::AttachDepth(Framebuffer::TextureType _type, const glm::ivec2& _size)
	{
		// Create the actual texture
		glGenTextures(1, &mDepthTexture);

		// For directional and sport lights we need a texture 2D
		if (_type == Framebuffer::TextureType::Texture2D) {
			glBindTexture(GL_TEXTURE_2D, mDepthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _size.x, _size.y,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		// For point lights however a cube map
		else if (_type == Framebuffer::TextureType::CubeMap) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, mDepthTexture);
			glTexStorage2D(GL_TEXTURE_CUBE_MAP, 10, GL_DEPTH_COMPONENT24, _size.x, _size.y);
			for (unsigned i = 0; i < 6; ++i) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
					_size.x, _size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			}
		}
		else {
			glDeleteTextures(1, &mDepthTexture);
			mDepthTexture = 0;
			return;
		}

		// Set the texture configuration
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Now, attach the texture onto the frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, mHandle);

		if (_type == Framebuffer::TextureType::Texture2D) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);
		}
		else if (_type == Framebuffer::TextureType::CubeMap) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0);
		}

		// Framebuffers "need" to have a color attachment to be complete, however, we do not need
		// it, we just need the depth info, so we are explicitly telling that we are not going to
		// use it by saying that we do not read nor write data
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void Framebuffer::AttachColor(const glm::ivec2& _size, int _attachments)
	{
		// Color attachements
		mColor.resize(_attachments);
		glGenTextures(_attachments, &mColor[0]);
		for (int i = 0; i < _attachments; ++i) {
			// Set the type of texture (Notice that is RGBA16F instead of just RGBA)
			glBindTexture(GL_TEXTURE_2D, mColor[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _size.x, _size.y, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// Add it to the framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 
				mColor[i], 0);
		}

		// Explicitly notify that we will be drawing onto N different color attachments
		std::vector<unsigned> active_attachments;
		for (int i = 0; i < _attachments; ++i)
			active_attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		glDrawBuffers(_attachments, active_attachments.data());

		// Depth (through a render buffer attachment, which we cannot directly read,
		// but we never intended to in the first place so it's ideal)
		glGenRenderbuffers(1, &mRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _size.x, _size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderbuffer);
	}

	void GFX::Framebuffer::Shutdown()
	{
		// Delete the textures
		if (mDepthTexture != 0) {
			glDeleteTextures(1, &mDepthTexture);
			mDepthTexture = 0;
		}
		for (auto color_texture : mColor) {
			if (color_texture != 0) {
				glDeleteTextures(1, &color_texture);
			}
		}
		mColor.clear();

		// Delete the renderbuffer
		if (mRenderbuffer != 0) {
			glDeleteRenderbuffers(1, &mRenderbuffer);
			mRenderbuffer = 0;
		}

		// Delete the actual framebuffer
		if (mHandle != 0) {
			glDeleteFramebuffers(1, &mHandle);
			mHandle = 0;
		}

		mbGenerated = false;
	}
	GLuint Framebuffer::GetHandle() const
	{
		return mHandle;
	}
	GLuint Framebuffer::GetDepthTexture() const
	{
		assert(mAttachments == AttachmentType::Depth);
		return mDepthTexture;
	}
	GLuint Framebuffer::GetColorTexture(int _attachment) const
	{
		assert(mAttachments == AttachmentType::Color);
		assert(_attachment < mColor.size());
		return mColor[_attachment];
	}

	void Framebuffer::BindTexture() const
	{
		switch (mTextureType)
		{
		case Framebuffer::TextureType::Texture2D:
			glBindTexture(GL_TEXTURE_2D, mDepthTexture);
			break;
		case Framebuffer::TextureType::CubeMap:
			glBindTexture(GL_TEXTURE_CUBE_MAP, mDepthTexture);
			break;
		default:
			break;
		}
	}
	bool Framebuffer::IsGenerated() const
	{
		return mbGenerated;
	}
	glm::ivec2 Framebuffer::GetSize() const
	{
		return mSize;
	}
	float Framebuffer::GetAspectRatio() const
	{
		return mAspectRatio;
	}
	Framebuffer::TextureType Framebuffer::GetTextureType() const
	{
		return mTextureType;
	}
	Framebuffer::AttachmentType Framebuffer::GetAttachmentType() const
	{
		return mAttachments;
	}











	GLuint GBuffer::GenerateTexture(const glm::ivec2& _size, bool high_precision)
	{
		auto texture_type = high_precision ? GL_RGBA16F : GL_RGBA;

		GLuint TextureHandle;
		glGenTextures(1, &TextureHandle);
		glBindTexture(GL_TEXTURE_2D, TextureHandle);
		glTexImage2D(GL_TEXTURE_2D, 0, texture_type, _size.x, _size.y, 0, GL_RGBA, 
			high_precision ? GL_FLOAT : GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		return TextureHandle;
	}
	void GBuffer::Initialize(const glm::ivec2& _size)
	{
		// Set the size
		mSize = _size;
		CheckGL_Error();

		// First of all, delete the already existing shadow map if needed
		if (mbGenerated) Shutdown();

		// Generate the buffer and the different textures
		CheckGL_Error();
		glGenFramebuffers(1, &mHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
		CheckGL_Error();

		// Attach the different textures of the GBuffer
		mPositionBuffer = GenerateTexture(_size, true);
		mNormalBuffer	= GenerateTexture(_size, true);
		mSpecularAlbedo = GenerateTexture(_size, false);
		CheckGL_Error();

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPositionBuffer, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mNormalBuffer, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mSpecularAlbedo, 0);

		CheckGL_Error();

		GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

		// Attach the depth buffer too
		glGenRenderbuffers(1, &mDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, mDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _size.x, _size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepth);

		CheckGL_Error();

		// Security check that everything went fine
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Unable to generate framebuffer" << std::endl;
			Shutdown();
			return;
		}

		// We are done, unbind everything just to be safe
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CheckGL_Error();

		// Save that the shadow map is valid and generated
		mbGenerated = true;
	}
	void GBuffer::Shutdown()
	{
		if (mHandle != 0)			glDeleteBuffers(1, &mHandle);
		if (mPositionBuffer != 0)	glDeleteTextures(1, &mPositionBuffer);
		if (mNormalBuffer != 0)		glDeleteTextures(1, &mNormalBuffer);
		if (mSpecularAlbedo != 0)	glDeleteTextures(1, &mSpecularAlbedo);
		if (mDepth != 0)			glDeleteRenderbuffers(1, &mDepth);
	}
}
