#pragma once
#include "stdafx.h"
#include "VoxelEditor.hpp"
#include "Input.hpp"
#include "Overlay.hpp"
#include "Gui.hpp"

float gridOpacity = 1.0f;
float gridCoveredOpacity = 0.1f;

VoxelEditor::VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, bool mirrorH, bool mirrorV, OrbitCamera referenceCamera) : mesh(mesh), pixelX(pixelX), pixelY(pixelY), mirrorH(mirrorH), mirrorV(mirrorV)
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
	// Check for camera rotation
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
		camera.AdjustRotation(xRot, yRot, 0.0f);
	}
	viewingAngle = camera.getViewingAngle();
	// Check for camera panning, and pan along editing plane
	if (InputState::keyboardMouse->mouseButtons[middle_mouse].state > 0)
	{
		float xPan = InputState::keyboardMouse->mouseDeltaX / 7;
		float yPan = InputState::keyboardMouse->mouseDeltaY / 7;
		// Pan along plane
		adjustWorkingPositionAnalog(-xPan, -yPan, 0);
	}
	// Select point on editor plane using mouse or buttons
	adjustWorkingPositionAnalog(InputState::functions[voxeleditor_moveright].value, InputState::functions[voxeleditor_movedown].value, 0);
	adjustWorkingPositionAnalog(-InputState::functions[voxeleditor_moveleft].value, -InputState::functions[voxeleditor_moveup].value, 0);

	if (InputState::keyboardMouse->calculatedWheelDelta != 0)
		adjustWorkingPosition(0, 0, InputState::keyboardMouse->calculatedWheelDelta);

	// Add or remove voxels with buttons or mouse
	getMouseHighlight();

	if (mouseInEditor && InputState::keyboardMouse->mouseButtons[left_mouse].down)
		mesh->updateVoxel(mouseHighlight.mirror(mirrorH, mirrorV), selectedColor);

	if (InputState::functions[voxeleditor_movein].activatedThisFrame)
	{
		adjustWorkingPosition(0, 0, 1);
	}

	else if (InputState::functions[voxeleditor_moveout].activatedThisFrame)
		adjustWorkingPosition(0, 0, -1);

	// Update camera position after selections may be changed
	updateCamera();

	selection = { xSelect, ySelect, zSelect };

	// Add or remove voxels
	if (InputState::functions[voxeleditor_setvoxel].active)
		mesh->updateVoxel(selection.mirror(mirrorH, mirrorV), selectedColor);
	else if (InputState::functions[voxeleditor_deletevoxel].active)
		mesh->updateVoxel(selection.mirror(mirrorH, mirrorV), 0);

	return mouseAvailable;
}

void VoxelEditor::render()
{
	N3s3d::setShader(overlay);
	N3s3d::updateMatricesWithCamera(&camera);
	// Check which side the camera is facing and draw appropriate guides
	Overlay::setColor(1.0f, 1.0f, 1.0f, 0.2f);
	N3s3d::setRasterFillState(true);
	Overlay::drawVoxelPreview(pixelX + xSelect, pixelY + ySelect, zSelect);
	// TEST draw where the mouse is intersecting the editor plane
	if (mouseInEditor)
	{
		Overlay::setColor(0.5f, 0.5f, 0.5f, 1.0f);
		Overlay::drawVoxelPreview(mouseHighlight.x + pixelX, mouseHighlight.y + pixelY, mouseHighlight.z);
	}
	if (viewingAngle.y == v_top)
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, gridCoveredOpacity);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect + 1, yAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, gridOpacity);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect + 1, yAxis);
	}
	else if (viewingAngle.y == v_bottom)
	{
		N3s3d::setDepthBufferState(false);
		Overlay::setColor(0.5f, 0.5f, 0.5f, gridCoveredOpacity);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect, yAxis);
		N3s3d::setDepthBufferState(true);
		Overlay::setColor(0.5f, 0.5f, 0.5f, gridOpacity);
		Overlay::drawVoxelGrid(pixelX, pixelY, ySelect, yAxis);
	}
	else
	{
		if (viewingAngle.x == v_front)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridCoveredOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect + 1, xAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect + 1, xAxis);
		}
		else if (viewingAngle.x == v_back)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridCoveredOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect, xAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, zSelect, xAxis);
		}
		else if (viewingAngle.x == v_left)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridCoveredOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect + 1, zAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect + 1, zAxis);
		}
		else
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridCoveredOpacity);
			Overlay::drawVoxelGrid(pixelX, pixelY, xSelect, zAxis);
			N3s3d::setDepthBufferState(true);
			Overlay::setColor(0.5f, 0.5f, 0.5f, gridOpacity);
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
	// Cache the old position
	int oldX = workingX; int oldY = workingY; int oldZ = workingZ;
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
		workingY += z;
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
		workingY -= z;
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
			workingX -= z;
			break;
		case v_left:
			workingZ -= x;
			workingX += z;
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

	// See if we should copy/move the old layer as well
	if (InputState::functions[selection_copy].active)
		mesh->moveLayer(oldX, oldY, oldZ, workingX, workingY, workingZ, true);
	else if (InputState::functions[selection_add].active)
		mesh->moveLayer(oldX, oldY, oldZ, workingX, workingY, workingZ, false);
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
	camera.SetPosition(
		editorX + (workingX * pixelSizeW),
		editorY - (workingY * pixelSizeW),
		editorZ + (workingZ * pixelSizeW)
	);
}

void VoxelEditor::getMouseHighlight()
{
	int mouseX = InputState::keyboardMouse->mouseX;
	int mouseY = InputState::keyboardMouse->mouseY;
	// Figure out which plane we're on
	if (viewingAngle.y == v_top || viewingAngle.y == v_bottom)
		mouseHighlight = N3s3d::getPixelCoordsFromFloat3(N3s3d::getPlaneIntersection(y_axis, ySelect + pixelY, &camera, mouseX, mouseY));
	else
		if ( viewingAngle.x == v_front || viewingAngle.x == v_back)
			mouseHighlight = N3s3d::getPixelCoordsFromFloat3(N3s3d::getPlaneIntersection(z_axis, zSelect, &camera, mouseX, mouseY));
		else
			mouseHighlight = N3s3d::getPixelCoordsFromFloat3(N3s3d::getPlaneIntersection(x_axis, xSelect + pixelX, &camera, mouseX, mouseY));
	// TODO: add shift-modifier to lock on X/Y/Z axis
	mouseHighlight.x -= pixelX;
	mouseHighlight.y -= pixelY;
	// See if the highlighted area is even in the model
	if (mouseHighlight.x >= 0 && mouseHighlight.x < 8 &&
		mouseHighlight.y >= 0 && mouseHighlight.y < 8 &&
		mouseHighlight.z >= 0 && mouseHighlight.z < 32)
	{
		mouseInEditor = true;
	}
	else
		mouseInEditor = false;
}