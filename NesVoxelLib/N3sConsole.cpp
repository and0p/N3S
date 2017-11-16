#include "stdafx.h"
#include "N3sConsole.hpp"

unique_ptr<vector<ConsoleLine>> lines;

void N3sConsole::init()
{
	lines = make_unique<vector<ConsoleLine>>();
}

void N3sConsole::writeLine(string line)
{
	lines->push_back(ConsoleLine(line));
}

void N3sConsole::update()
{
	if (lines->size() > 0)
	{
		// Create new list of lines
		vector<ConsoleLine>* temp = new vector<ConsoleLine>();
		for each(ConsoleLine c in *lines)
		{
			if (c.stillAlive())
				temp->emplace_back(c);
		}
		lines.reset(temp);
	}
}

void N3sConsole::render()
{
	N3s3d::setDepthBufferState(false);
	N3s3d::setShader(overlay);
	N3s3d::setOverlayColor(255, 255, 255, 255);
	N3s3d::setGuiProjection();
	for (int i = 0; i < lines->size(); i++)
	{
		Overlay::drawString(0, i * 8 * consoleScale, consoleScale, (*lines)[i].line);
	}
}

ConsoleLine::ConsoleLine(string line) : line(line)
{
	creationTime = clock();
}

bool ConsoleLine::stillAlive()
{
	if (((clock() - creationTime) / CLOCKS_PER_SEC) < messageDuration)
		return true;
	else
		return false;
}
