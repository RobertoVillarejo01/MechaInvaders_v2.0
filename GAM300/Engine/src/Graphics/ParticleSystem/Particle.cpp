#include <GL/glew.h>
#include <GL/GL.h>

#include "Particle.h"
#include "Graphics/RenderManager/RenderManager.h"
#include "Utilities/FrameRateController/FrameRateController.h"
#include "resourcemanager/Resourcemanager.h"
#include "System/Scene/SceneSystem.h"

#ifdef EDITOR
#include "../Editor/src/Editor.h"
#endif

template <typename T>
T Lerp(T const& a, T const& b, float t)
{
	return glm::mix(a, b, t);
}

template <int D>		/// Templatized dimension
glm::vec<D, float> RandomVector(glm::vec<D, float> const(&a)[2])
{
	glm::vec<D, float> result;
	for (unsigned i = 0; i < D; i++) {
		result[i] = Lerp(a[0][i], a[1][i], glm::linearRand(0.0f, 1.0f));
	}

	return result;
}

template <typename T>
inline json& operator<<(json& j, T const (&val)[2])
{
	j["Begin"]	<< val[0];
	j["End"]	<< val[1];
	return j;
}
template <typename T>
inline void operator>>(const json& j, T(&val)[2])
{
	j["Begin"]	>> val[0];
	j["End"]	>> val[1];
}

void ParticleSystem::Initialize()
{
	// Generate the memory
	GenerateBuffers(mMaxParticles);
	Reset();
}

void ParticleSystem::Shutdown()
{
	// Delete memory
	DeleteBuffers();
}

IComp* ParticleSystem::Clone()
{
	return Scene.CreateComp<ParticleSystem>(mOwner->GetSpace(), this);
}

#ifdef EDITOR
bool ParticleSystem::Edit()
{
	// Even more basic than basic properties
	if (ImGui::TreeNode("IRenderable")) {
		IRenderable::Edit();
		ImGui::TreePop();
	}

	// Basic properties
	if (ImGui::TreeNode("BasicProperties")) {

		// Particle delay and lifetime
		ImGui::Checkbox("InGameActive", &mbActive);
		ImGui::DragFloat("EmitRate", &mEmitRate, 0.05f, 0.0f, MAX_PARTICLE_COUNT);
		ImGui::DragFloat2("LifeTimeRange", &mLifeTimeRange[0], 0.05f);

		// Max particle count
		ImGui::DragInt("MaxParticles", &mTempMaxParticles, 1, 0, MAX_PARTICLE_COUNT);
		if (mTempMaxParticles > MAX_PARTICLE_COUNT)
			mTempMaxParticles = MAX_PARTICLE_COUNT;
		if (ImGui::Button("RegenerateBuffers")) {
			mMaxParticles = mTempMaxParticles;
			GenerateBuffers(mTempMaxParticles);
		}

		ImGui::Checkbox("Circle", &circle);
		ImGui::Checkbox("Rotate the circle 90 degrees", &circle_turn90);
		ImGui::DragFloat("Radious of the circle", &radius);

		{	// Texture selection

			std::string name;
			if (!mTexture || !mTexture->get())
				name = "-- NONE --";
			else {
				name = mTexture->getName();
			}

			auto& textures = ResourceMgr.GetResourcesOfType<GFX::Texture2D>();
			if (ImGui::BeginCombo("Texture", name.c_str()))
			{
				for (auto& tex : textures)
				{
					if (ImGui::Selectable(tex.first.data()))
					{
						mTexture = std::reinterpret_pointer_cast<TResource<GFX::Texture2D>>(tex.second);
					}
				}
				ImGui::EndCombo();
			}
		}

		ImGui::TreePop();
	}

	// Particle movement
	if (ImGui::TreeNode("Movement")) {

		ImGui::DragFloat3("Velocity1", &mVelocityRange[0][0], 0.05f);
		ImGui::DragFloat3("Velocity2", &mVelocityRange[1][0], 0.05f);

		ImGui::TreePop();
	}

	// Scale
	if (ImGui::TreeNode("Scale")) {

		ImGui::PushID("Scale");
		ImGui::DragFloat3("InitialRandom1", &mScaleRange[0][0].x, 0.05f);
		ImGui::SameLine();
		ImGui::PushID("A");
		ImGui::Checkbox("", &mbRandomInitialScale);
		ImGui::PopID();
		if (mbRandomInitialScale)
			ImGui::DragFloat3("InitialRandom2", &mScaleRange[0][1].x, 0.05f);
		else
			mScaleRange[0][1] = mScaleRange[0][0];

		ImGui::Checkbox("LerpScale", &mbLerpScale);
		if (mbLerpScale) {
			ImGui::DragFloat3("FinalRandom1", &mScaleRange[1][0].x, 0.05f);
			ImGui::SameLine();
			ImGui::PushID("B");
			ImGui::Checkbox("", &mbRandomFinalScale);
			ImGui::PopID();
			if (mbRandomFinalScale)
				ImGui::DragFloat3("FinalRandom2", &mScaleRange[1][1].x, 0.05f);
			else
				mScaleRange[1][1] = mScaleRange[1][0];
		}
		ImGui::PopID();

		ImGui::TreePop();
	}

	// Color
	if (ImGui::TreeNode("Color")) {

		ImGui::PushID("Color");
		ImGui::ColorEdit4("InitialRandom1", &mColorRange[0][0].x);
		ImGui::SameLine();
		ImGui::PushID("A");
		ImGui::Checkbox("", &mbRandomInitialColor);
		ImGui::PopID();
		if (mbRandomInitialColor)
			ImGui::ColorEdit4("InitialRandom2", &mColorRange[0][1].x);
		else
			mColorRange[0][1] = mColorRange[0][0];

		ImGui::Checkbox("LerpColor", &mbLerpColor);
		if (mbLerpColor) {
			ImGui::ColorEdit4("FinalRandom1", &mColorRange[1][0].x);
			ImGui::SameLine();
			ImGui::PushID("B");
			ImGui::Checkbox("", &mbRandomFinalColor);
			ImGui::PopID();
			if (mbRandomFinalColor)
				ImGui::ColorEdit4("FinalRandom2", &mColorRange[1][1].x);
			mColorRange[1][1] = mColorRange[1][0];
		}

		ImGui::PopID();
		ImGui::TreePop();
	}

	// Is the emitter active? Should it?
	ImGui::Separator();
	ImGui::Checkbox("EditorActive", &mbEditorActive);

	return false;
}
#endif

void ParticleSystem::Reset()
{
	mAliveParticles = 0;
	mAccomulatedTime = 0.0f;
}

void ParticleSystem::SetActive(bool _state)
{
	mbActive = _state;
}

void ParticleSystem::ToJson(nlohmann::json& j) const
{
	IRenderable::ToJson(j);

	// Essential informations about the particle system
	j["mbActive"]		<< mbActive;
	j["mMaxParticles"]	<< mMaxParticles;
	j["mEmitRate"]		<< mEmitRate;
	j["mLifeTimeRange"] << mLifeTimeRange;

#ifdef EDITOR
	if (mTempMaxParticles != mMaxParticles) {
		std::cerr << "WARNING: \nEstabas usando un numero de particulas distinto del"
			<< " que estas guardando. \nProbablemente no has dado regenerar buffers"
			<< " despue de cambiar el limite de particulas\n";
		std::cerr << "MaxParticles (las que se guardan) = " << mMaxParticles << std::endl;
		std::cerr << "TempMaxParticles (solo en editor) = " << mTempMaxParticles << std::endl;
	}
#endif

	j["circle"] << circle;
	j["circle_turn90"] << circle_turn90;
	j["radious"] << radius;

	// Some basic interpolations
	j["mVelocityRange"] << mVelocityRange;
	j["mScaleRange"]	<< mScaleRange;
	j["mColorRange"]	<< mColorRange;

	j["mbLerpScale"]			<< mbLerpScale;
	j["mbRandomInitialScale"]	<< mbRandomInitialScale;
	j["mbRandomFinalScale"]		<< mbRandomFinalScale;

	j["mbLerpColor"]			<< mbLerpColor;
	j["mbRandomInitialColor"]	<< mbRandomInitialColor;
	j["mbRandomFinalColor"]		<< mbRandomFinalColor;

	// Texture
	if (mTexture && mTexture->get())
		j["mTexture"]				<< mTexture->getName();
}

void ParticleSystem::FromJson(nlohmann::json& j)
{
	IRenderable::FromJson(j);

	// Basic properties
	j["mbActive"]		>> mbActive;
	j["mEmitRate"]		>> mEmitRate;
	j["mLifeTimeRange"] >> mLifeTimeRange;
	j["mMaxParticles"]	>> mMaxParticles;
#ifdef EDITOR
	mTempMaxParticles = mMaxParticles;
#endif // EDITOR

	// Particle generation orientation
	if (j.find("circle") != j.end())
		j["circle"] >> circle;
	if (j.find("circle_turn90") != j.end())
		j["circle_turn90"] >> circle_turn90;
	if (j.find("radious") != j.end())
		j["radious"] >> radius;

	// Some basic interpolations
	j["mVelocityRange"] >> mVelocityRange;
	j["mScaleRange"]	>> mScaleRange;
	j["mColorRange"]	>> mColorRange;

	j["mbLerpScale"]			>> mbLerpScale;
	j["mbRandomInitialScale"]	>> mbRandomInitialScale;
	j["mbRandomFinalScale"]		>> mbRandomFinalScale;

	j["mbLerpColor"]			>> mbLerpColor;
	j["mbRandomInitialColor"]	>> mbRandomInitialColor;
	j["mbRandomFinalColor"]		>> mbRandomFinalColor;

	// Get the texture
	if (j.find("mTexture") != j.end()) 
	{
		std::string mTexName; j["mTexture"] >> mTexName;
		mTexture = ResourceMgr.GetResource<GFX::Texture2D>(mTexName.data());
		if (!mTexture || !mTexture->get()) {
			std::cerr << "Could not load font : " << mTexName << std::endl;
		}
	}
	else {
		mTexture = nullptr;
	}
}

void ParticleSystem::GenerateBuffers(unsigned size)
{
	DeleteBuffers();
	Reset();

	assert(size < MAX_PARTICLE_COUNT);
	mParticles = new Particle[size];
	mParticleProperties = new ParticleProperties[size];

	for (unsigned i = 0; i < size; i++) {
		mParticleProperties[i].mParticle = &mParticles[i];
	}
}

void ParticleSystem::DeleteBuffers()
{
	delete[] mParticles;			mParticles = nullptr;
	delete[] mParticleProperties;	mParticleProperties = nullptr;
}

void ParticleSystem::Update()
{
	// If paused we do not want the time waiting to accomulate
	if (!mbActive) return;
#ifdef EDITOR
	if (EditorMgr.mbInEditor && !mbEditorActive) return;
#endif

	// Get the delta time
	auto dt = FRC.GetFrameTime();

	// Make sure the buffers are properly created
	if (!mParticles || !mParticleProperties) {
		std::cerr << "Error ParticleSystem::Update. No buffers generated" << std::endl;
		return;
	}

	// Update all particles
	std::queue<unsigned> mDeadParticles;
	for (int i = 0; i < mAliveParticles; ++i)
	{
		mParticleProperties[i].Update(dt);
		if (mParticleProperties[i].mbDead) {
			mDeadParticles.push(i);
		}
	}

	// Check if there is an existing emit rate
	if (glm::abs(mEmitRate) > cEpsilon) {

		// Get the time between particles
		float EmitDelay = 1.0f / (60.0f * mEmitRate);

		// Emit particles if needed
		mAccomulatedTime += dt;
		float time_passed = 0.0f;
		while (mAccomulatedTime > EmitDelay)
		{
			// Generate the particle
			auto particle_idx = EmitParticle(mDeadParticles);
			if (particle_idx == -1) {
				break;
			}
			assert(particle_idx >= 0 && particle_idx < mMaxParticles);
			auto& particle = mParticleProperties[particle_idx];

			// Update it based on the ellapsed time (probably not very important)
			particle.Update(time_passed);

			time_passed += EmitDelay;
			mAccomulatedTime -= EmitDelay;
		}
	}

	// Make sure alive particles are continous (particle N is influenced by 
	// particle properties N, it has an internal pointer to modify it directly)
	while (mDeadParticles.size() > 0)
	{
		// Get the last dead particle property
		auto& dead_idx = mDeadParticles.front(); mDeadParticles.pop();
		auto& prop = mParticleProperties[dead_idx];

		// Find last particle alive
		if ((int)dead_idx < mAliveParticles - 1)
		{
			unsigned last_alive = mAliveParticles - 1;
			while (last_alive > dead_idx && mParticleProperties[last_alive].mbDead)
				last_alive--;

			// Check if we have found a valid alive particle, if not there are no 
			// more swaps that can be done
			if (last_alive > dead_idx)
			{
				mAliveParticles -= (int)(mDeadParticles.size() + 1);
				break;
			}

			// Swap the last alive particle and the last dead
			std::swap(mParticles[last_alive], mParticles[dead_idx]);
			std::swap(mParticleProperties[last_alive], mParticleProperties[dead_idx]);
			std::swap(mParticleProperties[last_alive].mParticle,
				mParticleProperties[dead_idx].mParticle);
		}

		// Notify that there is one less particle
		mAliveParticles--;
	}
}

int ParticleSystem::EmitParticle(std::queue<unsigned>& _dead_particles)
{
	// Make sure there is enough space
	assert(mAliveParticles <= mMaxParticles);
	if (_dead_particles.empty() && mAliveParticles == mMaxParticles)
		return -1;

	// If there are dead particles, create the new particles where there are free spaces
	if (!_dead_particles.empty()) {
		auto particle_idx = _dead_particles.front(); _dead_particles.pop();
		CreateParticle(particle_idx);
		return particle_idx;
	}
	// Otherwise, just append them to the end
	else {
		CreateParticle(mAliveParticles);
		mAliveParticles++;
		return mAliveParticles - 1;
	}
}

void ParticleSystem::CreateParticle(unsigned idx)
{
	// Get the references to the particle we are creating
	auto& particle = mParticles[idx];
	auto& properties = mParticleProperties[idx];

	// Set the initial position, scale, lifetime and color
	particle.mPos					= mOwner->mTransform.mPosition;
	properties.mEllapsedLifeTime	= 0.0f;
	properties.mTotalLifeTime		= Lerp(mLifeTimeRange[0], mLifeTimeRange[1], glm::linearRand(0.0f, 1.0f));
	if (circle)
	{
		float angle = (rand() % 10000) / 10000.0f * 360.0f;
		if (circle_turn90)
		{
			glm::vec3 point = { mOwner->mTransform.mPosition.x + cos(angle), mOwner->mTransform.mPosition.y + sin(angle), 0 };
			properties.mVelocity = point - mOwner->mTransform.mPosition;
			properties.mVelocity.z = 0;
		}
		else
		{
			glm::vec3 point = { 0, mOwner->mTransform.mPosition.y + sin(angle), mOwner->mTransform.mPosition.z + cos(angle) };
			properties.mVelocity = point - mOwner->mTransform.mPosition;
			properties.mVelocity.x = 0;
		}
		properties.mVelocity *= radius;
	}
	else
		properties.mVelocity			= RandomVector(mVelocityRange);
	properties.mbLerpColor			= mbLerpColor;
	properties.mbLerpScale			= mbLerpScale;
	properties.mbDead				= false;

	if (mbLerpColor) {
		properties.mColorRange[0]	= RandomVector(mColorRange[0]);
		properties.mColorRange[1]	= RandomVector(mColorRange[1]);
	}
	else 
		particle.mColor				= RandomVector(mColorRange[0]);

	if (mbLerpScale) {
		properties.mScaleRange[0]	= RandomVector(mScaleRange[0]);
		properties.mScaleRange[1]	= RandomVector(mScaleRange[1]);
	}
	else 
		particle.mScale				= RandomVector(mScaleRange[0]);
}

void ParticleSystem::Render()
{
	// Maybe pause?
	Update();

	// Render
	if (!mbVisible) return;

	// Set shader program and uniforms (transformation matrix)
	RenderMgr.SetShaderProgram(GFX::Shader_t::Particle);

	auto m2w = mOwner->mTransform.ModelToWorld();
	auto cam = RenderMgr.GetCurrentCamera();

	glm::mat4 WorldProj = cam ? cam->GetW2Proj() : glm::mat4{ 1.0f };
	glm::mat4 WorldView = cam ? cam->GetW2Cam() : glm::mat4{ 1.0f };

	glUniformMatrix4fv(0, 1, GL_FALSE, &WorldProj[0][0]);
	glUniformMatrix4fv(2, 1, GL_FALSE, &WorldView[0][0]);

	// IF we have a texture selected use it, otherwise, use the default texture
	if (!mTexture || !mTexture->get()) {
		auto texture = ResourceMgr.GetResource<GFX::Texture2D>
			("./../Resources/Textures/Particles/particle.png");
		texture->get()->Bind(GFX::TextureUnit::Diffuse);
	}
	else 
	{
		mTexture->get()->Bind(GFX::TextureUnit::Diffuse);
	}
	glUniform1i(1, (u32)GFX::TextureUnit::Diffuse);

	glBindVertexArray(GFX::Model::Particle.mMeshes[0]->GetHandle());

	// Bind buffer with the instance data (and update said data)
	glBindBuffer(GL_ARRAY_BUFFER, GFX::Model::Particle.mMeshes[0]->GetInstanceBuffer());
	glBufferSubData(GL_ARRAY_BUFFER, 0, mAliveParticles * sizeof(Particle), mParticles);

	glDepthMask(GL_FALSE);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, mAliveParticles);
	glDepthMask(GL_TRUE);
}

void ParticleSystem::ParticleProperties::Update(float _dt)
{
	// Check life time
	mEllapsedLifeTime += _dt;
	if (mEllapsedLifeTime > mTotalLifeTime) {
		mbDead = true;
		return;
	}

	// Various interpolations
	float t = mEllapsedLifeTime / mTotalLifeTime;

	mParticle->mPos += mVelocity * _dt;
	if (mbLerpColor) mParticle->mColor = Lerp(mColorRange[0], mColorRange[1], t);
	if (mbLerpScale) mParticle->mScale = Lerp(mScaleRange[0], mScaleRange[1], t);
}
