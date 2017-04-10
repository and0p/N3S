#include "stdafx.h"
#include "VoxelEditor.hpp"
#include "Input.hpp"
#include "Overlay.hpp"

VoxelEditor::VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, OrbitCamera referenceCamera) : mesh(mesh), pixelX(pixelX), pixelY(pixelY)
{
	// Use scene camera as rotational reference, but adjust position to zero and zoom in
	camera = referenceCamera;
	camera.SetPosition(-1.0f + (pixelX + 4) * pixelSizeW, 1.0f - (pixelY + 4) * pixelSizeH, 0.0f);
	camera.setZoom(0.3f);
}

bool VoxelEditor::update(bool mouseAvailable)
{
	// Update camera position
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
		camera.AdjustRotation(xRot, yRot, 0.0f);
	}
	return false;
}

void VoxelEditor::render()
{
	N3s3d::setShader(overlay);
	N3s3d::updateMatricesWithCamera(&camera);
	// Check which side the camera is facing and draw appropriate guides
	ViewSide viewSide = camera.getViewSide();
	if (viewSide == view_front || viewSide == view_back)
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
		Overlay::drawVoxelGrid(pixelX, pixelY, 3, xAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
		Overlay::drawVoxelGrid(pixelX, pixelY, 3, xAxis);
	}
	else if (viewSide == view_top || viewSide == view_bottom)
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
		Overlay::drawVoxelGrid(pixelX, pixelY, 3, yAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
		Overlay::drawVoxelGrid(pixelX, pixelY, 3, yAxis);
	}
	else
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
		Overlay::drawVoxelGrid(pixelX, pixelY, 3, zAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
		Overlay::drawVoxelGrid(pixelX, pixelY, 3, zAxis);
	}

}
