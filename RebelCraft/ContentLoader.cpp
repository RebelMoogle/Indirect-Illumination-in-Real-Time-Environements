#include "DXUT.h"
#include "ContentLoader.h"
#include "File.h"
#include "Debug.h"

// ================================================================================
std::string ContentLoader::_ContentPath = "";

// ================================================================================
const std::string& ContentLoader::GetContentPath()
{
	if (_ContentPath == "")
	{
		std::string currDir = File::GetCurrentDir();
		std::string parentDir = File::GetParentDirectory(currDir);		
		std::string parent2Dir = File::GetParentDirectory(parentDir);
		
		_ContentPath = "";
		if (File::DirExists(currDir + "\\CUDA")) _ContentPath = currDir;
		else if (File::DirExists(parentDir + "\\CUDA")) _ContentPath = parentDir;		
		else if (File::DirExists(parent2Dir + "\\CUDA")) _ContentPath = parent2Dir;
	}
	return _ContentPath;
}

bool ContentLoader::LoadOptixProgramFromFile(const std::string& RelativePath, const std::string& EntryFunction, optix::Context OptixContext, optix::Program* Program)
{
	std::string path = GetContentPath() + "\\" + RelativePath;
	if (!File::FileExists(path))
	{
		Debug::PrintError("ContentLoader", "File not found: " + path);
		return false;
	}

	optix::Program prog = OptixContext->createProgramFromPTXFile(path, EntryFunction);
	prog->validate();
	*Program = prog;
	return true;
}