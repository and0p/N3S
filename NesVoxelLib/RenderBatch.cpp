#include "stdafx.h"
#include "RenderBatch.hpp"

RenderBatch::RenderBatch(shared_ptr<GameData> gameData, shared_ptr<PpuSnapshot> snapshot, shared_ptr<VirtualPatternTable> vPatternTable)
	:	gameData(gameData), snapshot(snapshot), vPatternTable(vPatternTable)
{
	computeSpritesOAM();
	computeSpritesNametable();
	processStencilGroups();
}

void RenderBatch::computeSpritesOAM()
{
	// Grab all (true) sprites from the snapshot
	for each (OamSprite s in snapshot->sprites)
	{
		int x = s.x;
		int y = s.y;
		int tile = s.tile;
		int palette = s.palette;
		// Make sure palette isn't out of bounds for any reason
		if (palette > 7)
			palette = 7;
		// Branch based on 16x8 mode
		if (snapshot->ctrl.spriteSize16x8)
		{
			// In 8x16, pattern table selection is specified by first bit.
			// Since you can only select even (in base 0) tiles it is free for this purpose.
			// Actual tile selection is bits 1-7.
			int patternSelection = tile & 1;
			// Nestopia tends to give tiles by absolute value, but seemingly only some of the time?
			// So we need to correct to be sure
			if (patternSelection && tile < 256)
				tile += 256;
			else if (!patternSelection && tile > 255)
				tile -= 256;
			if (patternSelection)
				tile--;
			// Flipped sprites in 8x16 also swap places on the Y axis
			if (s.vFlip)
				y += 8;
			// Get true tile #
			shared_ptr<VirtualSprite> vSprite = vPatternTable->getSprite(tile);
			shared_ptr<VirtualSprite> vSprite2 = vPatternTable->getSprite(tile + 1);
			// Render the first sprite
			computedSprites.push_back({ vSprite, nullptr, x, y, palette, s.hFlip, s.vFlip });
			//renderSprite(vSprite, x, y, s.palette, s.hFlip, s.vFlip);
			// Render the second sprite, which swaps place with vertical flip
			if (s.vFlip)
				computedSprites.push_back({ vSprite2, nullptr, x, y - 8, palette, s.hFlip, s.vFlip });
			else
				computedSprites.push_back({ vSprite2, nullptr, x, y + 8, palette, s.hFlip, s.vFlip });
		}
		else
		{
			// Select tile based on pattern table in CTRL register
			if (tile > 255 && !snapshot->getOAMSelectAtScanline(y))
			{
				tile -= 256;
			}
			else if (tile < 256 && snapshot->getOAMSelectAtScanline(y))
			{
				tile += 256;
			}
			shared_ptr<VirtualSprite> vSprite = vPatternTable->getSprite(tile);
			computedSprites.push_back({ vSprite, nullptr, x, y, palette, s.hFlip, s.vFlip });
		}
	}
	// Finally, process dynamic meshes
	processMeshesOAM();
}

void RenderBatch::computeSpritesNametable()
{
}

void RenderBatch::processMeshesOAM()
{
	// Process meshes (FOR NOW ASSUMING DEFAULT)
	for each(ComputedSprite s in computedSprites)
	{
		s.mesh = s.virtualSprite->defaultMesh;
	}
}

void RenderBatch::processStencilGroups()
{
	int previousColorIndex = -1;
	for each(ComputedSprite s in computedSprites)	// Assuming this maintains insertion order...
	{
		int newColorIndex = s.palette + s.mesh->outlineColor;
		// See if this mesh has any outlines
		if (s.mesh->outlineColor >= 0)
		{
			// Create bool, assuming same group
			bool sameGroup = true;
			// Check all grouping restraints
			if (gameData->oamGrouping == continous_samecolor)
			{
				// See if the color is the same as before
				if (newColorIndex != previousColorIndex)
					sameGroup = false;
			}
			// If grouping is not the same, increment stencil number
			if (!sameGroup)
				currentStencilNumber++;
			s.stencilGroup = currentStencilNumber;
		}
		else	// If this has no outline, 
		{
			s.stencilGroup = -1;
			previousColorIndex = -1;
		}
	}
}

void RenderBatch::batchDrawCalls()
{

}

void RenderBatch::render(shared_ptr<Camera> camera)
{
}
