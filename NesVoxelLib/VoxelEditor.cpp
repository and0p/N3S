#include "stdafx.h"
#include "VoxelEditor.hpp"
#include "Input.hpp"
#include "Overlay.hpp"

VoxelEditor::VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, OrbitCamera referenceCamera) : mesh(mesh), pixelX(pixelX), pixelY(pixelY)
{
	workingX = 0.0f;
	workingY = 0.0f;
	workingZ = 0.0f;
	// Set world coordinates of VoxelEditor's origin
	editorX = -1.0f + pixelX * pixelSizeW;
	editorY = 1.0f - (pixelY + 4) * pixelSizeH;
	editorZ = 0.0f;
	// Use scene camera as rotational reference, but adjust anposition to zero and zoom in
	camera = referenceCamera;
	camera.setZoom(0.3f);
	updateCamera();
}

bool VoxelEditor::update(bool mouseAvailable)
{
	updateCamera();
	// TESTING check keys to shift working position
	adjustWorkingPositionAnalog(InputState::functions[voxeleditor_moveright].value, InputState::functions[voxeleditor_movedown].value, 0);
	adjustWorkingPositionAnalog(-InputState::functions[voxeleditor_moveleft].value, -InputState::functions[voxeleditor_moveup].value, 0);
	float zAdjust = 0.0f;
	zAdjust += InputState::keyboardMouse->calculatedWheelDelta;
	zAdjust += InputState::functions[voxeleditor_movein].value;
	zAdjust -= InputState::functions[voxeleditor_moveout].value;
	adjustWorkingPositionAnalog(0, 0, zAdjust);
	// Add or remove voxels
	if (InputState::functions[voxeleditor_setvoxel].active)
		mesh->updateVoxel(xSelect, ySelect, zSelect, 1);
	if (InputState::functions[voxeleditor_deletevoxel].active)
		mesh->updateVoxel(xSelect, ySelect, zSelect, 0);
	return false;
}

void VoxelEditor::render()
{
	N3s3d::setShader(overlay);
	N3s3d::updateMatricesWithCamera(&camera);
	// Check which side the camera is facing and draw appropriate guides
	// Guides are displayed at floor() of working position
	xSelect = floor(workingX);
	ySelect = floor(workingY);
	zSelect = floor(workingZ);
	// TEST
	Overlay::setColor(1.0f, 1.0f, 1.0f, 0.2f);
	N3s3d::setRasterFillState(true);
	Overlay::drawVoxelPreview(pixelX + xSelect, pixelY + ySelect, zSelect);
	N3s3d::setRasterFillState(false);
	if (viewingAngle.y == v_top || viewingAngle.y == v_bottom)
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect, yAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect, yAxis);
	}
	else
	{
		if (viewingAngle.x == v_front || viewingAngle.x == v_back)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect, xAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect, xAxis);
		}
		else
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect, zAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect, zAxis);
		}
	}
}

void VoxelEditor::adjustWorkingPositionAnalog(float x, float y, float z)
{
	// Convert pixels of dragging into floats
	float xAdjust = x * 0.5f;
	float yAdjust = y * 0.5f;
	float zAdjust = z * 0.5f;
	// Change working position based on camera view side
	if (viewingAngle.y == v_top)
	{
		switch (viewingAngle.x)
		{
		case v_front:
			workingX += xAdjust;
			workingZ -= yAdjust;
			break;
		case v_back:
			workingX -= xAdjust;
			workingZ += yAdjust;
			break;
		case v_left:
			workingX += yAdjust;
			workingZ += xAdjust;
			break;
		case v_right:
			workingX -= yAdjust;
			workingZ -= xAdjust;
			break;
		}
		workingZ += zAdjust;
	}
	else if (viewingAngle.y == v_bottom)
	{
		switch (viewingAngle.x)
		{
		case v_front:
			workingX += xAdjust;
			workingZ += yAdjust;
			break;
		case v_back:
			workingX -= xAdjust;
			workingZ -= yAdjust;
			break;
		case v_left:
			workingX -= yAdjust;
			workingZ += xAdjust;
			break;
		case v_right:
			workingX += yAdjust;
			workingZ -= xAdjust;
			break;
		}
		workingZ -= zAdjust;
	}
	else // front
	{
		switch (viewingAngle.x)
		{
		case v_front:
			workingX += xAdjust;
			workingZ += zAdjust;
			break;
		case v_back:
			workingX -= xAdjust;
			workingZ -= zAdjust;
			break;
		case v_left:
			workingZ += xAdjust;
			workingX += zAdjust;
			break;
		case v_right:
			workingZ -= xAdjust;
			workingX -= zAdjust;
			break;
		}
		workingY += yAdjust; // Up is always up from the sides
	}
	// Make sure these are all within the bounds of our editing area and clamp if needed
	if (workingX >= 8)
		workingX = 7.99999;
	else if (workingX < 0)
		workingX = 0;
	if (workingY >= 8)
		workingY = 7.99999;
	else if (workingY < 0)
		workingY = 0;
	if (workingZ >= 32)
		workingZ = 31.99999;
	else if (workingZ < 0)
		workingZ = 0;
}

void VoxelEditor::changeLayers(int amount)
{

}

void VoxelEditor::updateCamera()
{
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
		camera.AdjustRotation(xRot, yRot, 0.0f);
	}
	viewingAngle = camera.getViewingAngle();
	camera.SetPosition(
		editorX + (workingX * pixelSizeW),
		editorY - (workingY * pixelSizeH),
		editorZ + (workingZ * pixelSizeH)
	);
}
