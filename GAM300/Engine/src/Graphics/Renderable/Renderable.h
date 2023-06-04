#pragma once

#include "Utilities/Math.h"
#include "Objects/Components.h"
#include "ResourceManager/Resource.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Model/Model.h"
#include "Geometry/Geometry.h"

class IRenderable : public IComp
{
public:
	// TODO: Probably make this pure virtual and make renderable an interface 
	// for both static models and animations
	virtual void	Render() = 0;
	void			SetColor(const glm::vec4& _color) { mColor = _color; }
	void			SetColor(const glm::vec3& _color);
	void			SetVisible(bool vis)  { mbVisible = vis; }
	void			SetEditorVisible(bool vis) { mbEditorVisible = vis; }
	bool			IsIndependent() const { return mbZIndependent; }
	bool			IsVisible() const { return mbVisible; }
	bool			IsEditorVisible() const { return mbEditorVisible; }
	bool			IsForward() const { return mbForward; }
	glm::vec4		GetColor() { return mColor; }

#ifdef EDITOR
	bool Edit()	override;
	bool IsIsolated() {return mbIsolate;}
#endif

protected:
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	glm::vec4	mColor = { 1, 1, 1, 1 };
	bool		mbVisible = true;
	bool		mbEditorVisible = true;
	bool		mbIsolate = false;
	bool		mbZIndependent = false;
	bool		mbForward = false;
};

class renderable : public IRenderable
{
public:
	//void Initialize() override;
	//void Shutdown() override;
	void Render() override;

#ifdef EDITOR
	bool Edit()	override;
#endif

	void SetModel(GFX::ModelRes _model) { mModel = _model; mbUseDefaultModel = false; }
	void SetDefaultModel(GFX::Model* _model) { mDefaultModel = _model; mbUseDefaultModel = true;}

	// Same basic gettors
	geometry::aabb GetAABB() const;
	bool           IsEmitter() const { return mbEmitter; }

	IComp* Clone();
	static const unsigned type_id = static_cast<const unsigned>(TypeIds::renderable);


protected:
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

private:

	GFX::ModelRes	mModel = nullptr;
	GFX::Model*		mDefaultModel = &GFX::Model::Sphere;
	bool			mbUseDefaultModel = true;
	bool			mbEmitter = false;
};
