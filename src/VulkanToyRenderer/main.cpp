#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

#include <VulkanToyRenderer/Renderer.h>

/* Commands:
*   
*   - addSkybox(
*        fileName,
*        folderName
*     );
*   - addObjectPBR(
*        name,
*        folderName,
*        fileName,
*        position,
*        rotation,
*        size
*     );
*   - addDirectionalLight(
*        name,
*        folderName,
*        fileName,
*        color,
*        position,
*        targetPosition,
*        size
*     );
*   - addSpotLight(
*        name,
*        folderName,
*        fileName,
*        color,
*        position,
*        targetPosition,
*        rotation,
*        size
*     );
*   - addPointLight(
*        name,
*        folderName,
*        fileName,
*        color,
*        position,
*        size
*     );
*/

int main()
{
   Renderer app;

   try
   {

      // SCENE 1 X
      //{
      //   app.addSkybox("sky.hdr", "DaySky");
      //   app.addObjectPBR(
      //         "Sponza",
      //         "sponza",
      //         "Sponza.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(1.0f, -1.555, 1.0f),
      //         glm::fvec3(1.0f)
      //   );
      //   //app.addPointLight(
      //   //      "Point",
      //   //      "lightSphere.obj",
      //   //      glm::fvec3(1.0f),
      //   //      glm::fvec3(0.0f),
      //   //      glm::fvec3(0.125f)
      //   //);
      //   //app.addSpotLight(
      //   //      "Spot1",
      //   //      "lightSphereDefault",
      //   //      "lightSphere.obj",
      //   //      glm::fvec3(1.0f),
      //   //      glm::fvec3(0.0f),
      //   //      glm::fvec3(0.0f),
      //   //      glm::fvec3(0.0f),
      //   //      glm::fvec3(0.125f)
      //   //);
      //   app.addDirectionalLight(
      //         "Sun",
      //         "lightSphereDefault",
      //         "lightSphere.obj",
      //         glm::fvec3(1.0f),
      //         glm::fvec3(1.0f, 87.0f, -49.0f),
      //         glm::fvec3(1.461f, 2.619f, 57.457f),
      //         glm::fvec3(0.125f)
      //   );
      //}

      // Scene 2 X D
      //{
      //   app.addSkybox("desert.hdr", "Desert");
      //   app.addObjectPBR(
      //         "Gun",
      //         "gun",
      //         "gun.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(0.047f, 0.0f, 0.0f),
      //         glm::fvec3(1.0f)
      //   );
      //   app.addDirectionalLight(
      //         "Sun",
      //         "lightSphereDefault",
      //         "lightSphere.obj",
      //         glm::fvec3(1.0f),
      //         glm::fvec3(1.0f, 87.0f, -49.0f),
      //         glm::fvec3(1.461f, 2.619f, 57.457f),
      //         glm::fvec3(0.125f)
      //   );
      //}
      
      // Scene 5 X
      //{
      //   app.addSkybox("countryClub.hdr", "CountryClub");
      //   app.addObjectPBR(
      //         "Revolver",
      //         "revolver",
      //         "revolver.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(-0.739f, 1.559f, 1.0f),
      //         glm::fvec3(0.863f)
      //   );
      //   app.addDirectionalLight(
      //         "Sun",
      //         "lightSphereDefault",
      //         "lightSphere.obj",
      //         glm::fvec3(1.0f),
      //         glm::fvec3(1.0f, 87.0f, -49.0f),
      //         glm::fvec3(1.461f, 2.619f, 57.457f),
      //         glm::fvec3(0.125f)
      //   );
      //}

      // Scene #10 X
      //{
      //   app.addSkybox("shangai.hdr", "Shangai");
      //   app.addObjectPBR(
      //         "MetalRoughSpheres",
      //         "MetalRoughSpheres",
      //         "MetalRoughSpheres.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(0.278f, 0.193f, -0.056f),
      //         glm::fvec3(1.0f)
      //   );
      //   app.addDirectionalLight(
      //         "Sun",
      //         "lightSphereDefault",
      //         "lightSphere.obj",
      //         glm::fvec3(1.0f),
      //         glm::fvec3(1.0f, 87.0f, -49.0f),
      //         glm::fvec3(1.461f, 2.619f, 57.457f),
      //         glm::fvec3(0.125f)
      //   );
      //}
      // Scene 11 X
      {
         app.addSkybox("neonStudio.hdr", "NeonStudio");
         app.addObjectPBR(
               "DamagedHelmet",
               "damagedHelmet",
               "DamagedHelmet.gltf",
               glm::fvec3(0.0f),
               glm::fvec3(0.0f),
               glm::fvec3(1.0f)
         );
         app.addDirectionalLight(
               "Sun",
               "lightSphereDefault",
               "lightSphere.obj",
               glm::fvec3(1.0f),
               glm::fvec3(1.0f, 87.0f, -49.0f),
               glm::fvec3(1.461f, 2.619f, 57.457f),
               glm::fvec3(0.125f)
         );
      }

      app.run();

   } catch (const std::exception& e)
   {
      std::cerr << e.what() << "\n";

      return 0;
   }

   return 0;
}
