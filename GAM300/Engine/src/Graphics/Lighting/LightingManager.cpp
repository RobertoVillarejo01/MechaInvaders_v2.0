#include <GL/glew.h>
#include <GL/GL.h>
#include "Graphics/Lighting/Light.h"
#include "System/Scene/SceneSystem.h"
#include "resourcemanager/Resourcemanager.h"

#ifdef EDITOR
#include "../Editor/src/Editor.h"
#endif // EDITOR

static constexpr glm::vec3 directions[GFX::CUBE_FACES]
{
	glm::vec3(1,  0,  0),
	glm::vec3(-1, 0,  0),
	glm::vec3(0,  1,  0),
	glm::vec3(0, -1,  0),
	glm::vec3(0,  0,  1),
	glm::vec3(0,  0, -1)
};
static constexpr glm::vec3 upVectors[GFX::CUBE_FACES]
{
	glm::vec3(0, -1,  0),
	glm::vec3(0, -1,  0),
	glm::vec3(0,  0,  1),
	glm::vec3(0,  0, -1),
	glm::vec3(0, -1,  0),
	glm::vec3(0, -1,  0)
};

namespace GFX {

	void RenderManager::InitializeLighting()
	{
		// Delete if already generated
		if (mUBOlights != 0)			glDeleteBuffers(1, &mUBOlights);
		if (mUBOLightTransforms != 0)	glDeleteBuffers(1, &mUBOLightTransforms);
		if (mUBOMaterials != 0)			glDeleteBuffers(1, &mUBOMaterials);

		// Generate
		CheckGL_Error();
		glGenBuffers(1, &mUBOlights);
		glBindBuffer(GL_UNIFORM_BUFFER, mUBOlights);
		glBufferData(GL_UNIFORM_BUFFER, MAX_LIGHTS * sizeof(Light) + sizeof(int), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		CheckGL_Error();
		glGenBuffers(1, &mUBOLightTransforms);
		glBindBuffer(GL_UNIFORM_BUFFER, mUBOLightTransforms);
		glBufferData(GL_UNIFORM_BUFFER, MAX_LIGHTS * sizeof(glm::mat4) + sizeof(int), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		CheckGL_Error();
		glGenBuffers(1, &mUBOMaterials);
		glBindBuffer(GL_UNIFORM_BUFFER, mUBOMaterials);
		glBufferData(GL_UNIFORM_BUFFER, MAX_MATERIALS * sizeof(GFX::Material) + sizeof(int), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		// Regular lighting
		CheckGL_Error();
		RenderMgr.SetShaderProgram(Shader_t::Lighting);
		unsigned int index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(), "Lights");
		glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), index, (unsigned)GFX::UBOBindPoint::Lights);
		glBindBufferBase(GL_UNIFORM_BUFFER, (unsigned)GFX::UBOBindPoint::Lights, mUBOlights);

		CheckGL_Error();
		index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(), "LightTransforms");
		glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), index, (unsigned)GFX::UBOBindPoint::LightTransforms);
		glBindBufferBase(GL_UNIFORM_BUFFER, (unsigned)GFX::UBOBindPoint::LightTransforms, mUBOLightTransforms);

		CheckGL_Error();
		index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(), "Materials");
		glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), index, (unsigned)GFX::UBOBindPoint::Materials);
		glBindBufferBase(GL_UNIFORM_BUFFER, (unsigned)GFX::UBOBindPoint::Materials, mUBOMaterials);
	

		// Deferred lighting
		CheckGL_Error();
		RenderMgr.SetShaderProgram(Shader_t::DeferredLighting);
		index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(), "Lights");
		glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), index, (unsigned)GFX::UBOBindPoint::Lights);
		glBindBufferBase(GL_UNIFORM_BUFFER, (unsigned)GFX::UBOBindPoint::Lights, mUBOlights);

	//	CheckGL_Error();
	//	index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(), "LightTransforms");
	//	glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), index, (unsigned)GFX::UBOBindPoint::LightTransforms);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, (unsigned)GFX::UBOBindPoint::LightTransforms, mUBOLightTransforms);

		CheckGL_Error();
		index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(), "Materials");
		glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), index, (unsigned)GFX::UBOBindPoint::Materials);
		glBindBufferBase(GL_UNIFORM_BUFFER, (unsigned)GFX::UBOBindPoint::Materials, mUBOMaterials);
		CheckGL_Error();
	}

	void RenderManager::CleanShadowMaps(Space* _space)
	{
		// First of all, we need to get the information about the geometry in the direction of 
		// each light, so that shadow maps can work
		auto& lights = Scene.GetComponentsType<LightComponent>(_space);

		// If there are no lights there is nothing to do
		if (lights.size() == 0) return;
		std::for_each(lights.begin(), lights.end(), [&](auto& light)
		{
			// Get the casted version 
			auto casted_light = reinterpret_cast<LightComponent*>(light);

			// If the light does not emit shadows this buffer is not used
			// if (casted_light->mbShadowMapComputed == false) return;
			// casted_light->mbShadowMapComputed = false;

			glViewport(0, 0, 1024, 1024); // TODO: arreglar esta cochinada
			if (casted_light->GetFramebuffer().IsGenerated() == 0) casted_light->Initialize();
			glBindFramebuffer(GL_FRAMEBUFFER, casted_light->GetFramebuffer().GetHandle());
			glClear(GL_DEPTH_BUFFER_BIT);
		});

		// Reset to default framebuffer
		RenderMgr.SetDefaultBuffer();
	}

	/**
	 * Render the scene from the given lights' point of view. 
	 * The result is stored in their framebuffer
	 */
	void RenderManager::ComputeShadowMaps(Space* _space, 
		light_container const& _lights)
	{
		// If there are no lights there is nothing to do
		if (_lights.size() == 0) return;

		// Set culling to front faces, if we render the shadows based on the back faces looks better
		glCullFace(GL_FRONT);

		std::for_each(_lights.begin(), _lights.end(), [&](auto& light)
		{
			// If the shadow map is already computed, there is no need to do it again
			// if (light->mbShadowMapComputed == true) return;
			// light->mbShadowMapComputed = true;

			// Set the viewport we will be drawing onto (make sure the buffer has already been generated)
			glViewport(0, 0, 1024, 1024); // TODO: arreglar esta cochinada
			if (light->GetFramebuffer().IsGenerated() == 0) light->Initialize();
			glBindFramebuffer(GL_FRAMEBUFFER, light->GetFramebuffer().GetHandle());

			// Set the correct shader
			if (light->mLight.light_type == (unsigned)Light::Type::POINT)
			{
				// We are rendering the scene onto a cube
				RenderMgr.SetShaderProgram(Shader_t::CubeDepth);

				// Upload basic light data
				glUniform1f((u32)DepthCubeLoc::FarPlane, light->mLight.far);
				glUniform3fv((u32)DepthCubeLoc::LightPosition, 1, 
					&light->mOwner->mTransform.mPosition[0]);

				// The projection amtrix is common to all 6 directions
				float aspect_ratio = light->GetFramebuffer().GetAspectRatio();
				glm::mat4 proj = glm::perspective(glm::radians(90.0f), aspect_ratio,
					light->mLight.near, light->mLight.far);

				// The view matrix, however, is not
				for (int i = 0; i < 6; ++i)
				{
					glm::mat4 w2c = glm::lookAt(light->mOwner->mTransform.mPosition,
						light->mOwner->mTransform.mPosition + directions[i], upVectors[i]);
					glm::mat4 final_mat = proj * w2c;

					glUniformMatrix4fv((u32)DepthCubeLoc::ShadowMatrices + i, 1, false, &final_mat[0][0]);
				}
			}
			else
			{
				// Simple projection of the scene onto a single texture
				RenderMgr.SetShaderProgram(Shader_t::Depth);
			}

			// Set the light as a camera (TODO: Different lights require different matrices. For 
			// example directional would use orthogonal projection)
			RenderMgr.SetLigthAsCamera(light);

			// Render all the objects using that config (TODO: Maybe need a space partition or smth so that
			// this does not balloon aout of control)
			mbUseShaderFromMesh = false;

			auto& renderables = _space->GetComponentsType<IRenderable>();
			std::for_each(renderables.begin(), renderables.end(), [](auto& renderab)
			{
				static_cast<IRenderable*>(renderab)->Render();
			});
			if (light->mLight.light_type == (unsigned)Light::Type::POINT)
				glGenerateTextureMipmap(light->GetFramebuffer().GetDepthTexture());

			mbUseShaderFromMesh = true;
			glUseProgram(0);
		});

		// Reset to default framebuffer
		RenderMgr.SetDefaultBuffer();

		// Reset to back face culling
		glCullFace(GL_BACK);
	}

	/**
	 * Upload the light information to 2 different buffer objects, one containing the light 
	 * information (position, direction, diffuse, specular...) and another containing transformations
	 * needed for shadow mapping.
	 */
	void RenderManager::UploadLights(light_container const& basic_lights, 
		light_container const& shadow_lights)
	{
//		std::cout << "In" << std::endl;

		// Check if we are trying to use more lights than we can
		CheckGL_Error();
		if (basic_lights.size() + shadow_lights.size() > MAX_LIGHTS)
		{
			std::cerr << "Trying to upload more lights (" << basic_lights.size() + 
				shadow_lights.size() << ") than allowed (" << MAX_LIGHTS << ")" << std::endl;
		}
		else if (shadow_lights.size() > MAX_SHADOW_LIGHTS) 
		{
			std::cerr << "Trying to upload more shadow casting lights (" << shadow_lights.size()
				<< ") than allowed (" << MAX_SHADOW_LIGHTS << ")" << std::endl;
		}

		// Get the sizes, just in case
		auto light_count = glm::min((uint32_t)(basic_lights.size() +
			shadow_lights.size()), MAX_LIGHTS);
		auto shadow_count = glm::min((uint32_t)shadow_lights.size(), MAX_LIGHTS);

		// Upload the information for each light
		unsigned light_idx = 0;
		unsigned shadow_idx = 0;

		// Basic lights info
		CheckGL_Error();
		for (auto& light : basic_lights)
		{
			// Update some variables for the light (First of all, transform both the position and 
			// direction to the right space)
			light->mLight.position = glm::vec3{ RenderMgr.mCurrentCamera->GetW2Cam() *
				glm::vec4(light->mOwner->mTransform.mPosition, 1.0f) };
			light->mLight.actual_dir = glm::vec3{ RenderMgr.mCurrentCamera->GetW2Cam() *
				glm::vec4(light->mLight.direction, 0.0f) };
			light->mLight.world_position = light->mOwner->mTransform.mPosition;
			light->mLight.texture_unit = -1;

			// Set the binding point
			glBindBuffer(GL_UNIFORM_BUFFER, mUBOlights);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Light) * light_idx, sizeof(Light), &light->mLight);

			// Update the index
			light_idx++;
		}

		// Set the shadow casting lights' information
		CheckGL_Error();
		for (auto& light : shadow_lights)
		{
			// Update some variables for the light (First of all, transform both the position and 
			// direction to the right space)
			light->mLight.position = glm::vec3{ RenderMgr.mCurrentCamera->GetW2Cam() *
				glm::vec4(light->mOwner->mTransform.mPosition, 1.0f) };
			light->mLight.actual_dir = glm::vec3{ RenderMgr.mCurrentCamera->GetW2Cam() *
				glm::vec4(light->mLight.direction, 0.0f) };
			light->mLight.world_position = light->mOwner->mTransform.mPosition;
			light->mLight.texture_unit = shadow_idx;

			// Set the binding point
			CheckGL_Error();
			glBindBuffer(GL_UNIFORM_BUFFER, mUBOlights);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Light) * light_idx, sizeof(Light), &light->mLight);
		
			// Shadow mapping transformations
			CheckGL_Error();
			auto light_world_2_proj = light->mCam2Proj * light->mWorld2Cam;
			glBindBuffer(GL_UNIFORM_BUFFER, mUBOLightTransforms);
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * shadow_idx, sizeof(glm::mat4),
				&light_world_2_proj[0][0]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			// Set the texture for the shadows
			CheckGL_Error();
			glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Shadow + shadow_idx);
			light->GetFramebuffer().BindTexture();

			// Depending on the texture type the Shadow texture unit will be linked with a 
			// 2d sampler or cube sampler
			CheckGL_Error();
			if (light->GetFramebuffer().GetTextureType() == GFX::Framebuffer::TextureType::Texture2D) {
				glUniform1i((u32)UniformLoc::ShadowMapSampler + shadow_idx, (u32)TextureUnit::Shadow + shadow_idx);
				glUniform1i((u32)UniformLoc::ShadowCubeSampler + shadow_idx, 0);
			}
			else {
				glUniform1i((u32)UniformLoc::ShadowCubeSampler + shadow_idx, (u32)TextureUnit::Shadow + shadow_idx);
				glUniform1i((u32)UniformLoc::ShadowMapSampler + shadow_idx, 0);
			}
			CheckGL_Error();

			// Update the indices
			shadow_idx++;
			light_idx++;
		}

		// Upload the number of lights we will be using
		CheckGL_Error();
		glBindBuffer(GL_UNIFORM_BUFFER, mUBOlights);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Light) * MAX_LIGHTS, sizeof(int), &light_idx);

		CheckGL_Error();
//		if (shadow_idx > 0) {
//			std::cout << "This?" << std::endl;
			glBindBuffer(GL_UNIFORM_BUFFER, mUBOLightTransforms);
			CheckGL_Error();
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * MAX_SHADOW_LIGHTS, sizeof(int), &shadow_idx);
			CheckGL_Error();
//		}
//		std::cout << "That" << std::endl;
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		CheckGL_Error();
//		std::cout << "Out" << std::endl;
	}

	void RenderManager::UploadMaterials()
	{
		// Give an identifier to each material / mesh
		CheckGL_Error();
		unsigned meshID = 0;
		glBindBuffer(GL_UNIFORM_BUFFER, mUBOMaterials);
		CheckGL_Error();

		// Iterate through each of the meshes and get their materials
		auto& models = ResourceMgr.GetResourcesOfType<Model>();
		for (auto& m_pair : models)
		{
			// Cast the pointer to be a Model resource so we are able to access its data
			auto model = std::reinterpret_pointer_cast<TResource<Model>>(m_pair.second)->get();
			if (!model) continue;

			Material m;
			m.mAmbient = { 1.0, 1.5, 1.99 };
			m.mDiffuse = { 2.0, 2.5, 2.99 };
			m.mSpecular = { 3.0, 3.5, 3.99 };
			m.mShininess = 345.678f;

			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Material) * meshID, sizeof(Material), &m);
			meshID++;

			// Iterate through the meshes
			for (auto& mesh : model->mMeshes)
			{
				// Upload the data
				CheckGL_Error();
				auto& mat = mesh->GetMaterial();
				glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Material) * meshID, sizeof(Material), &mat);
				CheckGL_Error();

				// Set the ID
				mesh->mMaterialID = meshID++;
			}
		}



		CheckGL_Error();
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Material) * MAX_MATERIALS, sizeof(int), &meshID);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		CheckGL_Error();
	}
}
