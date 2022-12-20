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
         app.addSkybox("piazza.hdr", "Piazza");
         app.addObjectPBR(
               "Sponza",
               "sponza",
               "Sponza.gltf",
               glm::fvec3(0.0f),
               glm::fvec3(1.0f, -1.555, 1.0f),
               glm::fvec3(1.0f)
         );
         //app.addPointLight(
         //      "Point",
         //      "lightSphere.obj",
         //      glm::fvec3(1.0f),
         //      glm::fvec3(0.0f),
         //      glm::fvec3(0.125f)
         //);
         //app.addSpotLight(
         //      "Spot1",
         //      "lightSphereDefault",
         //      "lightSphere.obj",
         //      glm::fvec3(1.0f),
         //      glm::fvec3(0.0f),
         //      glm::fvec3(0.0f),
         //      glm::fvec3(0.0f),
         //      glm::fvec3(0.125f)
         //);
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

      // Scene 2
      //{
      //   app.addSkybox("Apartment.hdr", "Apartment");
      //   //app.addSkybox("piazza.hdr", "Piazza");
      //   app.addObjectPBR(
      //         "Gun",
      //         "gun",
      //         "gun.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(0.047f, 0.0f, 0.0f),
      //         glm::fvec3(0.093f)
      //   );
      //   //app.addObjectPBR(
      //   //      "DamagedHelmet",
      //   //      "damagedHelmet",
      //   //      "DamagedHelmet.gltf",
      //   //      glm::fvec3(0.0f),
      //   //      glm::fvec3(1.227f, 0.0f, 0.0f),
      //   //      glm::fvec3(1.0f)
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
      
      // Scene 3
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
      //   app.addObjectPBR(
      //         "RockTerrain",
      //         "rockTerrain",
      //         "rockTerrain.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(-0.863f, 1.559f, 1.0f),
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
      
      // Scene 4
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
      //   app.addObjectPBR(
      //         "Village",
      //         "village",
      //         "village.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(-25.087f, -4.969f, 34.783f),
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
   
      
      // Scene 5
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
      //   app.addObjectPBR(
      //         "Fontaine",
      //         "fontaine",
      //         "fontaine.gltf",
      //         glm::fvec3(0.0f, -4.968f, 0.0f),
      //         glm::fvec3(-0.987f, 1.621f, 1.0f),
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
      // Scene 6
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
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

      // Scene 7
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
      //   app.addObjectPBR(
      //         "adamHead",
      //         "adamHead",
      //         "adamHead.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(-1.025f, 1.746f, 1.0f),
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
      // Scene 8
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
      //   app.addObjectPBR(
      //         "floor",
      //         "floor",
      //         "floor.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(-0.739f, 1.584f, 0.752f),
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
      // Scene #10
      //{
      //   app.addSkybox("piazza.hdr", "Piazza");
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
      // Scene 11
      //{
      //   app.addSkybox("Apartment.hdr", "Apartment");
      //   app.addObjectPBR(
      //         "DamagedHelmet",
      //         "damagedHelmet",
      //         "DamagedHelmet.gltf",
      //         glm::fvec3(0.0f),
      //         glm::fvec3(0.0f),
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

      app.run();

   } catch (const std::exception& e)
   {
      std::cerr << e.what() << std::endl;

      return 0;
   }

   return 0;
}
