#include <VulkanToyRenderer/Model/Attributes.h>

#include <vulkan/vulkan.h>

#include <vector>

///////////////////////////////////PBR/////////////////////////////////////////

/*
 * Describes at which rate to load data from memory through the vertices. It
 * specifies the number of bytes between data entries and whether to move to
 * the next data entry after each vertex or after each instance.
 */
VkVertexInputBindingDescription Attributes::PBR::getBindingDescription()
{
   VkVertexInputBindingDescription bindingDescription{};
   // Index of the binding in the array of bindings(to connect with the
   // attribute descriptions).
   bindingDescription.binding = 0;
   // Specifies the number of bytes from one entry to the next.
   bindingDescription.stride = sizeof(PBR::Vertex);
   // -VK_VERTEX_INPUT_RATE_VERTEX = Move to the next data entry after each
   //                                vertex(vertex rendering).
   // -VK_VERTEX_INPUT_RATE_INSTANCE = Move to the next data entry after each
   //                                  instance(intance rendering).
   bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   return bindingDescription;
}

/*
 * Describes how to extract a vertex attribute from a chunk of vertex data
 * originated from a binding description.
 */
std::vector<VkVertexInputAttributeDescription> 
   Attributes::PBR::getAttributeDescriptions() 
{
   std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

   // -Vertex Attribute: Position

   // Tells Vulkan which binding the per-vertex data comes.
   attributeDescriptions[0].binding = 0;
   // References the location directive of the input in the vertex shader.
   attributeDescriptions[0].location = 0;
   // Format -> vec3
   attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[0].offset = offsetof(PBR::Vertex, pos);

   // -Vertex Attribute: Color
   
   attributeDescriptions[1].binding = 0;
   attributeDescriptions[1].location = 1;
   attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[1].offset = offsetof(PBR::Vertex, color);

   // - Vertex Attribute: Texture coord.

   attributeDescriptions[2].binding = 0;
   attributeDescriptions[2].location = 2;
   attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
   attributeDescriptions[2].offset = offsetof(PBR::Vertex, texCoord);

   // - Vertex Attribute: Normal

   attributeDescriptions[3].binding = 0;
   attributeDescriptions[3].location = 3;
   attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[3].offset = offsetof(PBR::Vertex, normal);
   
   return attributeDescriptions;
}

////////////////////////////////////SKYBOX/////////////////////////////////////
/*
 * Describes at which rate to load data from memory through the vertices. It
 * specifies the number of bytes between data entries and whether to move to
 * the next data entry after each vertex or after each instance.
 */
VkVertexInputBindingDescription Attributes::SKYBOX::getBindingDescription()
{
   VkVertexInputBindingDescription bindingDescription{};
   // Index of the binding in the array of bindings(to connect with the
   // attribute descriptions).
   bindingDescription.binding = 0;
   // Specifies the number of bytes from one entry to the next.
   bindingDescription.stride = sizeof(SKYBOX::Vertex);
   // -VK_VERTEX_INPUT_RATE_VERTEX = Move to the next data entry after each
   //                                vertex(vertex rendering).
   // -VK_VERTEX_INPUT_RATE_INSTANCE = Move to the next data entry after each
   //                                  instance(intance rendering).
   bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   return bindingDescription;
}

/*
 * Describes how to extract a vertex attribute from a chunk of vertex data
 * originated from a binding description.
 */
std::vector<VkVertexInputAttributeDescription> 
   Attributes::SKYBOX::getAttributeDescriptions() 
{
   std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

   // -Vertex Attribute: Position

   // Tells Vulkan which binding the per-vertex data comes.
   attributeDescriptions[0].binding = 0;
   // References the location directive of the input in the vertex shader.
   attributeDescriptions[0].location = 0;
   // Format -> vec3
   attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[0].offset = offsetof(SKYBOX::Vertex, pos);

   return attributeDescriptions;
}
