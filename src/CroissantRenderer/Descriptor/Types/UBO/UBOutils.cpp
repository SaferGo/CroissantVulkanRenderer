#include <CroissantRenderer/Descriptor/Types/UBO/UBOutils.h>

#include <cstring>

#include <CroissantRenderer/Descriptor/Types/UBO/UBO.h>

void UBOutils::updateUBO(
      const VkDevice& logicalDevice,
      const std::shared_ptr<UBO>& ubo,
      const size_t size,
      void* dataToSend,
      const uint32_t& currentFrame
) {
   void* data;
   vkMapMemory(
         logicalDevice,
         ubo->getMemory(currentFrame),
         0,
         size,
         0,
         &data
   );
      memcpy(data, dataToSend, size);
   vkUnmapMemory(
         logicalDevice,
         ubo->getMemory(currentFrame)
   );
}
