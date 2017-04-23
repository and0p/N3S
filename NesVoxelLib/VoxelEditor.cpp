#pragma once
#include "stdafx.h"
#include "VoxelEditor.hpp"
#include "Input.hpp"
#include "Overlay.hpp"

VoxelEditor::VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, OrbitCamera referenceCamera) : mesh(mesh), pixelX(pixelX), pixelY(pixelY)
{
	workingX = 0.5f;
	workingY = 0.5f;
	workingZ = 0.5f;
	xSelect = floor(workingX);
	ySelect = floor(workingY);
	zSelect = floor(workingZ);
	// Set world coordinates of VoxelEditor's origin
	editorX = -1.0f + pixelX * pixelSizeW;
	editorY = 1.0f - (pixelY + 4) * pixelSizeW;
	editorZ = 0.0f;
	// Use scene camera as rotational reference, but adjust position to zero and zoom in
	camera = referenceCamera;
	camera.setZoom(0.3f);
	updateCamera();
}

bool VoxelEditor::update(bool mouseAvailable)
{
	// Check keys to shift working position
	adjustWorkingPositionAnalog(InputState::functions[voxeleditor_moveright].value, InputState::functions[voxeleditor_movedown].value, 0);
	adjustWorkingPositionAnalog(-InputState::functions[voxeleditor_moveleft].value, -InputState::functions[voxeleditor_moveup].value, 0);

	if (InputState::keyboardMouse->calculatedWheelDelta != 0)
		adjustWorkingPosition(0, 0, InputState::keyboardMouse->calculatedWheelDelta);
	
	if (InputState::functions[voxeleditor_movein].activatedThisFrame)
		adjustWorkingPosition(0, 0, 1);
	else if (InputState::functions[voxeleditor_moveout].activatedThisFrame)
		adjustWorkingPosition(0, 0, -1);

	// Update camera position after selections may be changed
	updateCamera();

	// Add or remove voxels
	if (InputState::functions[voxeleditor_setvoxel].active)
		mesh->updateVoxel(xSelect, ySelect, zSelect, 1);
	else if (InputState::functions[voxeleditor_deletevoxel].active)
		mesh->updateVoxel(xSelect, ySelect, zSelect, 0);
	return false;
}

void VoxelEditor::render()
{
	N3s3d::setShader(overlay);
	N3s3d::updateMatricesWithCamera(&camera);
	// Check which side the camera is facing and draw appropriate guides
	// TEST
	Overlay::setColor(1.0f, 1.0f, 1.0f, 0.2f);
	N3s3d::setRasterFillState(true);
	Overlay::drawVoxelPreview(pixelX + xSelect, pixelY + ySelect, zSelect);
	N3s3d::setRasterFillState(false);
	if (viewingAngle.y == v_top)
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect + 1, yAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect + 1, yAxis);
	}
	else if (viewingAngle.y == v_bottom)
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
		if (viewingAngle.x == v_front)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect + 1, xAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect + 1, xAxis);
		}
		else if (viewingAngle.x == v_back)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect, xAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect, xAxis);
		}
		else if (viewingAngle.x == v_left)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.1f);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect + 1, zAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, 0.3f);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect + 1, zAxis);
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

void VoxelEditor::adjustWorkingPosition(int x, int y, int z)
{
	// Snap position to 0.5
	workingX = floor(workingX) + 0.5f;
	workingY = floor(workingY) + 0.5f;
	workingZ = floor(workingZ) + 0.5f;
	// Change working position based on camera view side
	if (viewingAngle.y == v_top)
	{
		switch (viewingAngle.x)
		{
		case v_front:
			workingX += x;
			workingZ -= y;
			break;
		case v_back:
			workingX -= x;
			workingZ += y;
			break;
		case v_right:
			workingX += y;
			workingZ += x;
			break;
		case v_left:
			workingX -= y;
			workingZ -= x;
			break;
		}
		workingZ += z;
	}
	else if (viewingAngle.y == v_bottom)
	{
		switch (viewingAngle.x)
		{
		case v_front:
			workingX += x;
			workingZ += y;
			break;
		case v_back:
			workingX -= x;
			workingZ -= y;
			break;
		case v_right:
			workingX -= y;
			workingZ += x;
			break;
		case v_left:
			workingX += y;
			workingZ -= x;
			break;
		}
		workingZ -= z;
	}
	else // front
	{
		switch (viewingAngle.x)
		{
		case v_front:
			workingX += x;
			workingZ += z;
			break;
		case v_back:
			workingX -= x;
			workingZ -= z;
			break;
		case v_right:
			workingZ += x;
			workingX += z;
			break;
		case v_left:
			workingZ -= x;
			workingX -= z;
			break;
		}
		workingY += y; // Up is always up from the sides
	}
	// Make sure these are all within the bounds of our editing area and clamp if needed
	if (workingX >= 8)
		workingX = 7.5f;
	else if (workingX < 0)
		workingX = 0.5f;
	if (workingY >= 8)
		workingY = 7.5f;
	else if (workingY < 0)
		workingY = 0.5f;
	if (workingZ >= 32)
		workingZ = 31.5f;
	else if (workingZ < 0)
		workingZ = 0.5f;
	// The selection is always the floor() of the analog x/y/z
	xSelect = floor(workingX);
	ySelect = floor(workingY);
	zSelect = floor(workingZ);
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
		case v_right:
			workingX += yAdjust;
			workingZ += xAdjust;
			break;
		case v_left:
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
		case v_right:
			workingX -= yAdjust;
			workingZ += xAdjust;
			break;
		case v_left:
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
		case v_right:
			workingZ += xAdjust;
			workingX += zAdjust;
			break;
		case v_left:
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
	// The selection is always the floor() of the analog x/y/z
	xSelect = floor(workingX);
	ySelect = floor(workingY);
	zSelect = floor(workingZ);
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
		editorY - (workingY * pixelSizeW),
		editorZ + (workingZ * pixelSizeW)
	);
}
