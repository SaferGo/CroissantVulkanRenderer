#pragma once

// Code from:
// https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook/blob/master/shared/UtilsCubemap.h

#include <string>

#include <VulkanToyRenderer/Texture/Bitmap.h>

namespace cubemapUtils
{
   Bitmap convertEquirectangularMapToVerticalCross(const Bitmap& b);
   Bitmap convertVerticalCrossToCubeMapFaces(const Bitmap& b);
   glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);
   void float24to32(
      const int w,
      const int h,
      const float* img24,
      float* img32
   );
   void createIrradianceHDR(
         const float* img,
         const int texWidth,
         const int texHeight,
         const std::string pathToFile
   );
   void convolveDiffuse(
         const glm::fvec3* data,
         const int width,
         const int height,
         const int dstWidth,
         const int dstHeight,
         std::vector<glm::fvec3>& outPixels,
         const int nSamples
   );
   glm::fvec2 hammersley2d(const uint32_t i, const uint32_t N);
   float radicalInverse_VdC(uint32_t bits);

};
