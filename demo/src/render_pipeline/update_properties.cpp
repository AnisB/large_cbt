// Project includes
#include "render_pipeline/update_properties.h"

// Project includes
#include <string.h>

void mirror_update_properties(const Camera& camera, UpdateProperties& updateProperties)
{
	// Save the state
	updateProperties.viewProj = camera.viewProjection;
	updateProperties.invViewProj = camera.invViewProjection;
	updateProperties.position = camera.position;
	updateProperties.forward = { camera.view.m[2], camera.view.m[6], camera.view.m[10] };
	updateProperties.farPlane = camera.nearFar.y;
	updateProperties.fov = camera.fov;

	// Compute the frustum planes
	extract_planes_from_view_projection_matrix(updateProperties.viewProj, updateProperties.frustum);
}

void set_update_properties(const UpdateProperties& updateProperties, UpdateCB& updateCB)
{
	// Camera properties
	updateCB._UpdateViewProjectionMatrix = updateProperties.viewProj;
	updateCB._UpdateInvViewProjectionMatrix = updateProperties.invViewProj;
	updateCB._UpdateCameraPosition = updateProperties.position;
	updateCB._UpdateCameraPositionSP = { (float)updateProperties.position.x, (float)updateProperties.position.y, (float)updateProperties.position.z };
	updateCB._UpdateCameraForward = updateProperties.forward;
	updateCB._UpdateFOV = updateProperties.fov;
	updateCB._UpdateFarPlaneDistance = updateProperties.farPlane;
	memcpy(updateCB._UpdateFrustumPlanes, updateProperties.frustum.planes, sizeof(Plane) * 6);
}
