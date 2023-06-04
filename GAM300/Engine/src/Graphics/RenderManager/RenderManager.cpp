#include <GL/glew.h>
#include <GL/GL.h>

#include "Graphics/Graphics.h"
#include "Graphics/ParticleSystem/Particle.h"
#include "Graphics/TextRendering/TextRender.h"
#include "resourcemanager/Resourcemanager.h"

#ifdef EDITOR
#include "../Editor/src/Editor.h"
#endif // EDITOR

#include "System/Scene/SceneSystem.h"
#include "Utilities/Input/Input.h"
#include "Window/Window.h"

namespace GFX {

	// Initialize the point ant the line segment meshes
	Mesh Mesh::Point{ std::vector<glm::vec3>{ {0.0f, 0.0f, 0.0f} },	std::vector<unsigned>{0} };
	Mesh Mesh::Segment{ std::vector<glm::vec3>{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} },
		std::vector<unsigned>{0, 1} };

	void RenderManager::Load()
	{
		assert(MAX_LIGHTS >= MAX_SHADOW_LIGHTS);

		EnableGLErrorCallbacks();

		mBasicShader.CreateShader("./../Resources/Shaders/Color/color.vert",
			"./../Resources/Shaders/Color/color.frag");

		mDepthShader.CreateShader("./../Resources/Shaders/Depth/depth.vert",
			"./../Resources/Shaders/Depth/depth.frag");
		mCubeDepthShader.CreateShader("./../Resources/Shaders/Depth/depth_cube.vert",
			"./../Resources/Shaders/Depth/depth_cube.frag", 
			"./../Resources/Shaders/Depth/depth_cube.geom");
		mCubeMapShader.CreateShader("./../Resources/Shaders/Cubemap/cubemap.vert",
			"./../Resources/Shaders/Cubemap/cubemap.frag");

		mLightShader.CreateShader("./../Resources/Shaders/Light/Forward/light.vert",
			"./../Resources/Shaders/Light/Forward/light.frag");
		mToGBufferShader.CreateShader("./../Resources/Shaders/Light/Deferred/to_gbuffer.vert",
			"./../Resources/Shaders/Light/Deferred/to_gbuffer.frag");
		mGBufferMixerShader.CreateShader("./../Resources/Shaders/Light/Deferred/light.vert",
			"./../Resources/Shaders/Light/Deferred/light.frag");

		mScreenQuadShader.CreateShader("./../Resources/Shaders/Quad/quad.vert",
			"./../Resources/Shaders/Quad/quad.frag");
		mBlurShader.CreateShader("./../Resources/Shaders/Pingpong/blur.vert",
			"./../Resources/Shaders/Pingpong/blur.frag");
		mBlendShader.CreateShader("./../Resources/Shaders/BlendQuad/blend.vert",
			"./../Resources/Shaders/BlendQuad/blend.frag");

		mParticleShader.CreateShader("./../Resources/Shaders/Particle/particle.vert",
			"./../Resources/Shaders/Particle/particle.frag");
		mTextShader.CreateShader("./../Resources/Shaders/Text/text.vert",
			"./../Resources/Shaders/Text/text.frag");

		// Initalize the light sytem (create the Uniform Buffer Objects for it)
		InitializeLighting();
		forward = false;

		// Initialize buffer necessary for instanced rendering particles
		Model::Particle.LoadModel("./../Resources/Meshes/particle.obj", false);
		assert(Model::Particle.mMeshes.size() == 1);
		Model::Particle.mMeshes[0]->GenerateParticleInstanceBuffer();

		// Initialize small handy meshes for debug rendering
		Mesh::Point.UploadMesh();
		Mesh::Segment.UploadMesh();

		Mesh::Point.SetShape(Shape_t::Point);
		Mesh::Segment.SetShape(Shape_t::Line);

		Model::Quad.LoadModel("./../Resources/Meshes/quad.obj", false);
		Model::Cube.LoadModel("./../Resources/Meshes/cube.obj", false);
		Model::Sphere.LoadModel("./../Resources/Meshes/sphere.obj", false);
		Model::Cylinder.LoadModel("./../Resources/Meshes/cylinder.obj", false);

		// Pingpong framebuffers, for blur effect (Part of bloom)
	//	mBloom.ping.GenerateColorBuffer(WindowMgr.GetResolution(), 1);
	//	mBloom.pong.GenerateColorBuffer(WindowMgr.GetResolution(), 1);

		// Face culling
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// Polygon mode
		SetPolygonMode(PolygonMode_t::Solid);
		SetClearColor({ 0.1f, 0.1f, 0.12f, 0.0f });
		SetClearDepth(1.0f);

		// Depth testing    
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CheckGL_Error();
	}

	void RenderManager::Initialize()
	{
		// Compute the shadow maps only once at the beginning (maybe just for static shadow objects)
		auto& spaces = Scene.GetSpaces();
		std::for_each(spaces.begin(), spaces.end(), [&](auto& it)
		{
			// Get the lights in this space and cast them to their proper type (make sure they are the ones supposed to cast shadows)
			auto& lights = it->GetComponentsType<LightComponent>();
			light_container shadow_lights;
			for (auto& light : lights) {
				auto casted_light = reinterpret_cast<LightComponent*>(light);
				if (casted_light && casted_light->mbEmitShadow)
					shadow_lights.push_back(casted_light);
			}

			CleanShadowMaps(it);
			ComputeShadowMaps(it, shadow_lights);
		});
	}

	void RenderManager::Free()
	{
		SetClearColor({ 0.1f, 0.1f, 0.12f, 0.0f });
	}

	void RenderManager::RenderScene()
	{
		if (KeyDown(Key::Control) && KeyTriggered(Key::M))
			forward = !forward;

		UploadMaterials();

		if (forward)	RenderSceneForward();
		else			RenderSceneDeferred();
	}

	void RenderManager::RenderSceneForward()
	{
		// Set the type of configuration when rendring object with a lighting shader
		CheckGL_Error();
		SetLightingConfiguration(LConfig::LightComponentShadows);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		SetClearColor({ 1.0f, 1.0f, 1.0f, 0.0f });
		ClearBuffer();
		CheckGL_Error();

		// To render the scene we render the contents of each space through the cameras in said space
		auto& spaces = Scene.GetSpaces();
		std::for_each(spaces.begin(), spaces.end(), [&](auto& it)
		{
				CheckGL_Error();
			// We need to render the scene from each camera's perspective (usually the output is directed
			// to different buffer, however we do not have that for the moment so it does not make sense
			// to have different cameras). // TODO: Multiple cameras (menus and stuff)
			auto& cameras = it->GetComponentsType<CameraComp>();
			//assert(cameras.size() <= 1);
			std::for_each(cameras.begin(), cameras.end(), [&](auto& cam)
			{		
				auto* casted_cam = reinterpret_cast<CameraComp*>(cam);
				if (!casted_cam->render) return;

				// Get which lights are close to the camera and which of them should emit shadows 
				auto& lights = it->GetComponentsType<LightComponent>();
				light_container basic_lights;
				light_container shadow_lights;

				// Order them based on which one is closest to the player 
				std::sort(lights.begin(), lights.end(), [&cam](const IComp* _lhs, const IComp* _rhs) {
					assert(_lhs != nullptr && _rhs != nullptr);
					float dist_lhs = glm::length2(_lhs->mOwner->mTransform.mPosition
						- cam->mOwner->mTransform.mPosition);
					float dist_rhs = glm::length2(_rhs->mOwner->mTransform.mPosition
						- cam->mOwner->mTransform.mPosition);
					return dist_lhs < dist_rhs;
					});

				// Extract the closest ones 
				for (unsigned i = 0; i < lights.size(); ++i) {
					// Maybe we are already done 
					if (basic_lights.size() >= MAX_LIGHTS) break;

					auto casted_light = static_cast<LightComponent*>(lights[i]);
					auto distance_sqr = glm::length2(casted_light->mOwner->mTransform.mPosition -
						casted_cam->mOwner->mTransform.mPosition);

					// Check if the light is in range (if it is not, since they are sorted, the  
					// rest are not in range either) 
					if (distance_sqr < casted_cam->mLightActionRadius * casted_cam->mLightActionRadius)
						basic_lights.push_back(casted_light);
					else
						break;
				}

				// From the lights we have selected, choose the closest ones with shadows 
				if constexpr (MAX_SHADOW_LIGHTS > 0) {
					for (auto light_it = basic_lights.begin(); light_it != basic_lights.end(); light_it++) {

						// Check if the shadow light is out of reange 
						auto distance_sqr = glm::length2((*light_it)->mOwner->mTransform.mPosition -
							casted_cam->mOwner->mTransform.mPosition);
						if (distance_sqr > casted_cam->mShadowLightRadius * casted_cam->mShadowLightRadius)
							break;

						// If it is in range, check if it emits a shadow 
						if ((*light_it)->mbEmitShadow) {

							// Change this light from basic to shadow 
							shadow_lights.push_back(*light_it);

							// Check if we are done 
							if (shadow_lights.size() == MAX_SHADOW_LIGHTS)
								break;
						}
					}

					// Remove the shadows from the basic lights 
					for (auto shadow : shadow_lights) {
						auto found = std::find(basic_lights.begin(), basic_lights.end(), shadow);
						assert(found != basic_lights.end());
						basic_lights.erase(found);
					}
				}

				// Set the cameras of the space
				CheckGL_Error();
				RenderMgr.SetCamera(casted_cam);
				RenderMgr.SetShaderProgram(Shader_t::Lighting);
				UploadLights(basic_lights, shadow_lights);

				// Set the right framebuffer
				CheckGL_Error();
				assert(casted_cam->GetFramebuffer().GetHandle() != 0);
				glBindFramebuffer(GL_FRAMEBUFFER, casted_cam->GetFramebuffer().GetHandle());
				glBindTexture(GL_TEXTURE_2D, 0);

				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				ClearBuffer();

				// Render cubemap first because yes, probly fix later
				CheckGL_Error();
				RenderCubemap(casted_cam->GetCubemap());

				// Partially visibles
				std::vector<IRenderable*> semi_transparent;
				std::vector<IRenderable*> z_independents;

				// Render all the renderables
				CheckGL_Error();
				auto& renderables = it->GetComponentsType<IRenderable>();
				std::for_each(renderables.begin(), renderables.end(), [&semi_transparent, &z_independents](auto& renderab)
				{
					auto casted_render = static_cast<IRenderable*>(renderab);
					if (casted_render->IsIndependent())
						z_independents.push_back(casted_render);
					else if (casted_render->GetColor().a < 255)
						semi_transparent.push_back(casted_render);
					else
						casted_render->Render();
				});

				// Sort the object with alpha < 1.0f
				CheckGL_Error();
				std::sort(semi_transparent.begin(), semi_transparent.end(),
				[&](const IRenderable* lhs, const IRenderable* rhs) {
					float dist_lhs = glm::length2(casted_cam->mOwner->mTransform.mPosition
						- lhs->mOwner->mTransform.mPosition);
					float dist_rhs = glm::length2(casted_cam->mOwner->mTransform.mPosition
						- rhs->mOwner->mTransform.mPosition);
					return dist_lhs > dist_rhs;
				});

				CheckGL_Error();
				std::for_each(semi_transparent.begin(), semi_transparent.end(), [](auto& renderab) {
					renderab->Render();
				});

				// Particles
				CheckGL_Error();
				auto& particle_systems = it->GetComponentsType<ParticleSystem>();
				std::for_each(particle_systems.begin(), particle_systems.end(),
				[](auto& renderab)
				{
					auto casted_system = static_cast<ParticleSystem*>(renderab);
					casted_system->Render();
				});


				// Objects outside of 
				CheckGL_Error();
				glDisable(GL_DEPTH_TEST);
				std::for_each(z_independents.begin(), z_independents.end(),
				[](auto& renderab)
				{
					auto casted_render = static_cast<IRenderable*>(renderab);
					casted_render->Render();
				});
				glEnable(GL_DEPTH_TEST);


				// Actually show it to screen

				// I disable blending when rendering the quad for the main area because I do not want 
				// anything to blend the first time we render onto the framebuffer, however, I do want
				// the later spaces to blend together with this one
				CheckGL_Error();
				if (it->GetSpaceName() == "MainArea")
				{
						glDisable(GL_BLEND);
						RenderScreenQuad(casted_cam->GetFramebuffer());
						glEnable(GL_BLEND);
				}
				else
				{
					RenderScreenQuad(casted_cam->GetFramebuffer());
				}
				CheckGL_Error();
			});
		});
	}
	void RenderManager::RenderSceneDeferred()
	{
		CheckGL_Error();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ClearBuffer();

		auto& spaces = Scene.GetSpaces();
		for (auto& s : spaces)
		{
			CheckGL_Error();

			// There may be more than one camera, but realistically is going to be a single one
			
			// Remove the ones added from networking
			CameraComp* casted_cam = nullptr;
			auto& cameras = s->GetComponentsType<CameraComp>();
			if (cameras.size() == 0) continue;
			for (auto& c : cameras)
			{
				auto* casted_c = reinterpret_cast<CameraComp*>(c);
				if (casted_c && casted_c->render) {
					casted_cam = casted_c;
					break;
				}
			}
			if (!casted_cam)
				continue;

			CheckGL_Error();
			// Check if we can do deferred or need forward yes or yes
			if (casted_cam->render && s->GetSpaceName() == "MainArea")
			{
				// This is that single camera
				RenderMgr.SetCamera(casted_cam);

				// Set the right framebuffer (and clean it)
				CheckGL_Error();
				assert(casted_cam->GetGBuffer().GetHandle() != 0);
				glBindFramebuffer(GL_FRAMEBUFFER, casted_cam->GetGBuffer().GetHandle());
				CheckGL_Error();

				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				ClearBuffer();

				// Render all the renderables
				RenderMgr.SetShaderProgram(Shader_t::ToGBuffer);
				mbUseShaderFromMesh = false;

				// Store the special objects in separate vectors
				std::vector<IRenderable*> semi_transparent;
				std::vector<IRenderable*> z_independents;
				std::vector<IRenderable*> forwards;

				CheckGL_Error();
				auto& renderables = s->GetComponentsType<IRenderable>();
				glDisable(GL_BLEND);
				for (auto& r : renderables)
				{
					auto casted_render = static_cast<IRenderable*>(r);
					if (casted_render->IsForward())
						forwards.push_back(casted_render);
					else if (casted_render->IsIndependent())
						z_independents.push_back(casted_render);
					else if (casted_render->GetColor().a < 1)
						semi_transparent.push_back(casted_render);
					else
						casted_render->Render();
					CheckGL_Error();
				}
				glEnable(GL_BLEND);
				CheckGL_Error();

				// Sort the object with alpha < 1.0f
				CheckGL_Error();
				std::sort(semi_transparent.begin(), semi_transparent.end(),
				[&](const IRenderable* lhs, const IRenderable* rhs) {
					float dist_lhs = glm::length2(casted_cam->mOwner->mTransform.mPosition
						- lhs->mOwner->mTransform.mPosition);
					float dist_rhs = glm::length2(casted_cam->mOwner->mTransform.mPosition
						- rhs->mOwner->mTransform.mPosition);
					return dist_lhs > dist_rhs;
				});
			
				// Render SemiTransparents
				CheckGL_Error();
				std::for_each(semi_transparent.begin(), semi_transparent.end(), [](auto& renderab) {
					renderab->Render();
				});
			
				// Objects outside of Z-Ordering
				CheckGL_Error();
				glDisable(GL_DEPTH_TEST);
				std::for_each(z_independents.begin(), z_independents.end(),
				[](auto& renderab)
				{
					auto casted_render = static_cast<IRenderable*>(renderab);
					casted_render->Render();
				});
				glEnable(GL_DEPTH_TEST);
				mbUseShaderFromMesh = true;



				// Compute the final image
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				// Render cubemap first because yes, probly fix later
				CheckGL_Error();
				RenderCubemap(casted_cam->GetCubemap());
				glClear(GL_DEPTH_BUFFER_BIT);

				// Apply lighting
				auto& buffer = casted_cam->GetGBuffer();

				auto& lights = s->GetComponentsType<LightComponent>();
				light_container basic_lights;
				for (auto& l : lights)
					basic_lights.push_back((LightComponent*)l);

				// Order them based on which one is closest to the player 
				std::sort(lights.begin(), lights.end(), [&casted_cam](const IComp* _lhs, const IComp* _rhs) {
					assert(_lhs != nullptr && _rhs != nullptr);
					float dist_lhs = glm::length2(_lhs->mOwner->mTransform.mPosition
						- casted_cam->mOwner->mTransform.mPosition);
					float dist_rhs = glm::length2(_rhs->mOwner->mTransform.mPosition
						- casted_cam->mOwner->mTransform.mPosition);
					return dist_lhs < dist_rhs;
				});

				CheckGL_Error();
				RenderMgr.SetShaderProgram(Shader_t::DeferredLighting);
				CheckGL_Error();
				UploadLights(basic_lights, {});

				CheckGL_Error();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buffer.mPositionBuffer);
				glUniform1i(0, 0);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, buffer.mNormalBuffer);
				glUniform1i(1, 1);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, buffer.mSpecularAlbedo);
				glUniform1i(2, 2);
				CheckGL_Error();

				glBindVertexArray(Model::Quad.mMeshes[0]->GetHandle());
				CheckGL_Error();
				glDisable(GL_BLEND);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glEnable(GL_BLEND);

				// Copy the depth buffer
				auto& size = casted_cam->GetGBuffer().mSize;
				glBindFramebuffer(GL_READ_FRAMEBUFFER, casted_cam->GetGBuffer().GetHandle());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
				glBlitFramebuffer(
					0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST
				);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				// Add particles and text
				CheckGL_Error();
				auto& particle_systems = s->GetComponentsType<ParticleSystem>();
				std::for_each(particle_systems.begin(), particle_systems.end(),
				[](auto& renderab)
				{
					auto casted_system = static_cast<ParticleSystem*>(renderab);
					casted_system->Render();
				});
				std::for_each(forwards.begin(), forwards.end(),
				[](auto& renderab)
				{
					auto casted_system = static_cast<IRenderable*>(renderab);
					casted_system->Render();
				});

				CheckGL_Error();
			}
			else if (s->GetSpaceName() != "MainArea")
			{
				CheckGL_Error();
				// Set the cameras of the space
				RenderMgr.SetCamera(casted_cam);

				// Set the right framebuffer
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_DEPTH_BUFFER_BIT);

				// Render all the renderables
				auto& renderables = s->GetComponentsType<IRenderable>();
				std::for_each(renderables.begin(), renderables.end(), [](auto& renderab)
				{
					auto casted_render = static_cast<IRenderable*>(renderab);
					casted_render->Render();
				});
				CheckGL_Error();
			}
		}
	}

#ifdef EDITOR
	void RenderManager::RenderEditor()
	{
		// Set the type of configuration when rendring object with a lighting shader
		SetLightingConfiguration(EditorMgr.mConfig.mLightConfig);

		// To render the scene we render the contents of each space through the cameras in said space
		auto& spaces = Scene.GetSpaces();
		std::for_each(spaces.begin(), spaces.end(), [&](auto& it)
		{
			if (it->IsVisible())
			{
				// Each light produces shadows, we need the information before rendering the 
				// rest of the objects
				//	CleanShadowMaps(it);

				// TODO: FIX THIS HORRIBLE COPY-PASTE
				CameraComp* casted_cam = nullptr;
				auto& cameras = it->GetComponentsType<CameraComp>();
				if (!cameras.empty()) {
					casted_cam = static_cast<CameraComp*>(cameras.front());
				}

				// Get which lights are close to the camera and which of them should emit shadows
				auto& lights = it->GetComponentsType<LightComponent>();
				light_container basic_lights;
				light_container shadow_lights;

				if (casted_cam)
				{
					// Early out = No lights or less lights than maximum
					if (lights.size() < MAX_LIGHTS) {
						for (auto& light : lights) {
							auto casted_light = static_cast<LightComponent*>(light);
							auto distance_sqr = glm::length2(casted_light->mOwner->mTransform.mPosition -
								casted_cam->mOwner->mTransform.mPosition);

							// Check if the light is in range
							if (distance_sqr < casted_cam->mLightActionRadius * casted_cam->mLightActionRadius)
								basic_lights.push_back(casted_light);
						}
					}
					else {
						// Order them based on which one is closest to the player
						std::sort(lights.begin(), lights.end(),
							[&casted_cam](const IComp* _lhs, const IComp* _rhs) {
								assert(_lhs != nullptr && _rhs != nullptr);
								float dist_lhs = glm::length2(_lhs->mOwner->mTransform.mPosition
									- casted_cam->mOwner->mTransform.mPosition);
								float dist_rhs = glm::length2(_rhs->mOwner->mTransform.mPosition
									- casted_cam->mOwner->mTransform.mPosition);
								return dist_lhs < dist_rhs;
							});

						// Extract the closest ones
						for (unsigned i = 0; i < MAX_LIGHTS; ++i) {
							auto casted_light = static_cast<LightComponent*>(lights[i]);
							auto distance_sqr = glm::length2(casted_light->mOwner->mTransform.mPosition -
								casted_cam->mOwner->mTransform.mPosition);

							// Check if the light is in range (if it is not, since they are sorted, the 
							// rest are not in range either)
							if (distance_sqr < casted_cam->mLightActionRadius * casted_cam->mLightActionRadius)
								basic_lights.push_back(casted_light);
							else
								break;
						}
					}

					// From the lights we have selected, choose the closest ones with shadows
					if constexpr (MAX_SHADOW_LIGHTS > 0) {
						for (auto light_it = basic_lights.begin(); light_it != basic_lights.end(); light_it++) {

							// Check if the shadow light is out of reange
							auto distance_sqr = glm::length2((*light_it)->mOwner->mTransform.mPosition -
								casted_cam->mOwner->mTransform.mPosition);
							if (distance_sqr > casted_cam->mShadowLightRadius * casted_cam->mShadowLightRadius)
								break;

							// If it is in range, check if it emits a shadow
							if ((*light_it)->mbEmitShadow) {

								// Change this light from basic to shadow
								shadow_lights.push_back(*light_it);
								light_it = basic_lights.erase(light_it);

								// Check if we are done
								if (shadow_lights.size() == MAX_SHADOW_LIGHTS || light_it == basic_lights.end())
									break;
							}
						}
					}
				}
				else
				{
					for (auto& light : lights) {
						auto casted_light = static_cast<LightComponent*>(light);
						basic_lights.push_back(casted_light);
					}
				}

				// In the editor, we only need to compute the shadows if asked to do so
				if (EditorMgr.mConfig.mLightConfig == LConfig::LightComponentShadows)
					ComputeShadowMaps(it, shadow_lights);

				// Set the cameras of the space
				SetCamera(&EditorMgr.mCamera);
				RenderCubemap(EditorMgr.mCubemap);

				// Upload the inforation about the lights
				if (EditorMgr.mConfig.mLightConfig != LConfig::Diffuse) 
				{
					RenderMgr.SetShaderProgram(Shader_t::Lighting);
					UploadLights(basic_lights, shadow_lights);
				}

				auto& renderables = it->GetComponentsType<IRenderable>();
				std::for_each(renderables.begin(), renderables.end(), [](auto& renderab)
					{
						auto r = static_cast<IRenderable*>(renderab);
						if(r->IsEditorVisible())
							r->Render();
					});
				auto& systems = it->GetComponentsType<ParticleSystem>();
				std::for_each(systems.begin(), systems.end(), [](auto& renderab)
					{
						auto r = static_cast<IRenderable*>(renderab);
						if (r->IsEditorVisible())
							r->Render();
					});
			}
		});
		
	}
	void GFX::RenderManager::ChangeAllObjectsVisibility(bool _visible)
	{
		auto spaces = Scene.GetSpaces();
		for (auto it : spaces)
		{
			auto& renderables = it->GetComponentsType<renderable>();
			std::for_each(renderables.begin(), renderables.end(), [=](auto& renderab)
				{
					if(!static_cast<IRenderable*>(renderab)->IsIsolated())
						static_cast<IRenderable*>(renderab)->SetEditorVisible(_visible);
				});
		}
	}
#endif

	void RenderManager::SetCamera(Camera* _camera)
	{
		mCurrentCamera = _camera;

		// Update the matrices from the camera
		if (mCurrentCamera) mCurrentCamera->Update();
	}

	void RenderManager::SetLigthAsCamera(LightComponent* _light)
	{
		// Make sure
		assert(_light != nullptr);

		// Update the light the camera will be using
		mLightCamera.SetLight(_light);
		SetCamera(&mLightCamera);
	}

	void RenderManager::SetShaderProgram(Shader_t _type)
	{
		mCurrShaderProgramName = _type;

		switch (_type)
		{
		case Shader_t::Basic:
			mCurrentShaderProgram = &mBasicShader;
			break;
		case Shader_t::Depth:
			mCurrentShaderProgram = &mDepthShader;
			break;
		case Shader_t::CubeDepth:
			mCurrentShaderProgram = &mCubeDepthShader;
			break;
		case Shader_t::Cubemap:
			mCurrentShaderProgram = &mCubeMapShader;
			break;
		case Shader_t::Lighting:
			mCurrentShaderProgram = &mLightShader;
			break;
		case Shader_t::ToGBuffer:
			mCurrentShaderProgram = &mToGBufferShader;
			break;
		case Shader_t::DeferredLighting:
			mCurrentShaderProgram = &mGBufferMixerShader;
			break;
		case Shader_t::ScreenQuad:
			mCurrentShaderProgram = &mScreenQuadShader;
			break;
		case Shader_t::Blur:
			mCurrentShaderProgram = &mBlurShader;
			break;
		case Shader_t::Blend:
			mCurrentShaderProgram = &mBlendShader;
			break;
		case Shader_t::Particle:
			mCurrentShaderProgram = &mParticleShader;
			break;
		case Shader_t::Text:
			mCurrentShaderProgram = &mTextShader;
			break;
		default:
			mCurrentShaderProgram = nullptr;
			mCurrShaderProgramName = Shader_t::None;
			break;
		}

		if (mCurrentShaderProgram) 
		{
			glUseProgram(mCurrentShaderProgram->GetHandle());

			if (mCurrentShaderProgram == &mLightShader)
			{
				// More info in SetLightingConfiguration
				auto curr_config = (uint32_t)mLightConfiguration;
				glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &curr_config);
			}
		}
	}

	static int TextureNumber = 0;
	void RenderManager::RenderScreenQuad(GBuffer const& _gb)
	{
		// Rotate between the textures
		if (KeyDown(Key::Control) && KeyTriggered(Key::N)) {
			TextureNumber++;
			if (TextureNumber >= 3) TextureNumber = 0;
		}

		// Select the texture handle
		GLuint to_draw = _gb.mPositionBuffer;

		if (TextureNumber == 1) to_draw = _gb.mNormalBuffer;
		if (TextureNumber == 2) to_draw = _gb.mSpecularAlbedo;

		// Set the default buffer and clean it
		SetShaderProgram(Shader_t::ScreenQuad);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ClearBuffer();

		// Apply the selected texture from the GB to it
		glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Diffuse);
		glBindTexture(GL_TEXTURE_2D, to_draw);
		glUniform1i(0, (u32)TextureUnit::Diffuse);

		// Set the model (A simple quad)
		assert(Model::Quad.mMeshes.size() > 0);
		glBindVertexArray(Model::Quad.mMeshes[0]->GetHandle());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Cleaning
		glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Diffuse);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	void RenderManager::RenderScreenQuad(Framebuffer const& _fb)
	{
		// Set the default buffer and clean it (depth only to allow multiple spaces together)
		SetShaderProgram(Shader_t::ScreenQuad);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
		//RenderCubemap(casted_cam->GetCubemap());
		//ClearDepthBuffer();
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_BLEND);
	
		// Apply the texture from the FB to it
		glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Diffuse);
		glBindTexture(GL_TEXTURE_2D, _fb.GetColorTexture(0));
		glUniform1i(0, (u32)TextureUnit::Diffuse);
	
		// Set the model (A simple quad)
		assert(Model::Quad.mMeshes.size() > 0);
		glBindVertexArray(Model::Quad.mMeshes[0]->GetHandle());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
		// Cleaning
		glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Diffuse);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		//glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	void RenderManager::RenderMesh(const Mesh* _mesh, renderable* _renderable,
		const glm::mat4& _m2w, PolygonMode_t _mode)
	{
		// If there is no mesh there is nothing to do
		if (!_mesh) return;

		// Set the shader program (if the flag use shader from mesh is not set, it means
		// we are in the process of rendering all objects only based in their geometry or
		// something similar. For example, a depth only pass)
		CheckGL_Error();
		if(mbUseShaderFromMesh) 
			SetShaderProgram(_mesh->GetShaderProgram());

		// Select the resources
		glBindVertexArray(_mesh->GetHandle());

		// Set the uniforms, both the mtx and color
		CheckGL_Error();
		mCurrentShaderProgram->SetUniforms(_mesh, _m2w, mCurrentCamera, _renderable);

		// Wireframe / Fill / Points
		CheckGL_Error();
		auto prev_mode = mPolyMode;
		if (_mode != mPolyMode)
			SetPolygonMode(_mode);

		// Actual render call. Either using indices or not
		CheckGL_Error();
		glDrawElements(_mesh->GetShape(), _mesh->GetSize(), GL_UNSIGNED_INT, 0);

		if (prev_mode != mPolyMode)
			SetPolygonMode(prev_mode);

		// Clean the state
		CheckGL_Error();
		glBindVertexArray(0);
		if (mbUseShaderFromMesh)
			glUseProgram(0);
	}
	void RenderManager::RenderMesh(const Mesh* _mesh, const glm::vec4& _color,
		const glm::mat4& _m2w, PolygonMode_t _mode)
	{
		// If there is no mesh there is nothing to do
		if (!_mesh) return;

		SetShaderProgram(Shader_t::Basic);

		// Select the resources
		glBindVertexArray(_mesh->GetHandle());

		// Set the uniforms, both the mtx and color
		glm::mat4 MVP = mCurrentCamera ? mCurrentCamera->GetW2Proj() * _m2w : glm::mat4{ 1.0f } *_m2w;

		glUniformMatrix4fv((u32)UniformLoc::Model2ProjMtx, 1, GL_FALSE, &MVP[0][0]);
		glUniform4fv((u32)UniformLoc::Color, 1, &_color[0]);

		// Wireframe / Fill / Points
		auto prev_mode = mPolyMode;
		if (_mode != mPolyMode)
			SetPolygonMode(_mode);

		// Actual render call. Either using indices or not
		glDrawElements(_mesh->GetShape(), _mesh->GetSize(), GL_UNSIGNED_INT, 0);

		if (prev_mode != mPolyMode)
			SetPolygonMode(prev_mode);

		// Clean the state
		glBindVertexArray(0);
		if (mbUseShaderFromMesh)
			glUseProgram(0);
	}

	void RenderManager::RenderModel(const Model* _model, renderable* _renderable,
		const glm::mat4& _m2w, PolygonMode_t _mode)
	{
		if (!_model) return;

		for (unsigned int i = 0; i < _model->mMeshes.size(); i++)
			RenderMgr.RenderMesh(_model->mMeshes[i].get(), _renderable, _m2w, _mode);
	}
	void RenderManager::RenderModel(const Model* _model, const glm::vec4& _color,
		const glm::mat4& _m2w, PolygonMode_t _mode)
	{
		if (!_model) return;

		for (unsigned int i = 0; i < _model->mMeshes.size(); i++)
			RenderMgr.RenderMesh(_model->mMeshes[i].get(), _color, _m2w, _mode);
	}

	void GFX::RenderManager::RenderText(FontRes mFont, glm::vec2 mScale, glm::vec3 mOffset, 
		glm::vec4 mColor, Transform const& mTransform, bool mbHorizontalBill, bool mbVerticalBill,
		TextComponent::Alignment mAlignment, std::string const& msg)
	{
		if (!mFont) mFont = ResourceMgr.GetResource<GFX::Font>("./../Resources/Font/xd.ttf");
		if (!mFont) return;

		if (!mFont->get() || !mFont->get()->mProperlyLoaded) return;

		// Set shader program and uniforms (transformation matrix)
		auto prev_shader = RenderMgr.GetCurrShaderProgram();
		RenderMgr.SetShaderProgram(GFX::Shader_t::Text);

		auto m2w = mTransform.ModelToWorld();
		auto s2m = glm::translate(mOffset) * glm::scale(glm::vec3{ mScale.x, mScale.y, 1.0f });
		auto cam = RenderMgr.GetCurrentCamera();

		glm::mat4 WorldProj = cam ? cam->GetW2Proj() : glm::mat4{ 1.0f };
		glm::mat4 WorldView = cam ? cam->GetW2Cam() : glm::mat4{ 1.0f };
		glUniformMatrix4fv(0, 1, GL_FALSE, &s2m[0][0]);
		glUniformMatrix4fv(4, 1, GL_FALSE, &WorldView[0][0]);
		glUniformMatrix4fv(5, 1, GL_FALSE, &m2w[0][0]);
		glUniformMatrix4fv(6, 1, GL_FALSE, &WorldProj[0][0]);

		glBindVertexArray(mFont->get()->mVAO);
		glActiveTexture(GL_TEXTURE0 + (u32)GFX::TextureUnit::Diffuse);
		glUniform1i(2, (u32)GFX::TextureUnit::Diffuse);
		glUniform4fv(3, 1, &mColor.x);

		// Billboarding
		glUniform1i(7, (int)mbHorizontalBill);
		glUniform1i(8, (int)mbVerticalBill);


		// Render the message
		
		// This will be the position to place the first character (will be different 
		// based on alignment)
		float x = 0.0f, y = 0.0f;

		// IF the alignment is not exactly left, we need to now the length of the whole string
		if (mAlignment == TextComponent::Alignment::Centered) {

			unsigned total_advance = 0;
			for (auto& c : msg) {
				auto& ch = mFont->get()->mCharacters.find((unsigned)c)->second;
				total_advance += (ch.mAdvance >> 6);
			}

			x = total_advance * 0.5f;
			x *= -1.0f;
		}

		for (auto& c : msg) {

			// Get the texture of the character
			auto& ch = mFont->get()->mCharacters.find((unsigned)c)->second;
			glBindTexture(GL_TEXTURE_2D, ch.mTextureID);

			// Set the secondary transformation (from character to string space, which is a step before model to world)
			// The position is the center of the glyph
			float xpos = x + ch.mBearing.x + ch.mSize.x * 0.5f;
			float ypos = y + ch.mBearing.y - ch.mSize.y * 0.5f;
			x += (ch.mAdvance >> 6);    // For some reason the advance is 64 times the original size

			glm::mat4 char_to_string = glm::translate(glm::vec3{ xpos, ypos, 0.0f }) *
				glm::scale(glm::vec3{ ch.mSize.x, ch.mSize.y, 1.0f });
			glUniformMatrix4fv(1, 1, GL_FALSE, &char_to_string[0][0]);

			// Call drawing
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mFont->get()->mEBO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		RenderMgr.SetShaderProgram(prev_shader);
	}

	void RenderManager::RenderCubemap(CubemapRes _cubemap)
	{
		// Sanity check
		if (!_cubemap || !_cubemap->get()) return;

		// We want to draw a single cube, from inside it, with the cubemap shader
		mbUseShaderFromMesh = false;
		glCullFace(GL_FRONT);
		SetShaderProgram(Shader_t::Cubemap);

		// Set the cubemap texture
		_cubemap->get()->Bind(TextureUnit::Diffuse);
		glUniform1i((u32)CubemapLoc::CubemapShader, (u32)TextureUnit::Diffuse);

		renderable jej;
		RenderModel(&Model::Cube, &jej, glm::mat4(1.0f),
			PolygonMode_t::Solid);

		// Set the configurations to what they were before
		glCullFace(GL_BACK);
		mbUseShaderFromMesh = true;
	}

	void RenderManager::ReloadShaders()
	{
		mBasicShader.ReloadShader();
		mScreenQuadShader.ReloadShader();
		mLightShader.ReloadShader();
		mToGBufferShader.ReloadShader();
		mGBufferMixerShader.ReloadShader();
		mBlurShader.ReloadShader();
		mBlendShader.ReloadShader();
		mParticleShader.ReloadShader();
		mTextShader.ReloadShader();
	}

	void GFX::RenderManager::SetDefaultBuffer() const
	{
		auto& res = WindowMgr.GetResolution();

		glViewport(0, 0, res.x, res.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderManager::ClearBuffer() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderManager::ClearColorBuffer() const
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void RenderManager::ClearDepthBuffer() const
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void RenderManager::SetClearDepth(float _depth)
	{
		mClearDepth = _depth;
		glClearDepth(_depth);
	}

	void RenderManager::SetClearColor(const glm::vec4& _color)
	{
		mClearColor = _color;
		glClearColor(_color.r, _color.g, _color.b, _color.a);
	}

	void RenderManager::SetPolygonMode(GFX::PolygonMode_t _mode)
	{
		mPolyMode = _mode;
		switch (mPolyMode)
		{
		case PolygonMode_t::PointCloud:	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
		case PolygonMode_t::Wireframe:	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
		case PolygonMode_t::Solid:			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
		}
	}
	void RenderManager::SetLightingConfiguration(LConfig _config)
	{
		// Subroutines must be set in an array order (at slot 0 the index of the function we want to
		// use for the subroutine uniform with location 0). In our case we are using a single
		// subroutine uniform (so it is at loc 0) and the value that we pass is directly the current
		// configuration since we are making sure that the indices at the frag shader match the values of the
		// enum class
		mLightConfiguration = _config;
	}
}
