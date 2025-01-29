#include "Transform.h"

using namespace NCL::CSC8503;

Transform::Transform()	{
	scale = Vector3(1, 1, 1);
}

Transform::~Transform()	{

}

void Transform::UpdateMatrix() {
	matrix =
		Matrix::Translation(position) *
		Quaternion::RotationMatrix<Matrix4>(orientation) *
		Matrix::Scale(scale);
}

Transform& Transform::SetPosition(const Vector3& worldPos) {
	position = worldPos;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetScale(const Vector3& worldScale) {
	scale = worldScale;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetOrientation(const Quaternion& worldOrientation) {
	orientation = worldOrientation;
	UpdateMatrix();
	return *this;
}