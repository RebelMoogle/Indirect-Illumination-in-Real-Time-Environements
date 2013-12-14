#ifndef __RES__FILE__
#define __RES__FILE__

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

enum EFileOpen
{	
	EFileOpen_String = 0,
	EFileOpen_Binary = 1,
	EFileOpen_Out = 2
};

class File
{
	public:
		// Constructor. Immdidately opens the file.
		explicit File(const std::string& Path, unsigned int FileOpen = EFileOpen_String);

		// Destructor. Closes the file.
		~File();
		/*
		bool Open();
		void Close(); */

		// Creates a text file and is ready for writing.
		static File* CreateTextFile(const std::string& Path);
		// Creates a binary file and is ready for writing.
		static File* CreateBinaryFile(const std::string& Path);
		// Opens a text file and is ready for reading.
		static File* OpenTextFile(const std::string& Path);
		// Opens a binary file and is ready for reading.
		static File* OpenBinaryFile(const std::string& Path);

		// Read data from the file.
		void Read(char* Data, int NumChars);
		// Read a text line from the file.
		void ReadLine(char* Data, int NumChars);
		
		// Write data to the file.
		void Write(const char* Data, int NumChars);
		// Write native types to file. Be careful... not tested properly...
		template <typename T>
		void WriteText(T Value)
		{
			std::stringstream ss;
			ss << Value;
			std::string str =  ss.str();			
			if (str != "")
			Write(str.c_str(), str.length());
			*_Stream << Value;
		}

		// Checks whether end of file is reached.
		bool Eof() const;
		// Checks whether file is open.
		bool IsOpen() const;
		// Gets the length of the file.
		long long GetLength() const;

		// Gets the filename of this file.
		const std::string& GetPath() const;

		// Gets the extension of this file.
		std::string GetExt() const;
		// Gets the directory of this file.
		std::string GetDirectory() const;
		// Gets the filename of this file.
		std::string GetFilename() const;

		// Gets the current directory.
		static std::string GetCurrentDir();
		// Removes the extension of a string.
		static std::string RemoveExt(const std::string& fileName);
		// Gets the extension of a string.
		static std::string GetExt(const std::string& fileName);		
		// Gets the directory of a string.
		static std::string GetDirectory(const std::string& fileName);
		// Gets the filename of a string.
		static std::string GetFilename(const std::string& fileName);
		// Gets the parent directory.
		static std::string GetParentDirectory(const std::string& directory);

		// Checks whether a directory exists.
		static bool DirExists(const std::string& Path);
		// Checks whether a file exists.
		static bool FileExists(const std::string& Path);
	
		// Gets the modification date in a formatted string. ("%d-%m-%Y %H:%M:%S")
		static std::string GetModificationDateFormatted(const std::string& File);
		
		// Resolves a relative path to another full path and returns the resulting fullpath.
		static std::string ResolveRelativePathToOtherFile(const std::string& FullPath, const std::string& RelativePath);

		// Finds a content item.
		static std::string FindContent(const std::string& Path);

		// Finds a path for a precompiled content item.
		static std::string FindUniquePrecompileContentPath(const std::string& Path);

		// Gets the next line.
		void GetLine(std::string& line);

		template <typename T>
		File& operator <<(const T &obj)
		{
			*_Stream<<obj;
			return *this;
		}

		// function that takes a custom stream, and returns it
		typedef File& (*MyStreamManipulator)(File&);

		// take in a function with the custom signature
		File& operator<<(MyStreamManipulator manip)
		{
			// call the function, and return it's value
			return manip(*this);
		}

		// define the custom endl for this stream.
		// note how it matches the `MyStreamManipulator`
		// function signature
		File& endl(File& stream)
		{
			// print a new line
			*_Stream << std::endl;			
			return stream;
		}

		// this is the type of std::cout
		typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

		// this is the function signature of std::endl
		typedef CoutType& (*StandardEndLine)(CoutType&);

		// define an operator<< to take in std::endl
		File& operator<<(StandardEndLine manip)
		{
			// call the function, but we cannot return it's value
			manip(*_Stream);
			return *this;
		}


	private:
		// Path of the file
		std::string _Path;
		// Stream beneath.
		std::fstream* _Stream;
		// Length of the file.
		long long _Length;
};

#endif