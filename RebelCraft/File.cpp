#include "DXUT.h"
#include "File.h"
#include "Debug.h"
#include <algorithm>
#include "Util.h"
#include <time.h>
#ifdef WIN32
#include <sys/stat.h>
#include <Windows.h>
#endif

using namespace std;

File::File(const std::string& Path, unsigned int FileOpen) : _Path(Path), _Stream(NULL)
{		
	_Stream = new std::fstream();
	std::ios_base::open_mode tios = 0;
	if ((FileOpen & EFileOpen_Binary) != 0)		
		tios |= ios::binary;
	if ((FileOpen & EFileOpen_Out) != 0 )
		tios |= ios::out;
	else tios |= ios::in;

	if (tios)
		_Stream->open(Path.c_str(), tios);
	else _Stream->open(Path.c_str());

#ifdef _DEBUG
	if (!IsOpen())
		Debug::OutputString(("File not found: " + _Path).c_str());
#endif

	// get length of file:
	_Stream->seekg (0, ios::end);
	_Length = _Stream->tellg();
	_Stream->seekg (0, ios::beg);	
}

File::~File()
{
	if (_Stream) _Stream->close();
	SAFE_DELETE(_Stream);	
}

File* File::CreateTextFile(const std::string& Path)
{
	return new File(Path, EFileOpen_Out);
}

File* File::CreateBinaryFile(const std::string& Path)
{
	return new File(Path, EFileOpen_Out | EFileOpen_Binary);
}

File* File::OpenTextFile(const std::string& Path)
{
	if (File::FileExists(Path))
		return new File(Path);
	else return NULL;
}

File* File::OpenBinaryFile(const std::string& Path)
{
	if (File::FileExists(Path))
		return new File(Path, EFileOpen_Binary);
	else return NULL;
}

/*
bool File::Open()
{
	return true;
}

void File::Close()
{
	_Stream->close();
}
*/
void File::Read(char* Data, int NumChars)
{	
	_Stream->read(Data, NumChars);
}

void File::ReadLine(char* Data, int NumChars)
{	
	_Stream->getline(Data, NumChars);
}

void File::Write(const char* Data, int NumChars)
{
	_Stream->write(Data, NumChars);	
}
/*
void File::Write(const std::string& Text)
{
	_Stream->write(Text.c_str(), Text.length());
}
*/
bool File::Eof() const
{
	return _Stream->eof();
}

bool File::IsOpen() const
{
	return _Stream->is_open();
}

long long File::GetLength() const
{
	return _Length;
}

std::string File::GetCurrentDir()
{
#ifdef WIN32
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);	
	return std::string(path);
#else
	TODO: Get current directory.
#endif
}

std::string File::RemoveExt(const std::string& fileName) 
{
	return fileName.substr(0,fileName.find_last_of("."));
}

bool File::DirExists(const std::string& Path)
{
	struct _stat buffer;
	int iRetTemp = 0;
	memset((void*)&buffer, 0, sizeof(buffer));
	iRetTemp = _stat(Path.c_str(), &buffer);
	if (iRetTemp == 0)
	{
		if (buffer.st_mode & _S_IFDIR)
			return true;
		else return false;
	}
	else return false;
}

bool File::FileExists(const std::string& Path)
{
	struct _stat buffer;
	int iRetTemp = 0;
	memset((void*)&buffer, 0, sizeof(buffer));
	iRetTemp = _stat(Path.c_str(), &buffer);
	if (iRetTemp == 0)
	{
		if (buffer.st_mode & _S_IFMT)
			return true;
		else return false;
	}
	else return false;
}

std::string File::GetExt(const std::string& fileName) 
{
	size_t indexDot = fileName.find_last_of(".");
	size_t indexSlash = MAX(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")));
	if (indexDot == string::npos || (indexSlash != string::npos && indexDot < indexSlash)) return "";

	string ext = fileName.substr(indexDot+1);
	transform(ext.begin(), ext.end(), ext.begin(), tolower);
	return ext;
}

std::string File::GetDirectory(const std::string& fileName) 
{
	string path = fileName.substr(0,MAX(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")))+1);
	transform(path.begin(), path.end(), path.begin(), tolower);
	if (path[path.size() - 1] == '\\')
		path.pop_back();
	return path;
}

std::string File::GetFilename(const std::string& fileName) 
{
	size_t index = MAX(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")))+1;
	string name = fileName.substr(index,fileName.length()-index);
	transform(name.begin(), name.end(), name.begin(), tolower);
	return name;
}

std::string File::GetParentDirectory(const std::string& directory)
{
	std::string dir = directory;
	if (dir[dir.size() - 1] == '\\')
		dir.pop_back();
	return GetDirectory(dir);
}

std::string File::GetExt() const
{
	return GetExt(_Path);
}

std::string File::GetDirectory() const
{
	return GetDirectory(_Path);
}

std::string File::GetFilename() const
{
	return GetFilename(_Path);
}

const std::string& File::GetPath() const
{
	return _Path;
}

std::string File::GetModificationDateFormatted(const std::string& File)
{
	struct stat buf;
	if (!stat(File.c_str(), &buf))
	{
		char timeStr[ 100 ] = "";
		tm thetime;
		localtime_s(&thetime, &buf.st_mtime);
		strftime(timeStr, 100, "%d-%m-%Y %H:%M:%S", &thetime);
		return std::string(timeStr);
	}
	return "NO DATE";
}

std::string File::ResolveRelativePathToOtherFile(const std::string& FullPath, const std::string& RelativePath)
{
	std::string dir = std::string(FullPath.c_str());
	while(dir.size() != 0 && (dir[dir.size()-1] != '/' && dir[dir.size()-1] != '\\'))
		dir.erase(dir.size()-1, 1); // TODO: Test
	while(dir.size() != 0 && (dir[dir.size()-1] == '/' || dir[dir.size()-1] == '\\'))
		dir.erase(dir.size()-1, 1); // TODO: Test

	std::string rel = RelativePath;
	while (rel.size() > 2 && rel[0] == '.' && rel[1] == '.' && (rel[2] == '/' || rel[2] == '\\'))
	{
		std::stringstream s;
		s << rel.substr(3);
		s >> rel;

		while(dir.size() != 0 && (dir[dir.size()-1] != '/' && dir[dir.size()-1] != '\\'))
			dir.erase(dir.size()-1, 1); // TODO: Test

		while(dir.size() != 0 && (dir[dir.size()-1] == '/' || dir[dir.size()-1] == '\\'))
			dir.erase(dir.size()-1, 1); // TODO: Test
	}
	return dir + "\\" + rel;
}

std::string File::FindContent(const std::string& Path)
{
	char currentDirChar[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentDirChar);
	std::string currentDir = std::string(currentDirChar);

	std::string parentDir = currentDir;
	while (parentDir.at(parentDir.length()-1) != '\\')
	{
		parentDir.erase(parentDir.size()-1, 1); // TODO: Test
		if (parentDir.length() == 0)
			break;
	}
	currentDir.append("\\");

	std::string contentDir = currentDir + "Content\\";
	if (File::FileExists(contentDir + Path))
		return contentDir + Path;

	contentDir = parentDir + "Content\\";
	if (File::FileExists(contentDir + Path))
		return contentDir + Path;

	return "";
}

std::string File::FindUniquePrecompileContentPath(const std::string& Path)
{
	char currentDirChar[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, currentDirChar);
	std::string currentDir = std::string(currentDirChar);

	std::string parentDir = currentDir;
	while (parentDir.at(parentDir.length()-1) != L'\\')
	{
		parentDir.erase(parentDir.size()-1, 1); // TODO: Test
		if (parentDir.length() == 0)
			break;
	}
	currentDir.append("\\");

	std::string contentDir = currentDir + "Content\\";
	if (FileExists(contentDir + Path))
	{
		int uid = 0;
		while (FileExists(contentDir + "Compiled\\" + Path + Util::ToStr(uid)))
			uid++;

		CreateDirectoryA(GetDirectory(contentDir + "Compiled\\" + Path + Util::ToStr(uid)).c_str(), NULL);
		return contentDir + "Compiled\\" + Path + Util::ToStr(uid);
	}

	contentDir = parentDir + "Content\\";
	if (FileExists(contentDir + Path))
	{
		int uid = 0;
		while (FileExists(contentDir + "Compiled\\" + Path + Util::ToStr(uid)))
			uid++;
				
		CreateDirectoryA(GetDirectory(contentDir + "Compiled\\" + Path + Util::ToStr(uid)).c_str(), NULL);
		return contentDir + "Compiled\\" + Path + Util::ToStr(uid);
	}
		
	return "";
}

void File::GetLine(std::string& line)
{
	std::getline(*_Stream, line);
}