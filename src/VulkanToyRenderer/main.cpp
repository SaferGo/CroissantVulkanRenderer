#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

#include <VulkanToyRenderer/HelloTriangleApp.h>

int main()
{
   HelloTriangleApp app;

   try
   {
      app.run();
   } catch (const std::exception& e)
   {
      std::cerr << e.what() << std::endl;

      return 0;
   }

    return 0;
}
