#include <CroissantRenderer/Shader/shaderManager.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <vulkan/vulkan.h>

std::vector<char> shaderManager::getBinaryDataFromFile(
      const std::string& filename
) {
   // std::ios::ate    -> Starts reading at the end of the file.
   // std::ios::binary -> Reads the file as binary file (avoids text 
   // transformation).
   std::ifstream file(
         std::string(SHADERS_BINARY_DIR) + "/" + filename + ".spv",
         std::ios::ate | std::ios::binary
   );

   if (!file.is_open())
      throw std::runtime_error("Failed to open file: " + filename + ".spv");

   size_t fileSize = (size_t)file.tellg();
   std::vector<char> buffer(fileSize);

   // Seeks back to the beginning of the file.
   file.seekg(0);
   file.read(buffer.data(), fileSize);

   file.close();

   return buffer;
}

VkShaderModule shaderManager::createShaderModule(
      const std::vector<char>& code,
      const VkDevice& logicalDevice
) {
   VkShaderModuleCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = code.size();
   createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

   VkShaderModule shaderModule;
   auto status = vkCreateShaderModule(
         logicalDevice,
         &createInfo,
         nullptr,
         &shaderModule
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create shader module!");

   return shaderModule;
}


void shaderManager::destroyShaderModule(
      VkShaderModule& shaderModule,
      const VkDevice& logicalDevice
) {
   vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
}
