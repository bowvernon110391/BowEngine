#include "Helper.h"
#include <stdio.h>
#include "tinydir.h"

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

int Helper::getNearestPOT(int s)
{
    int pot = 1;  // start from one
    while (pot < s) {
        pot = pot << 1;
    }
    return pot;
}

int Helper::scanDirectory(const std::string& dir, std::vector<std::string>& output, const char** extensions) {

    const std::string forbidden(".");
    const std::string forbidden2("..");

    tinydir_dir tmpDir;
    tinydir_open(&tmpDir, dir.c_str());

    while (tmpDir.has_next) {
        tinydir_file f;
        tinydir_readfile(&tmpDir, &f);

        // if it's directory, recurse. otherwise? read more?
        // printf("name[%s], path[%s], ext[%s], ispath(%d)\n", f.name, f.path, f.extension, f.is_dir);

        std::string fname(f.name);

        // skip forbidden filename
        if (fname == forbidden || fname == forbidden2) {
            tinydir_next(&tmpDir);
            continue;
        }

        // recurse if we find a directory
        if (f.is_dir) {
            Helper::scanDirectory(std::string(f.path), output, extensions);
        } else {
            // add to output the path, if the extension matches?
            if (!extensions) {
                // no extensions provided?
                // directly add
                output.push_back(std::string(f.path));
            } else {
                // check for supported extension
                int i=0;
                while ( true ) {
                    const char* ext = extensions[i];

                    if (!ext)
                        break;

                    // check if it's supported, add after we find a match
                    std::string sext = std::string(ext);
                    if (f.extension == sext) {
                        output.push_back(std::string(f.path));
                        break;
                    }

                    ++i;
                };
            }
        }

        tinydir_next(&tmpDir);
    }

    return 0;
}

std::string Helper::stripLDirname(const std::string& path, const std::string& dirname) {
    // if path is found in the first name, remove it?
    auto pos = path.find_first_of(dirname + "/");   // unix
    auto pos2 = path.find_first_of(dirname + "\\"); // windows
    auto pos3 = path.find_first_of(dirname + ":");  // retardos

    int offset = dirname.length() + 1;

    if (pos != std::string::npos) {
        return path.substr(pos + offset);
    } else if (pos2 != std::string::npos) {
        return path.substr(pos2 + offset);
    } else if (pos3 != std::string::npos) {
        return path.substr(pos3 + offset);
    }
    return std::string(path);
}

void Helper::stripLDirnames(std::vector<std::string>& paths, const std::string& dirname) {
    for (int i=0; i<paths.size(); i++) {
        paths[i] = Helper::stripLDirname(paths[i], dirname);
    }
}