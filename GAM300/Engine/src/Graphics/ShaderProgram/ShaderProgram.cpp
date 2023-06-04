#include "ShaderProgram.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <GL/GL.h>

#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/RenderManager/RenderManager.h"

#pragma region // HELPER FUNCTIONS //

std::string LoadFileData(std::string filename)
{
	// Load the file and pass its content to the buffer
	std::ifstream file(filename);

	if (file.is_open())
	{
		std::stringstream buffer;

		buffer << file.rdbuf();

		// Return a string with the contents of the file
		return buffer.str();
	}
	else
	{
		std::cerr << "Error loading shader shader. Cannot open file: "
			<< filename << std::endl;
		return std::string();
	}
}

GLuint CompileShader(GLenum eShaderType, const std::string& strShaderFile)
{
	// Parse the actual file and create the respective string
	std::string shader_data = LoadFileData(strShaderFile);
	if (shader_data == "") return -1;

	// Get the handle and compile the shader (We are already given the type it is)
	GLuint shader = glCreateShader(eShaderType);
	const char* strFileData = shader_data.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	// Check for any possible errors
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		fprintf(stderr, "Compile failure in shader (%s):\n%s\n", strShaderFile.c_str(), strInfoLog);
		delete[] strInfoLog;
	}

	return shader;
}


#pragma endregion

namespace GFX {

	ShaderProgram::~ShaderProgram()
	{
		DestroyShader();
	}

	void ShaderProgram::CreateShader(std::string vert_shader_path, std::string frag_shader_path,
		std::string geom_shader_path)
	{
		// Save the path to the files of the shader, we will need them later if we want
		// to reload them dynamically
		mVtxShaderPath = vert_shader_path;
		mFragShaderPath = frag_shader_path;
		mGeomShaderPath = geom_shader_path;

		// Compile the two different shaders
		auto mVtxShader = CompileShader(GL_VERTEX_SHADER, mVtxShaderPath);
		auto mFragShader = CompileShader(GL_FRAGMENT_SHADER, mFragShaderPath);
		GLuint mGeomShader = -1;
		if (geom_shader_path != "") 
			mGeomShader = CompileShader(GL_GEOMETRY_SHADER, mGeomShaderPath);

		// Get the handle
		mProgramHandle = glCreateProgram();

		// Attach both shaders and link them
		glAttachShader(mProgramHandle, mVtxShader);
		glAttachShader(mProgramHandle, mFragShader);
		if (mGeomShader != (unsigned)(-1))
			glAttachShader(mProgramHandle, mGeomShader);
		glLinkProgram(mProgramHandle);

		// Check for errors
		GLint status;
		glGetProgramiv(mProgramHandle, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint infoLogLength;
			glGetProgramiv(mProgramHandle, GL_INFO_LOG_LENGTH, &infoLogLength);

			GLchar* strInfoLog = new GLchar[size_t(infoLogLength + 1)];
			glGetProgramInfoLog(mProgramHandle, infoLogLength, NULL, strInfoLog);

			fprintf(stderr, "%s\n", strInfoLog);
			delete[] strInfoLog;
		}

		// Clean up the "partial" shaders after linking them since they are no longer needed
		glDeleteShader(mVtxShader);
		glDeleteShader(mFragShader);
		if (mGeomShader != (unsigned)(-1))
			glDeleteShader(mGeomShader);

		// Save the locations of the uniforms
		glUseProgram(mProgramHandle);
		SaveLocations();
	}

	void ShaderProgram::ReloadShader()
	{
		// Destroy the currently bound program and upload the new one
		DestroyShader();
		CreateShader(mVtxShaderPath, mFragShaderPath);
	}

	void ShaderProgram::DestroyShader()
	{
		glDeleteProgram(mProgramHandle);
	}


	/***********************       Shader Program Specializations       ***********************/

	void LightShaderProgram::SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, 
		const Camera* _cam, renderable* _renderable) const
	{
		// Basic properties
		CheckGL_Error();
		glm::mat4 MVP = _cam ? _cam->GetW2Proj() * _m2w : glm::mat4{ 1.0f } *_m2w;

		glUniformMatrix4fv((u32)UniformLoc::Model2ProjMtx, 1, GL_FALSE, &MVP[0][0]);
		glUniform4fv((u32)UniformLoc::Color, 1, &_renderable->GetColor()[0]);

		glUniform1i((u32)UniformLoc::bEmitter, _renderable->IsEmitter());

		// Assume we will be using the diffuse from the material (not the texture) we 
		// may be proven wrong later
		CheckGL_Error();
		glUniform1i((u32)UniformLoc::UseTextures, false);
		glUniform1i(14, _mesh->mMaterialID);

		// Set textures (TODO: Arreglar esto)
		glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Diffuse);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i((u32)UniformLoc::DiffuseSampler, (u32)TextureUnit::Diffuse);
		for (int i = 0; i < (int)_mesh->GetTextures().size(); ++i)
		{
			const auto& texture = _mesh->GetTextures()[i]->get();
			if (texture && texture->mType == Texture2D::TextureType::Diffuse)
			{
				// Set texturing for diffuse color on
				glUniform1i((u32)UniformLoc::UseTextures, true);

				texture->Bind(TextureUnit::Diffuse);
				glUniform1i((u32)UniformLoc::DiffuseSampler, (u32)TextureUnit::Diffuse);

				break; // TODO: Right now we are only using one texture for rendering if at all
			}
		}

		// Useful for shadow mapping (So that using the light)
		CheckGL_Error();
		glUniformMatrix4fv((u32)UniformLoc::Model2WorldMtx, 1, GL_FALSE, &_m2w[0][0]);

		// Other matrices (for lighting)
		if (_cam)
		{
			auto m2v = _cam->GetW2Cam() * _m2w;
			auto normals_m2v = glm::transpose(glm::inverse(m2v));
			//auto normals_m2v = glm::mat4(1.0f);
			glUniformMatrix4fv((u32)UniformLoc::Model2ViewMtx, 1, GL_FALSE, &m2v[0][0]);
			glUniformMatrix4fv((u32)UniformLoc::M2VNormalsMtx, 1, GL_FALSE, &normals_m2v[0][0]);
		}

		CheckGL_Error();
	}

	//void LightShaderProgram::CreateShader(std::string vert_shader_path, 
	//	std::string frag_shader_path)
	//{
	//	ShaderProgram:CreateShader(vert_shader_path, frag_shader_path);
	//
	//	glGenBuffers(1, &mUBOlights);
	//	glBindBuffer(GL_UNIFORM_BUFFER, mUBOlights);
	//	glBufferData(GL_UNIFORM_BUFFER, MAX_LIGHTS * sizeof(Light) + sizeof(int), NULL, GL_STATIC_DRAW);
	//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//
	//	unsigned int lights_index = glGetUniformBlockIndex(RenderMgr.mCurrentShaderProgram->GetHandle(),
	//		"Lights");
	//	glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), lights_index, 2);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboMatrices);
	//}
	//
	//void LightShaderProgram::ReloadShader()
	//{
	//	ShaderProgram::ReloadShader();
	//
	//	glUseProgram(mProgramHandle);
	//	unsigned int lights_index = glGetUniformBlockIndex(mProgramHandle, "Lights");
	//	glUniformBlockBinding(RenderMgr.mCurrentShaderProgram->GetHandle(), lights_index, 2);
	//	glBindBufferBase(GL_UNIFORM_BUFFER, 2, mUBOlights);
	//}

	void LightShaderProgram::SaveLocations()
	{
		ambient_loc = glGetUniformLocation(mProgramHandle, "material.ambient");
		diffuse_loc = glGetUniformLocation(mProgramHandle, "material.diffuse");
		specular_loc = glGetUniformLocation(mProgramHandle, "material.specular");
		emissive_loc = glGetUniformLocation(mProgramHandle, "material.emissive");

		opacity_loc = glGetUniformLocation(mProgramHandle, "material.opacity");
		shininess_loc = glGetUniformLocation(mProgramHandle, "material.shininess");
		refractive_loc = glGetUniformLocation(mProgramHandle, "material.refractive");
	}




	void BasicShaderProgram::SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w,
		const Camera* _cam, renderable* _renderable) const
	{
		glm::mat4 MVP = _cam ? _cam->GetW2Proj() * _m2w : glm::mat4{ 1.0f } *_m2w;

		glUniformMatrix4fv((u32)UniformLoc::Model2ProjMtx, 1, GL_FALSE, &MVP[0][0]);
		glUniform4fv((u32)UniformLoc::Color, 1, &_renderable->GetColor()[0]);
		glUniform1i(2, _renderable->IsEmitter());
	}




	void DepthCubeShaderProgram::SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, 
		const Camera* _cam, renderable* _renderable) const
	{
		glUniformMatrix4fv((u32)DepthCubeLoc::Model2WorldMtx, 1, GL_FALSE, &_m2w[0][0]);
	}





	void CubemapShaderProgram::SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, 
		const Camera* _cam, renderable* _renderable) const
	{
		// We only want the orientation of the camera, not the displacement
		glm::mat4 MVP = glm::mat4{ 1.0f };
		if (_cam) {
			MVP = glm::mat4(glm::mat3(_cam->GetW2Cam()));
			MVP = _cam->GetCam2Proj() * MVP;
		}

		glUniformMatrix4fv((u32)CubemapLoc::Model2WorldMtx, 1, GL_FALSE, &MVP[0][0]);
	}

    void ParticleShaderProgram::SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, 
		const Camera* _cam, renderable* _renderable) const
    {
		glm::mat4 MVP = _cam ? _cam->GetW2Cam() * _m2w : glm::mat4{ 1.0f } *_m2w;
		glUniformMatrix4fv((u32)UniformLoc::Model2ProjMtx, 1, GL_FALSE, &MVP[0][0]);
		//glUniform1i(1, 0);
    }

    void ToGBufferShaderProgram::SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, const Camera* _cam, renderable* _renderable) const
    {
		// Basic properties
		CheckGL_Error();
		glm::mat4 MVP = _cam ? _cam->GetW2Proj() * _m2w : glm::mat4{ 1.0f } *_m2w;

		glUniformMatrix4fv((u32)UniformLoc::Model2ProjMtx, 1, GL_FALSE, &MVP[0][0]);

		// Assume we will be using the diffuse from the material (not the texture) we 
		// may be proven wrong later
		glUniform1i((u32)UniformLoc::UseTextures, false);
		glUniform4fv((u32)UniformLoc::Color, 1, &_renderable->GetColor()[0]);
		glUniform1i(7, false);
		glUniform1i(14, (int)_mesh->mMaterialID);

		// Set textures (TODO: Arreglar esto)
		glActiveTexture(GL_TEXTURE0 + (u32)TextureUnit::Diffuse);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i((u32)UniformLoc::DiffuseSampler, (u32)TextureUnit::Diffuse);
		CheckGL_Error();
		for (int i = 0; i < (int)_mesh->GetTextures().size(); ++i)
		{
			const auto& texture = _mesh->GetTextures()[i]->get();
			if (texture && texture->mType == Texture2D::TextureType::Diffuse)
			{
				// Set texturing for diffuse color on
				glUniform1i((u32)UniformLoc::UseTextures, true);

				texture->Bind(TextureUnit::Diffuse);
				glUniform1i((u32)UniformLoc::DiffuseSampler, (u32)TextureUnit::Diffuse);
			}
			else if (texture && texture->mType == Texture2D::TextureType::Normal)
			{
				// Set texturing for diffuse color on
				glUniform1i(7, true);

				texture->Bind(TextureUnit::Normal);
				glUniform1i(8, (u32)TextureUnit::Normal);
			}
		}

		// Useful for shadow mapping (So that using the light)
		glUniformMatrix4fv((u32)UniformLoc::Model2WorldMtx, 1, GL_FALSE, &_m2w[0][0]);

		// Other matrices (for lighting)
		if (_cam)
		{
			auto m2v = _cam->GetW2Cam() * _m2w;
			auto normals_m2v = glm::transpose(glm::inverse(m2v));
			//auto normals_m2v = glm::mat4(1.0f);
			glUniformMatrix4fv((u32)UniformLoc::Model2ViewMtx, 1, GL_FALSE, &m2v[0][0]);
			glUniformMatrix4fv((u32)UniformLoc::M2VNormalsMtx, 1, GL_FALSE, &normals_m2v[0][0]);
		}
    }

}
