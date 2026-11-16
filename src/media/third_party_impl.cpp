// The two single-header libraries used by the meme branch, compiled once here.
//   stb_image  (public domain, Sean Barrett) - decodes JPEG / PNG files
//   miniaudio  (public domain, David Reid)   - plays sound on macOS / Linux / Windows
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#include "stb_image.h"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "miniaudio.h"
