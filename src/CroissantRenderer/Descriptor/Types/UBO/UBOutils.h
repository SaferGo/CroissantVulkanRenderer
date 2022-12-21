#pragma once

#include <memory>

#include <CroissantRenderer/Descriptor/Types/UBO/UBO.h>

namespace UBOutils
{
   void updateUBO(
         const VkDevice& logicalDevice,
         const std::shared_ptr<UBO>& ubo,
         const size_t size,
         void* dataToSend,
         const uint32_t& currentFrame
   );
};
