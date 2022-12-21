#pragma once

#include <vector>
#include <string>
#include <memory>

#include <CroissantRenderer/Image/Image.h>
#include <CroissantRenderer/Texture/Texture.h>
#include <CroissantRenderer/Descriptor/DescriptorSets.h>
#include <CroissantRenderer/Model/Attributes.h>

template<typename T>
struct Mesh
{
   // Vertex
   std::vector<T>                         vertices;
   std::vector<uint32_t>                  indices;

   VkBuffer                               vertexBuffer;
   VkBuffer                               indexBuffer;
   VkDeviceMemory                         vertexMemory;
   VkDeviceMemory                         indexMemory;

   std::vector<std::shared_ptr<Texture>>  textures;
   std::vector<TextureToLoadInfo>         texturesToLoadInfo;

   // (One descriptor set for all the ubo and samplers of a mesh)
   // (The same descriptor set for each frame in flight)
   DescriptorSets                         descriptorSets;
};

