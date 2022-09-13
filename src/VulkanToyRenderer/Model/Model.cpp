#include <VulkanToyRenderer/Model/Model.h>

#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <VulkanToyRenderer/Model/Vertex.h>

// REMEMBER: LoadObj automatically applies triangularization by default!


/*
 * Approach taken from https://en.cppreference.com/w/cpp/utility/hash.
 * This helps us to use std::unordered_map to avoid vertex duplication.
 */
namespace std {
   template<> struct hash<Vertex>
   {
      size_t operator()(Vertex const& vertex) const
      {
         return (
               (hash<glm::vec3>()(vertex.pos) ^
               (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1
         );
      }
   };
};

void Model::readModel(const char* pathToMesh)
{
   // Holds all the positions, normals and texture coordinates.
   tinyobj::attrib_t attrib;
   // Contains all the objects and their faces.
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;

   std::string warn, err;

   auto status = tinyobj::LoadObj(
         &attrib,
         &shapes,
         &materials,
         &warn,
         &err,
         pathToMesh
   );

   if (!status)
      throw std::runtime_error(warn + err);

   std::unordered_map<Vertex, uint32_t> uniqueVertices{};

   for (const auto& shape : shapes)
   {
      for (const auto& index : shape.mesh.indices)
      {
         Vertex vertex{};

         vertex.pos = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]
         };

         vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            // Flips the vertical component.
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
         };

         // In this case de mesh is already iluminated in the texture.
         vertex.color = {1.0f, 1.0f, 1.0f};

         // Avoids vertex duplication
         if (uniqueVertices.count(vertex) == 0)
         {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
         }

         indices.push_back(uniqueVertices[vertex]);
      }
   }
}

