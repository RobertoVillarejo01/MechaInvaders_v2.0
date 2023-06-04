#include "../Objects/GameObject.h"
#include "Transform.h"


void operator<<(nlohmann::json& j, const Transform& _rhs)
{
	j["Position"] << _rhs.mPosition;
	j["Offset"]   << _rhs.mOffset;
	j["Scale"]    << _rhs.mScale;
	j["Forward"]  << _rhs.mViewVector;
	j["Up"]       << _rhs.mUpVect;
	j["Right"]    << _rhs.mRightVector;
#ifdef EDITOR
	j["Orientation"] << _rhs.mOrientation;
#endif
}

void operator>>(nlohmann::json& j, Transform& _rhs)
{
	if (j.find("Position") != j.end())
		j["Position"] >> _rhs.mPosition;
	if (j.find("Offset") != j.end())
		j["Offset"]   >> _rhs.mOffset;
	if (j.find("Scale") != j.end())
		j["Scale"]    >> _rhs.mScale;

	if (j.find("Forward") != j.end())
		j["Forward"]  >> _rhs.mViewVector;
	if (j.find("Up") != j.end())
		j["Up"]       >> _rhs.mUpVect;
	if (j.find("Right") != j.end())
		j["Right"]	  >> _rhs.mRightVector;
#ifdef EDITOR
	if (j.find("Orientation") != j.end())
		j["Orientation"] >> _rhs.mOrientation;
#endif
}

void Transform::RotateAround(glm::vec3 _vector, float _ang_deg)
{
	auto mtx = glm::rotate(glm::radians(_ang_deg), _vector);

	mViewVector   = glm::vec3(mtx * glm::vec4(mViewVector, 0.0f));
	mUpVect       = glm::vec3(mtx * glm::vec4(mUpVect, 0.0f));
	mRightVector  = glm::vec3(mtx * glm::vec4(mRightVector, 0.0f));
}

void Transform::RotateAround(glm::vec3 _pos, glm::vec3 _vector, float _ang_deg)
{
	//compute the rotation matrix around the vector
	glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), glm::radians(_ang_deg), _vector);

	//rotate the direction vectors of the object
	mViewVector = rotMtx * glm::vec4(mViewVector, 0.0f);
	mRightVector = rotMtx * glm::vec4(mRightVector, 0.0f);
	mUpVect = rotMtx * glm::vec4(mUpVect, 0.0f);

	//vector that will represent the b vector of the transformation
	glm::vec3 translation = glm::vec4(_pos, 1.0f) - (rotMtx * glm::vec4(_pos, 1.0f));

	//set that vector into the matrix tranformation
	glm::mat4 transMtx  = glm::translate(glm::mat4(1.0f), translation);
	rotMtx = transMtx * rotMtx;

	//rotate the object
	mPosition = rotMtx * glm::vec4(mPosition, 1.0f);

	//normalize the direction vectors of the object
	mViewVector = glm::normalize(mViewVector);
	mRightVector = glm::normalize(mRightVector);
	mUpVect = glm::normalize(mUpVect);
}

glm::mat4 Transform::ModelToWorld() const
{
	auto translate = glm::translate(mPosition);
	auto scale     = glm::scale(mScale);
	auto rotate    = GetRotMtx();
	return translate * rotate * scale;
}

glm::mat4 Transform::TransRotMtx(GameObject* parent) const
{
	auto translate = glm::translate(mPosition);
	auto rotate = GetRotMtx();
	if (parent)
		return parent->mTransform.TransRotMtx(parent->GetParent()) * translate * rotate;
	return translate * rotate;
}

glm::mat4 Transform::ModelToWorldOffset(const glm::mat4& parent_mtx) const
{
	auto translate = glm::translate(glm::vec3(parent_mtx * glm::vec4(mOffset, 1.0f)));
	auto scale = glm::scale(mScale);

	return translate * scale;
}




glm::mat4 Transform::GetRotMtx() const
{
	glm::mat4 mRot(mRightVector.x, mRightVector.y, mRightVector.z, 0.0f,
				   mUpVect.x, mUpVect.y, mUpVect.z,    0.0f,
				   -mViewVector.x, -mViewVector.y, -mViewVector.z, 0.0f,
				   0.0f, 0.0f, 0.0f, 1.0f);

	return mRot;
}


void Transform::LookAt(glm::vec3 target)
{
	mViewVector  = glm::normalize(target - mPosition);
	mRightVector = glm::normalize(glm::cross(mViewVector, glm::vec3(0.0f, 1.0f, 0.0f)));
	mUpVect      = glm::cross(mRightVector, mViewVector);
}


#ifdef EDITOR
void Transform::UpdateRotationVectors()
{
	mViewVector = glm::vec3(sin(glm::radians(mOrientation.y)),
		-sin(glm::radians(mOrientation.x)) * cos(glm::radians(mOrientation.y)),
		cos(glm::radians(mOrientation.x)) * cos(glm::radians(mOrientation.y)));
	mUpVect = glm::vec3(-cos(glm::radians(mOrientation.y)) * sin(glm::radians(mOrientation.z)),
		-sin(glm::radians(mOrientation.x)) * sin(glm::radians(mOrientation.y)) * sin(glm::radians(mOrientation.z)) + cos(glm::radians(mOrientation.x)) * cos(glm::radians(mOrientation.z)),
		cos(glm::radians(mOrientation.x)) * sin(glm::radians(mOrientation.y)) * sin(glm::radians(mOrientation.z)) + sin(glm::radians(mOrientation.x)) * cos(glm::radians(mOrientation.z)));
	mRightVector = -glm::cross(mUpVect, mViewVector);

}

glm::mat4 Transform::M2Weuler() const
{
	auto translate = glm::translate(mPosition);
	auto scale = glm::scale(mScale);
	auto rotate = glm::rotate(glm::radians(mOrientation.z), glm::vec3(0.0f, 0.0f, -1.0f)) *
		glm::rotate(glm::radians(mOrientation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::radians(mOrientation.x), glm::vec3(1.0f, 0.0f, 0.0f));

	return translate * rotate * scale;
}

glm::mat4 Transform::TransRotEulerMtx() const
{
	auto translate = glm::translate(mPosition);
	auto rotate = glm::rotate(glm::radians(mOrientation.z), glm::vec3(0.0f, 0.0f, -1.0f)) *
		glm::rotate(glm::radians(mOrientation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::radians(mOrientation.x), glm::vec3(1.0f, 0.0f, 0.0f));

	return translate * rotate;
}
#endif