#include "Helper.h"
#include <stdio.h>
/// <summary>
/// readFileContent: reads the bytes of a file (from assets folder supposedly)
/// </summary>
/// <param name="filename">the filename relative to assets folder</param>
/// <param name="outFilesize">pointer to size of the read buffer</param>
/// <returns></returns>
char* Helper::readFileContent(const char* filename, size_t *outFilesize) {
    SDL_RWops *io = SDL_RWFromFile(filename, "rb");
    if (!io) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed opening '%s' : %s", filename, SDL_GetError());
        return NULL;
    }

    int64_t filesize = SDL_RWsize(io);

#ifdef _DEBUG
	SDL_Log("reading content of %s, (%u) bytes", filename, filesize);
#endif

    char *bytes = new char[filesize];

    if (!bytes) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed allocating %u bytes of memory when reading file '%s'", 
            filesize, filename);
        return NULL;
    }

    // gotta keep reading until we got all we need
    size_t total_read = 0, read_ret = 1;

    while (total_read < filesize && read_ret != 0) {
        read_ret = SDL_RWread(io, &bytes[total_read], 1, (filesize-total_read));
        total_read += read_ret;

#ifdef _DEBUG
		SDL_Log("last_read : %u, total_read: %u", read_ret, total_read);
#endif
    }

    if (total_read != filesize) {
        SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "[%s] : Try to read %d bytes, only got %d. Bailing...", filename, filesize, total_read);
        delete [] bytes;
        return NULL;
    }

	// close the file
	SDL_RWclose(io);

    if (outFilesize) {
        *outFilesize = filesize;
    }

    return bytes;
}

void Helper::printMatrix(const float* m)
{
    for (int i = 0; i < 4; i++) {
        printf("\t%.2f %.2f %.2f %.2f\n", m[i], m[i + 4], m[i + 8], m[i + 12]);
    }
}

void Helper::computePosAndSize(const ImVec2& region, float imgAspect, ImVec2& pos, ImVec2& size)
{
    // compute region aspect
    if (region.y <= 0.0f) {
        return;
    }
    float regionAspect = region.x / region.y;

    // either we adjust x or y only
    if (regionAspect > imgAspect) {
        // adjust the x only
        // pos.y is zero
        size.y = region.y;
        pos.y = 0;

        // compute size x, which is basically imgAspect * region.y
        size.x = imgAspect * size.y;
        pos.x = (region.x - size.x) * 0.5f;
    }
    else {
        // adjust the y only then
        size.x = region.x;
        pos.x = 0;

        // compute size y, which is basically imgAspect * size.x
        size.y = (1.0f/imgAspect) * size.x;
        pos.y = (region.y - size.y) * 0.5f;
    }
}
