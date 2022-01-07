#include "Texture2d.h"
#include "Helper.h"

#define	STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../stb/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

Texture2D* Texture2D::loadFromFile(const char* filename,
	bool useMipmap,
	GLint minFilter, GLint magFilter,
	GLint wrapS, GLint wrapT) {
#ifdef _DEBUG
	SDL_Log("TEXTURE_LOAD: %s (%d, %d, %d %d)", filename, minFilter, magFilter, wrapS, wrapT);
#endif // _DEBUG

	size_t bufSize;
	const char* buf = Helper::readFileContent(filename, &bufSize);

	Texture2D* tex = loadFromMemory(buf, bufSize, useMipmap, minFilter, magFilter, wrapS, wrapT);
	if (tex) {
		delete[] buf;
	}

	return tex;
}

Texture2D* Texture2D::loadFromMemory(const char* buf, int bufSize,
	bool useMipmap,
	GLint minFilter, GLint magFilter,
	GLint wrapS, GLint wrapT) {
	// first, check the buffer?
	if (buf && bufSize) {
		// init libspng context
		bool failed = false;
		unsigned char* decodedImage = NULL;
		int w, h, bpp;

		// decode using stb, vertically flipped for opengl
		stbi_set_flip_vertically_on_load(1);
		decodedImage = stbi_load_from_memory((const unsigned char*)buf, bufSize, &w, &h, &bpp, 0);
		failed = decodedImage == NULL;

		// if we failed, clean up
		if (failed) {
			if (decodedImage)
				stbi_image_free(decodedImage);
			return NULL;
		}
		else {
			// we spawn the real shit
			Texture2D* tex = new Texture2D();

			tex->width = w;
			tex->height = h;
			tex->texData = decodedImage;
			tex->format = bpp == 4 ? GL_RGBA : GL_RGB;

			tex->minFilter = minFilter;
			tex->magFilter = magFilter;
			tex->wrapS = wrapS;
			tex->wrapT = wrapT;
			tex->useMipmap = useMipmap;

			if (tex->upload()) {
				stbi_image_free(decodedImage);
				if (!tex->ownPixelData || !tex->retainPixelData) {
					// zero out
					tex->texData = NULL;
				}
				return tex;
			}

			// we failed
			delete tex;
			return NULL;
		}
	}
	return NULL;
}

bool Texture2D::upload(bool retainPixelData) {
	assert(glGetError() == GL_NO_ERROR);

	if (width && height) {
		if (!texId) {
			createHandle();
			assert(texId != 0);
		}

		assert(glGetError() == GL_NO_ERROR);
#ifdef _DEBUG
		SDL_Log("TEXTURE_LOAD size(%d x %d)", width, height);
#endif
		GLenum error;
		// generate rgba by default, don't ask don't tell?
		glBindTexture(GL_TEXTURE_2D, texId);
		error = glGetError();
		assert(error == GL_NO_ERROR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		error = glGetError();
		assert(error == GL_NO_ERROR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

		error = glGetError();
		assert(error == GL_NO_ERROR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texData);

		assert(glGetError() == GL_NO_ERROR);
		
		if (useMipmap) {
			generateMipMap();
			assert(glGetError() == GL_NO_ERROR);
		}


		// free texture data
		if (!retainPixelData && ownPixelData) {
			delete[] texData;
			texData = NULL;
		}

		return true;
	}

	return false;
}

static unsigned char* sampleSource(unsigned char* src, int srcW, int srcH, int bpp, int x, int y) {
	// clamp coordinate
	if (x < 0) x = 0;
	if (x >= srcW) x = srcW - 1;
	if (y < 0) y = 0;
	if (y >= srcH) y = srcH - 1;

	// index into it
	return &src[(y * srcW + x) * bpp];
}

static void blur(unsigned char* src, int srcW, int srcH, int bpp) {
	const int kernel[3][3] = {
		{1, 2, 1},
		{2, 4, 2},
		{1, 2, 1}
	};
	const int kernelSize = 16;
	// allocate buffer
	unsigned char* tmp = new unsigned char[srcW * srcH * bpp];
	memset(tmp, 0, srcW * srcH * bpp);

	for (int y = 0; y < srcH; y++) {
		for (int x = 0; x < srcW; x++) {
			int pixelId = (y * srcW + x) * bpp;
			unsigned char* pixel = &tmp[pixelId];

			for (int b = 0; b < bpp; b++) {
				unsigned int pixelValue = 0;
				// manual?
				pixelValue += sampleSource(src, srcW, srcH, bpp, x - 1, y - 1)[b] * kernel[0][0];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x    , y - 1)[b] * kernel[0][1];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x + 1, y - 1)[b] * kernel[0][2];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x - 1, y    )[b] * kernel[1][0];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x    , y    )[b] * kernel[1][1];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x + 1, y    )[b] * kernel[1][2];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x - 1, y + 1)[b] * kernel[2][0];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x    , y + 1)[b] * kernel[2][1];
				pixelValue += sampleSource(src, srcW, srcH, bpp, x + 1, y + 1)[b] * kernel[2][2];
				pixelValue /= kernelSize;

				pixel[b] = pixelValue;
			}
		}
	}

	// alter src
	memcpy(src, tmp, srcW * srcH * bpp);

	delete[] tmp;
}

static void downsample(unsigned char* src, int srcW, int srcH, unsigned char* dst, int targetW, int targetH, int bpp) {
	// scan vertically in target image?
	for (int y = 0; y < targetH; ++y) {
		// stride horizontally
		for (int x = 0; x < targetW; ++x) {
			int pixelId = (y * targetW + x) * bpp;
			unsigned char* pixel = &dst[pixelId];
			// for each channel
			unsigned char* tl = sampleSource(src, srcW, srcH, bpp, x << 1, y << 1);
			unsigned char* tr = sampleSource(src, srcW, srcH, bpp, (x << 1) + 1, y << 1);
			unsigned char* br = sampleSource(src, srcW, srcH, bpp, (x << 1) + 1, (y << 1) + 1);
			unsigned char* bl = sampleSource(src, srcW, srcH, bpp, x << 1, (y << 1) + 1);

			for (int c = 0; c < bpp; ++c) {
				// average them, except for alpha maybe? NAH ALL OF EM
				pixel[c] = (
					(tl[c]) + (tr[c]) + (br[c]) + (bl[c])
					) / 4;
			}
		}
	}
}

void Texture2D::generateMipMap() {
	// if there was glGenerateMipmap, use it by default
	if (glGenerateMipmap) {
#ifdef _DEBUG
		SDL_Log("glGenerateMipmap exists!");
#endif
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
#ifdef _DEBUG
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "glGenerateMipmap is not present!");
		SDL_Log("Generating mipmap using stb image resize!");
#endif // DEBUG

		// use our customs shiet
		// use stbir
		int level = 1;
		int currentW = this->width;
		int currentH = this->height;
		int bpp = this->format == GL_RGBA ? 4 : this->format == GL_RGB ? 3 : 1;

		unsigned char* tmpBuffer = NULL; 
		unsigned char* currentBuffer = NULL;
		int targetW = currentW >> 1;
		int targetH = currentH >> 1;
		// stop condition
		bool done = !targetW || !targetH;

		if (!done) {
			tmpBuffer = new unsigned char[currentW * currentH * bpp];
			currentBuffer = new unsigned char[currentW * currentH * bpp];

			// copy current buffer to current data
			memcpy(currentBuffer, this->texData, currentW * currentH * bpp);

			int randomId = rand() % 65000;
			//char randomFilename[64];

			// build until it's done
			while (!done) {
				// fix target size?
#ifdef _DEBUG
				SDL_Log("GENERATE_MIPMAP_LEVEL(%d), size(%d x %d), bpp(%d)", level, targetW, targetH, bpp);
#endif // _DEBUG
				// maybe, recreate the buffer everytime?
				/*if (tmpBuffer) {
					delete[] tmpBuffer;
					tmpBuffer = new unsigned char[targetW * targetH * bpp];
				}*/

				//// resize using target
				/*int ret = stbir_resize_uint8(currentBuffer, currentW, currentH, 0,
					tmpBuffer, targetW, targetH, 0, bpp);*/
				// use our custom resampler?
				downsample(currentBuffer, currentW, currentH, tmpBuffer, targetW, targetH, bpp);
				/*int ret = stbir_resize_uint8_srgb(currentBuffer, currentW, currentH, 0, tmpBuffer, targetW, targetH, 0, bpp,
					bpp > 3, 0);*/

				// write it down?
#ifdef _DEBUG
				/*sprintf(randomFilename, "%d_%dx%d.bmp", randomId, targetW, targetH);
				stbi_flip_vertically_on_write(1);
				assert(stbi_write_bmp(randomFilename, targetW, targetH, bpp, tmpBuffer));*/
#endif

				//assert(ret);

				// resize successful, tmpbuffer is usable
				glTexImage2D(GL_TEXTURE_2D, level, this->format, targetW, targetH, 0,
					this->format, GL_UNSIGNED_BYTE, tmpBuffer);

				// set tmp buffer as current buffer
				memcpy(currentBuffer, tmpBuffer, targetW * targetH * bpp);

				// save current size
				currentW = targetW;
				currentH = targetH;

				// compute target size (which is next size)
				targetH = (targetH >> 1) ;
				targetW = (targetW >> 1) ;

				// increase level
				++level;

				// update done condition
				done = (targetW == 0) && (targetH == targetW);

				// fix size
				targetW = targetW ? targetW : 1;
				targetH = targetH ? targetH : 1;
			}

			// clean up
			if (tmpBuffer)
				delete[] tmpBuffer;
			if (currentBuffer)
				delete[] currentBuffer;
		}

		// set appropriate filter
	}

}