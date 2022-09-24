#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

#include <VulkanToyRenderer/Renderer.h>

int main()
{
   Renderer app;

   try
   {
      app.addModel("Bunny", "stanford-bunny.obj");
      app.addModel("Light", "lightSphere.obj");
      //app.addModel("Erato", "erato.obj", "erato.jpg");
      app.addModel("Viking room", "viking_room.obj", "viking_room.png");
      app.run();
   } catch (const std::exception& e)
   {
      std::cerr << e.what() << std::endl;

      return 0;
   }

    return 0;
}
