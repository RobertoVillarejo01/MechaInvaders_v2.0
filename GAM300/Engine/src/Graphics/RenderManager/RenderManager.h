#pragma once

#include "Utilities/Math.h"
#include "Utilities/Singleton.h"
#include "Graphics/GlEnums.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/ShaderProgram/ShaderProgram.h"
#include "Graphics/TextRendering/TextRender.h"

class LightComponent;

namespace GFX {

	// OpenGL debugging
	void EnableGLErrorCallbacks();
	void  _check_gl_error(const char* file, int line);
	#define CheckGL_Error() GFX::_check_gl_error(__FILE__, __LINE__)

	class Mesh;
	class Model;
	class Camera;

	class RenderManager /*: public ISystem�? */
	{
		MAKE_SINGLETON(RenderManager)

	public:
		// Intialize the render manager
		void Load();
		void Initialize();

		// Mostly avoid having pointers to potentially deleted memory
		void Free();

		void RenderScene();
#ifdef EDITOR
		void RenderEditor();
		void ChangeAllObjectsVisibility(bool _visible);
#endif

		// Currently bound buffer clearing
		void ClearBuffer() const;
		void ClearColorBuffer() const;
		void ClearDepthBuffer() const;

		// Basic Rendering Settings
		void  SetClearDepth(float _depth);
		float GetClearDepth() { return mClearDepth; }

		void  SetClearColor(const glm::vec4& _color);
		const glm::vec4& GetClearColor() { return mClearColor; }

		void  SetPolygonMode(PolygonMode_t _mode);
		PolygonMode_t GetPolygonMode() { return mPolyMode; }

		void  SetLightingConfiguration(LConfig _config);
		const Camera* GetCurrentCamera() const { return mCurrentCamera; }

		// Various utilities
		void ReloadShaders();
		void SetDefaultBuffer() const;

		/**
		 * Given the current shader program and camera, draw the given mesh
		 */
		void RenderMesh(const Mesh* _mesh, renderable* _renderable /* This should actually be a
				texture or at least have an overloaded function with a texture */, const glm::mat4& _m2w,
			PolygonMode_t _mode);
		void RenderMesh(const Mesh* _mesh, const glm::vec4& _color /* This should actually be a
				texture or at least have an overloaded function with a texture */, const glm::mat4& _m2w,
			PolygonMode_t _mode);

		/**
		 * Given the current shader program and camera, draw the given model (that is just a
		 * bunch of textures)
		 */
		void RenderModel(const Model* _model, renderable* _renderable /* This should actually be a
				texture or at least have an overloaded function with a texture */, const glm::mat4& _m2w,
			PolygonMode_t _mode);
		void RenderModel(const Model* _model, const glm::vec4& _color /* This should actually be a
				texture or at least have an overloaded function with a texture */, const glm::mat4& _m2w,
			PolygonMode_t _mode);


		void RenderText(FontRes mFont, glm::vec2 mScale, glm::vec3 mOffset,
			glm::vec4 mColor, Transform const& mTransform, bool mbHorizontalBill, bool mbVerticalBill,
			TextComponent::Alignment mAlignment, std::string const& msg);

		/**
		 * Render a cube map for the whole scene
		 */
		void RenderCubemap(CubemapRes _cubemap);

		/**
		 * Sets a camera so that meshes drawn after are affected by its transformations
		 * (unless the camera is a nullptr -> w2proj = identity) and updates its matrices
		 * The camera position and direction should already be set at this point
		 */
		void SetCamera(Camera* _camera);

		/**
		 * I still have no idea if this function makes any sense at all, maybe we want it to draw
		 * a certain subset of elements with the same shader program  (that is not the one they are
		 * currently holding in their meshses (a debug shader or smth))
		 */
		void SetShaderProgram(Shader_t _type);
		Shader_t GetCurrShaderProgram() const { return mCurrShaderProgramName; }
		void InitializeLighting();

	private:

		// Rendering
		void RenderSceneForward();
		void RenderSceneDeferred();
		bool forward = true;

		/**
		 * We need to render the scene with the light being a camera
		 */
		void SetLigthAsCamera(LightComponent* _light);

		/**
		 * Render the texture of the given framebuffer to screen (Makes more sense if it is a color
		 * framebuffer but it is not limited to it)
		 */
		void RenderScreenQuad(GBuffer const& _fb);
		void RenderScreenQuad(Framebuffer const& _fb);

		// Basic Rendering Settings
		glm::vec4	    mClearColor = {};
		float           mClearDepth = 1.0f;
		bool			mbUseShaderFromMesh = true;

		// The current render confug, likely to change several times per frame
		Camera* mCurrentCamera = nullptr;
		PolygonMode_t mPolyMode = PolygonMode_t::Solid;
		ShaderProgram* mCurrentShaderProgram = nullptr;
		Shader_t mCurrShaderProgramName = Shader_t::None;

		// The different shader, it could be done using a container, but we will have few
		BasicShaderProgram			mBasicShader = {};
		BasicShaderProgram			mDepthShader = {};
		CubemapShaderProgram		mCubeMapShader = {};
		DepthCubeShaderProgram		mCubeDepthShader = {};
		LightShaderProgram			mLightShader = {};
		ToGBufferShaderProgram		mToGBufferShader = {};
		ShaderProgram				mGBufferMixerShader = {};
		ShaderProgram				mScreenQuadShader = {};
		ShaderProgram				mBlurShader = {};
		ShaderProgram				mBlendShader = {};
		ParticleShaderProgram		mParticleShader = {};
		ShaderProgram				mTextShader = {};
		
		// For the lighting system
		using light_container = std::vector<LightComponent*>;
		void CleanShadowMaps(Space* _space);
		void ComputeShadowMaps(Space* _space, light_container const& _lights);
		void UploadLights(light_container const& basic_lights,
			light_container const& shadow_lights);
		void UploadMaterials();

		GLuint		mUBOlights = 0;
		GLuint		mUBOMaterials = 0;
		GLuint		mUBOLightTransforms = 0;
		LightCamera	mLightCamera{};
		LConfig		mLightConfiguration = LConfig::Diffuse;
		glm::mat4   mDepthCubeProjMtx{};

		// Bloom effect
	//	struct Bloom {
	//		Framebuffer ping;
	//		Framebuffer pong;
	//	} mBloom;

	public:
		//Some debugging
		unsigned int queryID[4]{};
	};

}

#define RenderMgr (GFX::RenderManager::Instance())
