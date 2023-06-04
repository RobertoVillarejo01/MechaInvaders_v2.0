#include "Renderable.h"

#include "Objects/GameObject.h"
#include "Graphics/Graphics.h"
#include "resourcemanager/Resourcemanager.h"
#include "System/Scene/SceneSystem.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR


using namespace GFX;

void renderable::Render()
{
	// Make sure we have an owner
	if (!mOwner || (!mbUseDefaultModel && mModel == nullptr) || !mbVisible) return;

	// Actual render
    CheckGL_Error();
	RenderMgr.RenderModel(mbUseDefaultModel ? mDefaultModel : mModel->get(), this, 
    mOwner->mTransform.ModelToWorld(), RenderMgr.GetPolygonMode());
    CheckGL_Error();
}

#ifdef EDITOR

bool renderable::Edit()
{
  // Maybe not usefull now, but later on it will be needed (for Ctrl+Z etc.)
  bool changed = false;

  IRenderable::Edit();
  ImGui::Separator();

  // Edit the color
  changed = ImGui::ColorEdit4("Color", &mColor[0]);

  ImGui::Checkbox("Emitter", &mbEmitter);
  // Change the model (get the name that should be displayed) (Optimizable)
  std::string current_name;
  if (mbUseDefaultModel)
  {
    if      (mDefaultModel == &Model::Quad) current_name = " - Quad - ";
    else if (mDefaultModel == &Model::Cube) current_name = " - Cube - ";
    else if (mDefaultModel == &Model::Sphere) current_name = " - Sphere - ";
    else if (mDefaultModel == &Model::Cylinder) current_name = " - Cylinder - ";
  }
  else
  {
    if (mModel) current_name = mModel->getName();
    else current_name = " * NO MODEL * ";
  }

  // Get all the models
	auto& models = ResourceMgr.GetResourcesOfType<Model>();
  if (ImGui::BeginCombo(" Texture", current_name.c_str()))
  {
    if (ImGui::Selectable(" - Quad - "))
    {
      mDefaultModel = &Model::Quad;
      mbUseDefaultModel = true;
      changed = true;
    }
    if (ImGui::Selectable(" - Cube - "))
    {
      mDefaultModel = &Model::Cube;
      mbUseDefaultModel = true;
      changed = true;
    }
    if (ImGui::Selectable(" - Sphere - "))
    {
      mDefaultModel = &Model::Sphere;
      mbUseDefaultModel = true;
      changed = true;
    }
    if (ImGui::Selectable(" - Cylinder - "))
    {
        mDefaultModel = &Model::Cylinder;
        mbUseDefaultModel = true;
        changed = true;
    }

    for (auto& model : models)
    {
      if (ImGui::Selectable(model.first.data()))
      {
        mModel = std::reinterpret_pointer_cast<TResource<Model>>(model.second);
        mbUseDefaultModel = false;

        changed = true;
      }
    }
    ImGui::EndCombo();
  }

    return changed;
}

geometry::aabb renderable::GetAABB() const
{
  if (mbUseDefaultModel)
    return mDefaultModel->GetAABB();
  else {
    if (mModel && mModel->get())
      return mModel->get()->GetAABB();
    else
      return geometry::aabb{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };
  }
    
}

#endif

void renderable::ToJson(nlohmann::json& j) const
{
    IRenderable::ToJson(j);

  j["Color"] << mColor;
  j["mbUseDefaultModel"] << mbUseDefaultModel;
  j["mbEmitter"] << mbEmitter;

  if (mbUseDefaultModel)
  {
    if      (mDefaultModel == &Model::Quad)   j["Model"] << std::string("Quad");
    else if (mDefaultModel == &Model::Cube)   j["Model"] << std::string("Cube");
    else if (mDefaultModel == &Model::Sphere) j["Model"] << std::string("Sphere");
    else if (mDefaultModel == &Model::Cylinder) j["Model"] << std::string("Cylinder");
  }
  else
  {
    if (mModel) j["Model"] << mModel->getName();
    else        j["Model"] << std::string("");
  }
}

void renderable::FromJson(nlohmann::json& j)
{
    IRenderable::FromJson(j);

	j["Color"] >> mColor;
    j["mbUseDefaultModel"] >> mbUseDefaultModel;
    if (j.find("mbEmitter") != j.end())
        j["mbEmitter"] >> mbEmitter;

  if (mbUseDefaultModel)
  {
    auto modelName = j["Model"].get<std::string>();

    if      (modelName == "Quad")   mDefaultModel = &Model::Quad;
    else if (modelName == "Cube")   mDefaultModel = &Model::Cube;
    else if (modelName == "Sphere") mDefaultModel = &Model::Sphere;
    else if (modelName == "Cylinder") mDefaultModel = &Model::Cylinder;
  }
  else
  {
    std::string modelPath; j["Model"] >> modelPath;
    if (modelPath != "") SetModel(ResourceMgr.GetResource<Model>(modelPath.c_str()));
  }
}

IComp* renderable::Clone()
{
	return Scene.CreateComp<renderable>(mOwner->GetSpace(), this);
}

#ifdef EDITOR
bool IRenderable::Edit()
{
    ImGui::Checkbox("Visible", &mbVisible);
    ImGui::Checkbox("Visible in Editor", &mbEditorVisible);
    ImGui::Checkbox("No Z-Order", &mbZIndependent);
    ImGui::Checkbox("Forward", &mbForward);
    if (ImGui::Checkbox("Isolate object", &mbIsolate))
    {
        RenderMgr.ChangeAllObjectsVisibility(!mbIsolate);
        //mbVisible = true;
    }

    return false;
}
#endif
void IRenderable::ToJson(nlohmann::json& j) const
{
    j["mbVisible"]      << mbVisible;
    j["mbZIndependent"] << mbZIndependent;
    j["mbForward"]      << mbForward;
}

void IRenderable::FromJson(nlohmann::json& j)
{
    if (j.find("mbVisible") != j.end())
    {
        j["mbVisible"]      >> mbVisible;
        j["mbZIndependent"] >> mbZIndependent;
    }
    if (j.find("mbForward") != j.end())
    {
        j["mbForward"] >> mbForward;
    }
}

void	IRenderable::SetColor(const glm::vec3& _color)
{
    mColor.x = _color.x;
    mColor.y = _color.y;
    mColor.z = _color.z;
}
