#include "Graphics/Texture/Texture.h"
#include <GL/glew.h>
#include <GL/gl.h>

namespace GFX {
  Texture::Texture()
  {
    glGenTextures(1, &mHandle);
  }
  Texture::~Texture()
  {
    glDeleteTextures(1, &mHandle);
  }
  void Texture2D::Bind(TextureUnit _unit) const
  {
    glActiveTexture(GL_TEXTURE0 + (uint32_t)_unit);
    glBindTexture(GL_TEXTURE_2D, mHandle);
  }
  void Cubemap::Bind(TextureUnit _unit) const
  {
    glActiveTexture(GL_TEXTURE0 + (uint32_t)_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mHandle);
  }
}