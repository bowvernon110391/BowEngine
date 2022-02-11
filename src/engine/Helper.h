#ifndef __HELPER_H__
#define __HELPER_H__

#include <SDL.h>
#include "../imgui/imgui.h"
#include <vector>
#include <string>

namespace Helper {
	// read file content, return char buffer
	char* readFileContent(const char* filename, size_t *filesize = NULL);
	// print matrix values
	void printMatrix(const float* m);
	// calculate pos and size to display inside a specific region
	void computePosAndSize(const ImVec2& region, float imgAspect, ImVec2& pos, ImVec2& size);
	// grab nearest power of two integer?
	int getNearestPOT(int s);
	// scan directory for filenames
	int scanDirectory(const std::string& dir, std::vector<std::string>& output, const char** extensions = 0);
	// strip leading directory name
	std::string stripLDirname(const std::string& path, const std::string& dirname);
	// remove all leading dirname from paths
	void stripLDirnames(std::vector<std::string>& paths, const std::string& dirname);
	// copy string with from file buffer (add null terminator)
	std::string stringFromFileBuffer(const char* buf, size_t bufsize);
}


#endif