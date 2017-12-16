#include "stdafx.h"
#include "FileUtil.hpp"
#include <iostream>
#include <memory>
#include <fstream>

N3sRomConnection::N3sRomConnection(string filePath) : givenPath(filePath)
{
	try
	{
		// See if ROM exists
		romPath = path(filePath);
		if (exists(romPath) && romPath.has_filename())
			valid = true;
		// Do other operations if so
		if (valid)
		{
			// Get the game name, minus extension
			path temp = romPath;
			string gameName = temp.replace_extension("").filename().string();
			// Get the ROM name itself
			romName = romPath.filename().string();
			// Find the directory that the rom is sitting in
			temp = romPath;
			romDirectory = temp.remove_filename();
			// Create the N3S directory, if needed
			temp = romDirectory;
			n3sFolderPath = temp.append(".n3s");
			if (!exists(n3sFolderPath))
				create_directory(n3sFolderPath);
			// Create the game folder, if needed
			temp = n3sFolderPath;
			romN3sPath = temp.append(gameName);
			if (!exists(romN3sPath))
				create_directory(romN3sPath);
			// Find the path of the default N3S file for this ROM
			temp = romPath;
			n3sPath = temp.replace_extension("n3s");
			if (exists(n3sPath))
				n3sFileExists = true;
			else
				n3sFileExists = false;
		}
	}
	catch (exception e)
	{
		// Any exception for now just marks this as invalid (aka unloadable)
		valid = false;
	}
}

const char * N3sRomConnection::getRomCStr()
{
	romPathAsString = romPath.string();
	return romPathAsString.c_str();
}

const wchar_t * N3sRomConnection::getRomWCStr()
{
	return romPath.c_str();
}

const wchar_t * N3sRomConnection::getN3sWCStr()
{
	return n3sPath.c_str();
}

string N3sRomConnection::getN3sString()
{
	return n3sPath.string();
}

string N3sRomConnection::getRomN3sPath()
{
	return romN3sPath.string();
}

bool N3sRomConnection::isValid()
{
	return valid;
}

bool N3sRomConnection::disconnect()
{
	return true;
}
