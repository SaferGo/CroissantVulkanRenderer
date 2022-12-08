// Code from:
// https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook/blob/master/shared/UtilsCubemap.h

#include <VulkanToyRenderer/Texture/cubemapUtils.h>

#include <algorithm>
#include <string>

#include <stb/stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

glm::vec3 cubemapUtils::faceCoordsToXYZ(int i, int j, int faceID, int faceSize)
{
	const float A = 2.0f * float(i) / faceSize;
	const float B = 2.0f * float(j) / faceSize;

	if (faceID == 0) return glm::vec3(-1.0f, A - 1.0f, B - 1.0f);
	if (faceID == 1) return glm::vec3(A - 1.0f, -1.0f, 1.0f - B);
	if (faceID == 2) return glm::vec3(1.0f, A - 1.0f, 1.0f - B);
	if (faceID == 3) return glm::vec3(1.0f - A, 1.0f, 1.0f - B);
	if (faceID == 4) return glm::vec3(B - 1.0f, A - 1.0f, 1.0f);
	if (faceID == 5) return glm::vec3(1.0f - B, A - 1.0f, -1.0f);

	return glm::vec3();
}

Bitmap cubemapUtils::convertEquirectangularMapToVerticalCross(const Bitmap& b)
{
	if (b.type_ != eBitmapType_2D) return Bitmap();

	const int faceSize = b.w_ / 4;

	const int w = faceSize * 3;
	const int h = faceSize * 4;

	Bitmap result(w, h, b.comp_, b.fmt_);

	const glm::ivec2 kFaceOffsets[] =
	{
      glm::ivec2(faceSize, faceSize * 3),
		glm::ivec2(0, faceSize),
		glm::ivec2(faceSize, faceSize),
		glm::ivec2(faceSize * 2, faceSize),
		glm::ivec2(faceSize, 0),
		glm::ivec2(faceSize, faceSize * 2)
	};

	const int clampW = b.w_ - 1;
	const int clampH = b.h_ - 1;

	for (int face = 0; face != 6; face++)
	{
		for (int i = 0; i != faceSize; i++)
		{
			for (int j = 0; j != faceSize; j++)
			{
				const glm::vec3 P = faceCoordsToXYZ(i, j, face, faceSize);
				const float R = std::hypot(P.x, P.y);
				const float theta = std::atan2(P.y, P.x);
				const float phi = std::atan2(P.z, R);
				//	float point source coordinates
				const float Uf = float(
                  2.0f *
                  faceSize *
                  (theta + glm::pi<float>()) / glm::pi<float>()
            );
				const float Vf = float(
                  2.0f *
                  faceSize *
                  (glm::pi<float>() / 2.0f - phi) / glm::pi<float>()
            );
				// 4-samples for bilinear interpolation
				const int U1 = std::clamp(int(std::floor(Uf)), 0, clampW);
				const int V1 = std::clamp(int(std::floor(Vf)), 0, clampH);
				const int U2 = std::clamp(U1 + 1, 0, clampW);
				const int V2 = std::clamp(V1 + 1, 0, clampH);
				// fractional part
				const float s = Uf - U1;
				const float t = Vf - V1;
				// fetch 4-samples
				const glm::vec4 A = b.getPixel(U1, V1);
				const glm::vec4 B = b.getPixel(U2, V1);
				const glm::vec4 C = b.getPixel(U1, V2);
				const glm::vec4 D = b.getPixel(U2, V2);
				// bilinear interpolation
				const glm::vec4 color = (
                  A * (1 - s) * (1 - t) +
                  B * (s) * (1 - t) +
                  C * (1 - s) * t +
                  D * (s) * (t)
            );
				result.setPixel(
                  i + kFaceOffsets[face].x, j + kFaceOffsets[face].y, color
            );
			}
		};
	}

	return result;
}

Bitmap cubemapUtils::convertVerticalCrossToCubeMapFaces(const Bitmap& b)
{
	const int faceWidth = b.w_ / 3;
	const int faceHeight = b.h_ / 4;

	Bitmap cubemap(faceWidth, faceHeight, 6, b.comp_, b.fmt_);
	cubemap.type_ = eBitmapType_Cube;

	const uint8_t* src = b.data_.data();
	uint8_t* dst = cubemap.data_.data();

	/*
			------
			| +Y |
	 ----------------
	 | -X | -Z | +X |
	 ----------------
			| -Y |
			------
			| +Z |
			------
	*/

	const int pixelSize = (
         cubemap.comp_ * Bitmap::getBytesPerComponent(cubemap.fmt_)
   );

	for (int face = 0; face != 6; ++face)
	{
		for (int j = 0; j != faceHeight; ++j)
		{
			for (int i = 0; i != faceWidth; ++i)
			{
				int x = 0;
				int y = 0;

				switch (face)
				{
					// POSITIVE_X
				case 0:
					x = i;
					y = faceHeight + j;
					break;

					// NEGATIVE_X
				case 1:
					x = 2 * faceWidth + i;
					y = 1 * faceHeight + j;
					break;

					// POSITIVE_Y
				case 2:
					x = 2 * faceWidth - (i + 1);
					y = 1 * faceHeight - (j + 1);
					break;

					// NEGATIVE_Y
				case 3:
					x = 2 * faceWidth - (i + 1);
					y = 3 * faceHeight - (j + 1);
					break;

					// POSITIVE_Z
				case 4:
					x = 2 * faceWidth - (i + 1);
					y = b.h_ - (j + 1);
					break;

					// NEGATIVE_Z
				case 5:
					x = faceWidth + i;
					y = faceHeight + j;
					break;
				}

				memcpy(dst, src + (y * b.w_ + x) * pixelSize, pixelSize);

				dst += pixelSize;
			}
		}
	}

	return cubemap;
}

void cubemapUtils::float24to32(
      const int w,
      const int h,
      const float* img24,
      float* img32
) {
   const int numPixels = w * h;

	for (int i = 0; i != numPixels; i++)
	{
		*img32++ = *img24++;
		*img32++ = *img24++;
		*img32++ = *img24++;
		*img32++ = 1.0f;
	}
}

float cubemapUtils::radicalInverse_VdC(uint32_t bits)
{
   bits = (bits << 16u) | (bits >> 16u);
   bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
   bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
   bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
   bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

   return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

// Generates Points on the Hemisphere
// From: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
glm::fvec2 cubemapUtils::hammersley2d(const uint32_t i, const uint32_t N)
{
   return glm::fvec2(float(i) / float(N), radicalInverse_VdC(i));
}

void cubemapUtils::convolveDiffuse(
      const glm::fvec3* data,
      const int width,
      const int height,
      const int dstWidth,
      const int dstHeight,
      std::vector<glm::fvec3>& outPixels,
      const int nSamples
) {
   if (width != 2 * height)
      throw std::runtime_error(
            "The width of the HDRI image needs to be twice the height."
      );

   std::vector<glm::fvec3> tmp(dstWidth * dstHeight);
   stbir_resize_float_generic(
         reinterpret_cast<const float*>(data), width, height, 0,
         reinterpret_cast<float*>(tmp.data()), dstWidth, dstHeight, 0, 3,
         STBIR_ALPHA_CHANNEL_NONE,
         0,
         STBIR_EDGE_CLAMP,
         STBIR_FILTER_CUBICBSPLINE,
         STBIR_COLORSPACE_LINEAR,
         nullptr
   );

   // Cache data
   const float PI = glm::pi<float>();
   const float PI2 = PI * PI;

   const glm::fvec3* pixels = tmp.data();
   for (int y = 0; y != dstHeight; y++)
   {
      const float theta1 = float(y) / float(dstHeight) * PI;
      for (int x = 0; x != dstWidth; x++)
      {
         const float phi1 = float(x) / float(dstWidth) * PI2;
         const glm::fvec3 V1 = glm::fvec3(
               glm::sin(theta1) * glm::cos(phi1),
               glm::sin(theta1) * glm::sin(phi1),
               glm::cos(theta1)
         );

         glm::fvec3 color = glm::fvec3(0.0f);
         float weight = 0.0f;
         for (int i = 0; i != nSamples; i++)
         {
            const glm::fvec2 h = hammersley2d(i, nSamples);
            const int x1 = int(glm::floor(h.x * dstWidth));
            const int y1 = int(glm::floor(h.y * dstHeight));
            const float theta2 = float(y1) / float(dstHeight) * PI;
            const float phi2 = float(x1) / float(dstWidth) * PI2;
            const glm::fvec3 V2 = glm::fvec3(
                  glm::sin(theta2) * glm::cos(phi2),
                  glm::sin(theta2) * glm::sin(phi2),
                  glm::cos(theta2)
            );
            const float D = std::max(0.0f, glm::dot(V1, V2));

            if (D > 0.01f)
            {
               color += pixels[y1 * dstWidth + x1] * D;
               weight += D;
            }

            outPixels[y * dstWidth + x] = color / weight;
         }
      }
   }
}

void cubemapUtils::createIrradianceHDR(
      const float* img,
      const int texWidth,
      const int texHeight,
      const std::string pathToFile
) {
   const int dstWidth = 256;
   const int dstHeight = 128;
   const int nSamples = 1024;

   std::vector<glm::fvec3> outPixels(dstWidth * dstHeight);
   convolveDiffuse(
         (glm::vec3*)img,
         texWidth,
         texHeight,
         dstWidth,
         dstHeight,
         outPixels,
         nSamples
   );

   stbi_write_hdr(
         pathToFile.c_str(),
         dstWidth,
         dstHeight,
         3,
         (float*)outPixels.data()
   );
}
