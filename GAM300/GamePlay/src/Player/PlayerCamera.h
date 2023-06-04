#pragma once
#include "LogicSystem/Logic.h"

class CameraComp;

class PlayerCamera : public ILogic
{
public:
    void   Initialize() override;
    void   Update() override;

#ifdef EDITOR
    bool   Edit() override;
#endif

    IComp* Clone() override;
    void   UpdateVectors();
    void   GetRotation();

    void ToJson(nlohmann::json& j) const override;
    void FromJson(nlohmann::json& j) override;

    CameraComp* get_camera() { return mCamera; }

    glm::vec2 angle = { 0.0f, -90.0f };

    bool base_player = false;

private:
    bool mbStaticCamera = false;
    glm::vec2 mMousePos = {};
    
    CameraComp* mCamera = nullptr;
};