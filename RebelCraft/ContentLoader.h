#ifndef __CONTENT_LOADER__
#define __CONTENT_LOADER__

class ContentLoader
{
	public:
	
		// Loads an optix program from file.
		static bool LoadOptixProgramFromFile(const std::string& RelativePath, const std::string& EntryFunction, optix::Context OptixContext, optix::Program* Program);

		// Gets the content path.
		static const std::string& GetContentPath();

	private:
		
		// Stores the content path.
		static std::string _ContentPath;

};

#endif