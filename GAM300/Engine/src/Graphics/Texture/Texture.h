#pragma once

#include "Graphics/GlEnums.h"
#include "resourcemanager/resource.h"

using GLuint = unsigned;

namespace GFX {

  struct Texture
  {
    Texture();
    virtual ~Texture();
    virtual void Bind(TextureUnit _unit = TextureUnit::Diffuse) const = 0;

  protected:
    GLuint      mHandle = 0;
  };

  struct Texture2D : public Texture
  {
    enum class TextureType {
      Diffuse, Normal, Specular, Height
    };

    TextureType mType = TextureType::Diffuse;
    void Bind(TextureUnit _unit = TextureUnit::Diffuse) const override;
  };

  struct Cubemap : public Texture
  {
    void Bind(TextureUnit _unit = TextureUnit::Diffuse) const override;
  };

  using TextureRes = std::shared_ptr<TResource<Texture2D>>;
  using CubemapRes = std::shared_ptr<TResource<Cubemap>>;

}