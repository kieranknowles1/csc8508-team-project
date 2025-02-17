#include "Camera.h"
#include "Window.h"

using namespace NCL;

/*
Polls the camera for keyboard / mouse movement.
Should be done once per frame! Pass it the msec since
last frame (default value is for simplicities sake...)
*/
void Camera::UpdateCamera(float dt, bool movement) {
	if (!activeController) {
		return;
	}

	//Update the mouse by how much
	pitch -= activeController->GetAnalogue(Controller::AnalogueControl::LookY);


	//Bounds check the pitch, to be between straight up and straight down ;)
	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);


	if (movement) {
		yaw -= activeController->GetAnalogue(Controller::AnalogueControl::LookX);
		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}

		float frameSpeed = speed * dt;
		Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));

		position += yawRotation * Vector3(0, 0, -activeController->GetAnalogue(Controller::AnalogueControl::MoveForward)) * frameSpeed;
		position += yawRotation * Vector3(activeController->GetAnalogue(Controller::AnalogueControl::MoveSidestep), 0, 0) * frameSpeed;

		position.y += activeController->GetAnalogue(Controller::AnalogueControl::MoveUpDown) * frameSpeed;
	}
	else {
		yaw = 0;
	}

}

/*
Generates a view matrix for the camera's viewpoint. This matrix can be sent
straight to the shader...it's already an 'inverse camera' matrix.
*/
Matrix4 Camera::BuildViewMatrix() const {
	//Why do a complicated matrix inversion, when we can just generate the matrix
	//using the negative values ;). The matrix multiplication order is important!
	return	Matrix::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix::Rotation(-roll, Vector3(0, 0, 1)) *
		Matrix::Translation(-position);
};

Matrix4 PerspectiveCamera::BuildProjectionMatrix(float currentAspect) const {
	return Matrix::Perspective(nearPlane, farPlane, currentAspect, fov);
}

Matrix4 OrhographicCamera::BuildProjectionMatrix(float currentAspect) const {
	return Matrix::Orthographic(left, right, bottom, top, nearPlane, farPlane);
}
