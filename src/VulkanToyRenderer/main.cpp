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
      //app.addLight(
      //      "Light1",
      //      "lightSphere.obj",
      //      glm::fvec4(0.0f, 0.0f, 1.0f, 1.0f)
      //);
      //app.addLight(
      //      "Light2",
      //      "lightSphere.obj",
      //      glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f)
      //);

      //app.addObject(
      //      "Viking room",
      //      "viking_room.obj",
      //      "viking_room.png"
      //);
      
      
      app.run();

   } catch (const std::exception& e)
   {
      std::cerr << e.what() << std::endl;

      return 0;
   }

   return 0;
}
