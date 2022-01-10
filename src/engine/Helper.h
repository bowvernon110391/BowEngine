#ifndef __HELPER_H__
#define __HELPER_H__

#include <SDL.h>
#include "../imgui/imgui.h"

namespace Helper {
	// read file content, return char buffer
	char* readFileContent(const char* filename, size_t *filesize = NULL);
	// print matrix values
	void printMatrix(const float* m);
	// calculate pos and size to display inside a specific region
	void computePosAndSize(const ImVec2& region, float imgAspect, ImVec2& pos, ImVec2& size);
	// grab nearest power of two integer?
	int getNearestPOT(int s);
}


#endif