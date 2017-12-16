#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>

using namespace std;
using namespace std::experimental::filesystem::v1;

class FileUtil {
	//static void initWithRom(string romPath);
};

class N3sRomConnection {
public:
	N3sRomConnection() {}
	N3sRomConnection(string filePath);
	const char * getRomCStr();
	const wchar_t* getRomWCStr();
	const wchar_t* getN3sWCStr();
	string getN3sString();
	string getRomN3sPath();
	bool isValid();
	bool n3sFileExists = false;
	bool disconnect();
private:
	bool valid = false;
	bool mustMakeRelativeN3S = true;
	string givenPath;
	string romName;			// Name of file, minus extension
	path romPath;			// ROM full path, including filename
	path romDirectory;		// Directory that the ROM is in
	path n3sPath;			// N3S file full path, including filename
	path romN3sPath;		// Where the N3S and save states are stored
	path n3sFolderPath;		// Relative N3S path for this rom (/.n3s/)
	string romPathAsString;
};