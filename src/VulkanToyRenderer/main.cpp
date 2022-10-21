#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

#include <VulkanToyRenderer/Renderer.h>

int main()
{
   Renderer app;

   try
   {

      // SCENE 1
      {
         app.addSkybox("Town", "SmallTown");
         app.addObjectPBR(
               "Gun",
               "gun.gltf",
               glm::fvec3(0.0f),
               glm::fvec3(0.0f),
               glm::fvec3(0.036f)
         );
         app.addDirectionalLight(
               "DirectionaLight1",
               "lightSphere.obj",
               glm::fvec3(1.0f),
               glm::fvec3(0.0f, 0.678f, 2.881f),
               glm::fvec3(0.0f),
               glm::fvec3(0.125f)
         );
      }
      
      
      app.run();

   } catch (const std::exception& e)
   {
      std::cerr << e.what() << std::endl;

      return 0;
   }

   return 0;
}
