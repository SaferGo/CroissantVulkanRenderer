#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

#include <VulkanToyRenderer/Renderer.h>

int main()
{
   Renderer app;

   try
   {
      // Adds the light and normal models.
      
      /*
       * Normal Model -> Model that interacts with light.
       * Light Model  -> Model that produces light.
       */

      //app.addObject("Bunny", "stanford-bunny.obj");
      app.addSkybox("Town", "SmallTown");
      app.addObjectPBR("Gun", "gun.gltf");
      app.addDirectionalLight("DirectionaLight1", "lightSphere.obj");
      
      
      app.run();

   } catch (const std::exception& e)
   {
      std::cerr << e.what() << std::endl;

      return 0;
   }

   return 0;
}
