#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>

#include <CroissantRenderer/Renderer.h>

int main()
{
   Renderer app;

   try
   {

      app.demo1();
      app.run();

   } catch (const std::exception& e)
   {

      std::cerr << e.what() << "\n";

      return 0;
   }

   return 0;
}
