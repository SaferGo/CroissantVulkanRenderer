#include <VulkanToyRenderer/MeshLoader/Vertex.h>

#include <vulkan/vulkan.h>

#include <array>

/*
 * Describes at which rate to load data from memory through the vertices. It
 * specifies the number of bytes between data entries and whether to move to
 * the next data entry after each vertex or after each instance.
 */
VkVertexInputBindingDescription Vertex::getBindingDescription()
{
   VkVertexInputBindingDescription bindingDescription{};
   // Index of the binding in the array of bindings(to connect with the
   // attribute descriptions).
   bindingDescription.binding = 0;
   // Specifies the number of bytes from one entry to the next.
   bindingDescription.stride = sizeof(Vertex);
   // -VK_VERTEX_INPUT_RATE_VERTEX = Move to the next data entry after each
   //                                vertex(vertex rendering).
   // -VK_VERTEX_INPUT_RATE_INSTANCE = Move to the next data entry after each
   //                                  instance(intance rendering).
   bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   return bindingDescription;
}

/*
 * Describes how to extract a vertex attribute from a chunk of vertex data
 * originated from a binfing description.
 */
std::array<VkVertexInputAttributeDescription, 2> 
   Vertex::getAttributeDescriptions() 
{
   std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

   // -Vertex Attribute: Position

   // Tells Vulkan which binding the per-vertex data comes.
   attributeDescriptions[0].binding = 0;
   // References the location directive of the input in the vertex shader.
   attributeDescriptions[0].location = 0;
   // Format -> vec3
   attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
   attributeDescriptions[0].offset = offsetof(Vertex, pos);

   // -Vertex Attribute: Color
   
   attributeDescriptions[1].binding = 0;
   attributeDescriptions[1].location = 1;
   attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[1].offset = offsetof(Vertex, color);

   return attributeDescriptions;
}

