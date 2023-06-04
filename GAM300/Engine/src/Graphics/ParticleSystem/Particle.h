#pragma once

#include <queue>
#include "Graphics/Texture/Texture.h"
#include "Graphics/Renderable/Renderable.h"

static const unsigned DEFAULT_PARTICLE_COUNT = 100;
static const unsigned MAX_PARTICLE_COUNT = 1 << 11;

class ParticleSystem : public IRenderable
{
public:
	void Initialize() override;
	void Render() override;
	void Shutdown() override;
	IComp* Clone();

#ifdef EDITOR
	bool Edit()	override;
#endif

	// Particle system functionality
	void Reset();
	void SetActive(bool _state);
	void SetEmitRate(float _rate) { mEmitRate = _rate; }

	struct Particle
	{
		glm::vec3 mPos;
		glm::vec3 mScale;
		glm::vec4 mColor;
	};

protected:
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

private:

	struct ParticleProperties 
	{
		float mEllapsedLifeTime = 0.0f;
		float mTotalLifeTime = 0.0f;

		// Behaviour
		glm::vec3 mVelocity{};
		glm::vec3 mScaleRange[2]{};
		glm::vec4 mColorRange[2]{};
		bool mbLerpScale;
		bool mbLerpColor;

		// Basic info
		bool mbDead = false;
		Particle* mParticle = 0;

		// Utility
		void Update(float _dt);
	};

	void GenerateBuffers(unsigned size);
	void DeleteBuffers();

	void Update();
	int  EmitParticle(std::queue<unsigned>& _dead_particles);
	void CreateParticle(unsigned idx);

	// Essential informations about the particle system
	int		 mMaxParticles = DEFAULT_PARTICLE_COUNT;
	int		 mAliveParticles = 0;

	// Other general information
	GFX::TextureRes	mTexture = nullptr;

	// The actual mmeory pool for particles in this system (size = mMaxParticles)
	Particle* mParticles = nullptr;
	ParticleProperties* mParticleProperties = nullptr;

	// Particle generation and death
	float   mEmitRate = 5.0f;			
	float   mAccomulatedTime = 0.0f;		
	float	mLifeTimeRange[2] = {0.5f, 2.0f};

	// Some basic interpolations
	glm::vec3 mVelocityRange[2] = { {-5.0f, 3.0f, -5.0f},{5.0f, 3.0f, 5.0f} };
	glm::vec3 mScaleRange[2][2] = { {{},{}}, { glm::vec3{10.0f}, glm::vec3{20.0f} } };
	glm::vec4 mColorRange[2][2] = { {{},{}}, { glm::vec4{1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} } };
	bool mbLerpScale = true; bool mbRandomInitialScale = false; bool mbRandomFinalScale = false;
	bool mbLerpColor = true; bool mbRandomInitialColor = false; bool mbRandomFinalColor = false;

	bool mbActive = true;

	bool circle = false;
	bool circle_turn90 = false;
	float radius = 0.0f;

#ifdef EDITOR
	bool mbEditorActive = false;
	int  mTempMaxParticles = DEFAULT_PARTICLE_COUNT;
#endif
};
