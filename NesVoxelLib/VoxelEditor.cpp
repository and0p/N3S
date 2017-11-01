#pragma once
#include "stdafx.h"
#include "VoxelEditor.hpp"
#include "Input.hpp"
#include "Overlay.hpp"
#include "Gui.hpp"

float gridOpacity = 1.0f;
float gridCoveredOpacity = 0.1f;

VoxelEditor::VoxelEditor(shared_ptr<SpriteMesh> mesh, SceneSprite *ss, shared_ptr<OrbitCamera> referenceCamera) : mesh(mesh), ss(ss)
{
	workingX = 0.5f;
	workingY = 0.5f;
	workingZ = 0.5f;
	xSelect = floor(workingX);
	ySelect = floor(workingY);
	zSelect = floor(workingZ);
	// Set world coordinates of VoxelEditor's origin
	editorX = -1.0f + ss->x * pixelSizeW;
	editorY = 1.0f - (ss->y + 4) * pixelSizeW;
	editorZ = 0.0f;
	// Use scene camera as rotational reference, but adjust position to zero and zoom in
	camera = referenceCamera;
	camera->setZoom(0.3f);
	crop = { 0, 0, 0 ,0 };
	mirrorStyle = no_mirroring;
	mirrorPoint = { 0, 0 ,0 };
	updateCamera();
}

bool VoxelEditor::update(bool mouseAvailable)
{
	// Check for camera rotation
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
		camera->AdjustRotation(xRot, yRot, 0.0f);
	}
	viewingAngle = camera->getViewingAngle();
	// Check for camera panning, and pan along editing plane
	if (InputState::keyboardMouse->mouseButtons[middle_mouse].state > 0)
	{
		float xPan = InputState::keyboardMouse->mouseDeltaX / 7;
		float yPan = InputState::keyboardMouse->mouseDeltaY / 7;
		// Pan along plane
		adjustWorkingPositionAnalog(-xPan, -yPan, 0);
	}
	// Select point on editor plane using mouse or buttons
	adjustWorkingPositionAnalog(InputState::functions[editor_moveright]->value, InputState::functions[editor_movedown]->value, 0);
	adjustWorkingPositionAnalog(-InputState::functions[editor_moveleft]->value, -InputState::functions[editor_moveup]->value, 0);

	if (InputState::keyboardMouse->calculatedWheelDelta != 0)
		adjustWorkingPosition(0, 0, InputState::keyboardMouse->calculatedWheelDelta);

	// Add or remove voxels with buttons or mouse
	getMouseHighlight();

	if (mouseInEditor && InputState::keyboardMouse->mouseButtons[left_mouse].down)
	{
		mesh->updateVoxel(mouseHighlight.mirror(ss->mirrorH, ss->mirrorV), selectedColor);
		if (mirrorStyle > no_mirroring)
			mesh->updateVoxel(mirroredMouseHighlight.mirror(ss->mirrorH, ss->mirrorV), selectedColor);
	}

	if (InputState::functions[voxeleditor_movein]->activatedThisFrame)
	{
		adjustWorkingPosition(0, 0, 1);
	}

	else if (InputState::functions[voxeleditor_moveout]->activatedThisFrame)
		adjustWorkingPosition(0, 0, -1);

	// Update camera position after selections may be changed
	updateCamera();

	selection = { xSelect, ySelect, zSelect };
	
	// Check for mirroring change
	if (InputState::functions[voxeleditor_mirror]->activatedThisFrame)
		setMirroring();

	// If mirroring is active, update mirrored mouse highlight and selection
	mirroredSelection = selection.mirrorMesh(mirrorPoint, mirrorDirection, mirrorStyle);
	mirroredMouseHighlight = mouseHighlight.mirrorMesh(mirrorPoint, mirrorDirection, mirrorStyle);

	// See if we're holding alt, and update sprite cropping if so
	if (InputState::functions[selection_remove]->active)
		updateCroping();
	else
		crop = { 0, 0, 0, 0 };

	// Add or remove voxels
	if (InputState::functions[voxeleditor_setvoxel]->active)
	{
		mesh->updateVoxel(selection.mirror(ss->mirrorH, ss->mirrorV), selectedColor);
		if(mirrorStyle > no_mirroring)
			mesh->updateVoxel(mirroredSelection.mirror(ss->mirrorH, ss->mirrorV), selectedColor);
	}
	else if (InputState::functions[voxeleditor_deletevoxel]->active)
	{
		mesh->updateVoxel(selection.mirror(ss->mirrorH, ss->mirrorV), 0);
		if (mirrorStyle > no_mirroring)
			mesh->updateVoxel(mirroredSelection.mirror(ss->mirrorH, ss->mirrorV), 0);
	}

	return mouseAvailable;
}

void VoxelEditor::render()
{
	// Render the sprite being edited in place
	N3s3d::setDepthBufferState(true);
	N3s3d::setShader(color);
	if (crop.zeroed())
	{	// TODO this only applies to one game and/or N3S file that I know of, get rid of???
		mesh->render(ss->x + crop.left, ss->y + crop.top, ss->palette, ss->mirrorH, ss->mirrorV, crop);
	}
	else
	{
		mesh->render(ss->x + crop.left, ss->y + crop.top, ss->palette, ss->mirrorH, ss->mirrorV, crop);
	}
	// Render overlays
	N3s3d::setShader(overlay);
	N3s3d::updateMatricesWithCamera(camera);
	// Check which side the camera is facing and draw appropriate guides
	Overlay::setColor(1.0f, 1.0f, 1.0f, 0.2f);
	N3s3d::setRasterFillState(true);
	Overlay::drawVoxelPreview(ss->x + xSelect, ss->y + ySelect, zSelect);
	// TEST draw where the mouse is intersecting the editor plane
	if (mouseInEditor)
	{
		Overlay::setColor(0.5f, 0.5f, 0.5f, 1.0f);
		Overlay::drawVoxelPreview(mouseHighlight.x + ss->x, mouseHighlight.y + ss->y, mouseHighlight.z);
	}
	renderGrid(selection, viewingAngle, { 0.8f, 0.8f, 0.8f, 0.4f }, true);
	renderGrid(selection, viewingAngle, { 0.8f, 0.8f, 0.8f, 0.4f }, false);
	ViewingAngle mirrorViewingAngle;
	if (mirrorStyle > no_mirroring)
	{
		// Render single mirror guide
		if (mirrorDirection == mirror_x)
		{
			mirrorViewingAngle.x = v_left;
			mirrorViewingAngle.y = v_facing;
		}
		else if (mirrorDirection == mirror_y)
		{
			mirrorViewingAngle.x = v_facing;
			mirrorViewingAngle.y = v_top;
		}
		else if (mirrorDirection == mirror_z)
		{
			mirrorViewingAngle.x = v_front;
			mirrorViewingAngle.y = v_facing;
		}
		renderGrid(mirrorPoint, mirrorViewingAngle, { 1.0f, 0.0f, 0.0f, 0.4f }, false);
		if (mirrorStyle == mirror_offset)
		{
			// Render additional, offset mirror guide
			Vector3D mirrorOffset = mirrorPoint;
			if (mirrorDirection == mirror_x)
			{
				mirrorOffset.x++;
			}
			else if (mirrorDirection == mirror_y)
			{
				mirrorOffset.y++;
			}
			else if (mirrorDirection == mirror_z)
			{
				mirrorOffset.z++;
			}
			renderGrid(mirrorOffset, mirrorViewingAngle, { 1.0f, 0.0f, 0.0f, 0.4f }, false);
		}
		// Draw mirrored position
		Overlay::setColor(1.0f, 0.0f, 0.0f, 0.2f);
		Overlay::drawVoxelPreview(mirroredSelection.x + ss->x, mirroredSelection.y + ss->y, mirroredSelection.z);
		if (mouseInEditor)
		{
			Overlay::drawVoxelPreview(mirroredMouseHighlight.x + ss->x, mirroredMouseHighlight.y + ss->y, mirroredMouseHighlight.z);
		}
	}
}

void VoxelEditor::renderGrid(Vector3D v, ViewingAngle vA, Color4F color, bool depthEnabled)
{
	if (vA.y == v_top)
	{
		N3s3d::setDepthBufferState(depthEnabled);
		Overlay::setColor(color.r, color.g, color.b, color.a);
		Overlay::drawVoxelGrid(ss->x, ss->y, v.y + 1, yAxis);
	}
	else if (vA.y == v_bottom)
	{
		N3s3d::setDepthBufferState(depthEnabled);
		Overlay::setColor(color.r, color.g, color.b, color.a);
		Overlay::drawVoxelGrid(ss->x, ss->y, v.y, yAxis);
	}
	else
	{
		if (vA.x == v_front)
		{
			N3s3d::setDepthBufferState(depthEnabled);
			Overlay::setColor(color.r, color.g, color.b, color.a);
			Overlay::drawVoxelGrid(ss->x, ss->y, v.z + 1, xAxis);
		}
		else if (vA.x == v_back)
		{
			N3s3d::setDepthBufferState(depthEnabled);
			Overlay::setColor(color.r, color.g, color.b, color.a);
			Overlay::drawVoxelGrid(ss->x, ss->y, v.z, xAxis);
		}
		else if (vA.x == v_left)
		{
			N3s3d::setDepthBufferState(depthEnabled);
			Overlay::setColor(color.r, color.g, color.b, color.a);
			Overlay::drawVoxelGrid(ss->x, ss->y, v.x + 1, zAxis);
		}
		else
		{
			N3s3d::setDepthBufferState(depthEnabled);
			Overlay::setColor(color.r, color.g, color.b, color.a);
			Overlay::drawVoxelGrid(ss->x, ss->y, v.x, zAxis);
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
	if (InputState::functions[selection_copy]->active)
	{
		mesh->moveLayer(oldX, oldY, oldZ, workingX, workingY, workingZ, true);
	}
	else if (InputState::functions[selection_add]->active)
	{
		mesh->moveLayer(oldX, oldY, oldZ, workingX, workingY, workingZ, false);
	}
	if (mirrorStyle > 0)
	{
		// Awkwardly transition to Vector3D for mirroring
		Vector3D oldC = { oldX, oldY, oldZ };
		Vector3D newC = { workingX, workingY, workingZ };
		oldC = oldC.mirrorMesh(mirrorPoint, mirrorDirection, mirrorStyle);
		newC = newC.mirrorMesh(mirrorPoint, mirrorDirection, mirrorStyle);
		if (InputState::functions[selection_copy]->active)
		{
			mesh->moveLayer(oldC.x, oldC.y, oldC.z, newC.x, newC.y, newC.z, true);
		}
		else if (InputState::functions[selection_add]->active)
		{
			mesh->moveLayer(oldC.x, oldC.y, oldC.z, newC.x, newC.y, newC.z, false);
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
	camera->SetPosition(
		editorX + (workingX * pixelSizeW),
		editorY - (workingY * pixelSizeW),
		editorZ + (workingZ * pixelSizeW)
	);
}

void VoxelEditor::updateCroping()
{
	if ((viewingAngle.x == v_front || viewingAngle.x == v_back) && viewingAngle.y == v_facing)
	{
		crop = { 0, 0, 0, 0 };
	}
	else if (viewingAngle.y == v_top)
	{
		crop = { selection.y, 0, 0, 0 };
	}
	else if (viewingAngle.y == v_bottom)
	{
		crop = { 0, 0, 7 - selection.y, 0 };
	}
	else if (viewingAngle.x == v_left)
	{
		crop = { 0, selection.x, 0, 0 };
	}
	else if (viewingAngle.x == v_right)
	{
		crop = { 0, 0, 0, 7 - selection.x };
	}
}

void VoxelEditor::getMouseHighlight()
{
	int mouseX = InputState::keyboardMouse->mouseX;
	int mouseY = InputState::keyboardMouse->mouseY;
	// Figure out which plane we're on
	if (viewingAngle.y == v_top || viewingAngle.y == v_bottom)
		mouseHighlight = N3s3d::getPixelCoordsFromFloat3(N3s3d::getPlaneIntersection(y_axis, ySelect + ss->y, camera.get(), mouseX, mouseY));
	else
		if ( viewingAngle.x == v_front || viewingAngle.x == v_back)
			mouseHighlight = N3s3d::getPixelCoordsFromFloat3(N3s3d::getPlaneIntersection(z_axis, zSelect, camera.get(), mouseX, mouseY));
		else
			mouseHighlight = N3s3d::getPixelCoordsFromFloat3(N3s3d::getPlaneIntersection(x_axis, xSelect + ss->x, camera.get(), mouseX, mouseY));
	// TODO: add shift-modifier to lock on X/Y/Z axis
	mouseHighlight.x -= ss->x;
	mouseHighlight.y -= ss->y;
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

void VoxelEditor::setMirroring()
{
	// If we are not currently mirroring, set up single mirroring at current selection / direction
	if (mirrorStyle == no_mirroring)
	{
		mirrorPoint = selection;
		mirrorStyle = mirror_single;
		mirrorDirection = camera->getMirrorDirection();
	}
	else
	{
		// See if mirror spot / direction is the same
		MirrorDirection mD = camera->getMirrorDirection();
		if (mirrorPoint.equals(selection) && mirrorDirection == mD)
		{
			// Cycle (or remove) mirroring
			if (mirrorStyle == mirror_single)
				mirrorStyle = mirror_offset;
			else
				mirrorStyle = no_mirroring;
		}
		else
		{
			// Update mirroring as single from new selection point
			mirrorStyle = mirror_single;
			mirrorDirection = mD;
			mirrorPoint = selection;
		}

	}
}
