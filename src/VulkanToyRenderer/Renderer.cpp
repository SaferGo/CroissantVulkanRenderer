#include <VulkanToyRenderer/Renderer.h>

#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <limits>
#include <algorithm>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Settings/vLayersConfig.h>
#include <VulkanToyRenderer/Window/WindowManager.h>
#include <VulkanToyRenderer/ValidationLayers/vlManager.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/ShaderManager/shaderManager.h>
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipelineManager.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>
#include <VulkanToyRenderer/Extensions/extensionsUtils.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Buffers/bufferUtils.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/UBO.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/DepthBuffer/DepthBuffer.h>

float rotX = 1.0f;
float rotY = 1.0f;
float rotZ = 1.0f;

float movX = 1.0f;
float movY = 1.0f;
float movZ = 1.0f;

void Renderer::initImgui()
{
   // -Descriptor Pool

   // (calculates the total size of the pool depending of the descriptors
   // we send as parameter and the number of descriptor SETS defined)
   m_descriptorPoolImgui.createDescriptorPool(
         m_device.getLogicalDevice(),
         // Type of descriptors / Count of each type of descriptor in the pool.
         {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
         },
         // Descriptor SETS count.
         // (11 -> count of all the descriptor types)
         1000 * 11
   );

   // -RenderPass

   VkAttachmentDescription attachment = {};
   attachment.format = m_swapchain.getImageFormat();
   attachment.samples = VK_SAMPLE_COUNT_1_BIT;
   // Tells Vulkan to not clear the content of the framebuffer but to draw
   // over it instead.
   // (because we want the GUI to be drawn over our main rendering)
   attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
   attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   // We don't care about stencil in Imgui.
   attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   // We want optimal performance because we are going to draw some stuff.
   attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   // Specifies that this render pass is the last one(because we want to draw 
   // it over all the other render passes).
   attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   // Specifies the actual color that our render pass needs.
   VkAttachmentReference colorAttachment = {};
   colorAttachment.attachment = 0;
   // As we described above, the layout we're using to draw is
   // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
   colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   // -Subpass
   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorAttachment;

   // -Synch. between this render pass and the one from the renderer.
   VkSubpassDependency dependency = {};
   // To create a dependency outside the current render pass.
   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   // Refers to our first and only subpass by its index.
   dependency.dstSubpass = 0;
   // Here we have to state what we're watining for.
   // Before drawing the GUI, we want our geometry to be already renderer. That
   // means we want the pixels to be already written to the framebuffer(that's
   // why we use VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
   dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   // Here we state when we want to draw(also the same thing that we are
   // waiting in srcStageMask).
   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   // 0 means "nothing". As in, the ir no memory dependency the barrier
   // introduces. Implicit sync. means Vulkan does it for us.
   dependency.srcAccessMask = 0;
   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

   VkRenderPassCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   info.attachmentCount = 1;
   info.pAttachments = &attachment;
   info.subpassCount = 1;
   info.pSubpasses = &subpass;
   info.dependencyCount = 1;
   info.pDependencies = &dependency;

   auto status = vkCreateRenderPass(
         m_device.getLogicalDevice(),
         &info,
         nullptr,
         &m_renderPassImgui
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create Imgui's render pass");

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   (void)io;

   ImGui::StyleColorsDark();

   ImGui_ImplGlfw_InitForVulkan(m_windowM.getWindow(), true);
   ImGui_ImplVulkan_InitInfo initInfo = {};
   initInfo.Instance = m_vkInstance;
   initInfo.PhysicalDevice = m_device.getPhysicalDevice();
   initInfo.Device = m_device.getLogicalDevice();
   initInfo.QueueFamily = m_qfIndices.graphicsFamily.value();
   initInfo.Queue = m_qfHandles.graphicsQueue;
   initInfo.PipelineCache = VK_NULL_HANDLE;
   initInfo.DescriptorPool = m_descriptorPoolImgui.getDescriptorPool();
   initInfo.Allocator = nullptr;
   initInfo.MinImageCount = m_swapchain.getMinImageCount();
   initInfo.ImageCount = m_swapchain.getImageCount();
   initInfo.CheckVkResultFn = nullptr;
   ImGui_ImplVulkan_Init(&initInfo, m_renderPassImgui);
   

   // -Creation of command buffers and command pool
   // Improve this!
   m_commandPoolImgui.createCommandPool(
         m_device.getLogicalDevice(),
         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         m_qfIndices
   );
   
   // (CHANGE THIS -> BEGIN COMMAND BUFFER TO UTILS! IT HAST NOTHING TO DO
   // WITH COMMANDPOOL.h)
   // (one time usage command buffer...)
   m_commandBuffersImgui.resize(config::MAX_FRAMES_IN_FLIGHT);
   m_commandPoolImgui.allocCommandBuffer(m_commandBuffersImgui[0]);
   m_commandPoolImgui.allocCommandBuffer(m_commandBuffersImgui[1]);
   {
      // -Uploading the fonts to the GPU
      // (one time command buffer)
      // We can just use any commandBuffer.
      VkCommandBuffer newCommandBuffer;

      m_commandPoolImgui.allocCommandBuffer(newCommandBuffer);
      m_commandPoolImgui.beginCommandBuffer(
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            newCommandBuffer
      );

         ImGui_ImplVulkan_CreateFontsTexture(newCommandBuffer);

      m_commandPoolImgui.endCommandBuffer(newCommandBuffer);
      m_commandPoolImgui.submitCommandBuffer(
            m_qfHandles.graphicsQueue,
            newCommandBuffer
      );
   }

   // FrameBuffer
   {
      m_framebuffersImgui.resize(m_swapchain.getImageCount());

      VkImageView attachment[1];
      VkFramebufferCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      info.renderPass = m_renderPassImgui;
      info.attachmentCount = 1;
      info.pAttachments = attachment;
      info.width = m_swapchain.getExtent().width;
      info.height = m_swapchain.getExtent().height;
      // The layers is 1 because our imageViews are single images and not
      // arrays.
      info.layers = 1;
      for (uint32_t i = 0; i < m_swapchain.getImageCount(); i++)
      {
         attachment[0] = m_swapchain.getImageView(i);
         vkCreateFramebuffer(
               m_device.getLogicalDevice(),
               &info,
               nullptr,
               &m_framebuffersImgui[i]
         );
      }
   }
}

void Renderer::run()
{
// Improve this!
// NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
clearValues.resize(2);
clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
clearValues[1].color = {1.0f, 0.0f};


   if (m_models.size() == 0)
   {
      std::cout << "Nothing to render..\n";
      return;
   }

   m_windowM.createWindow(
         config::RESOLUTION_W,
         config::RESOLUTION_H,
         config::TITLE
   );
   initVK();
   initImgui();
   mainLoop();
   cleanup();
}

void Renderer::addModel(
      const std::string& meshFile,
      const std::string& textureFile
) {
   std::unique_ptr<Model> newModel = std::make_unique<Model>(
         (std::string(MODEL_DIR) + meshFile).c_str(),
         textureFile
   );

   m_models.push_back(std::move(newModel));
}

void Renderer::createSyncObjects()
{
   m_imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
   m_renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
   m_inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

   //---------------------------Sync. Objects Info-----------------------------

   VkSemaphoreCreateInfo semaphoreInfo{};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fenceInfo{};
   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   // Creates the fence in the signaled state, so that the first call to
   // vkWaitForFences() returns immediately since the fence is already signaled.
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   //---------------------Creation of Sync. Objects----------------------------

   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
   {
      auto status = vkCreateSemaphore(
            m_device.getLogicalDevice(),
            &semaphoreInfo,
            nullptr,
            &m_imageAvailableSemaphores[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create semaphore!");

      status = vkCreateSemaphore(
         m_device.getLogicalDevice(),
         &semaphoreInfo,
         nullptr,
         &m_renderFinishedSemaphores[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create semaphore!");

      status = vkCreateFence(
            m_device.getLogicalDevice(),
            &fenceInfo,
            nullptr,
            &m_inFlightFences[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create fence!");
   }
}

void Renderer::createVkInstance()
{
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED &&
       !vlManager::areAllRequestedLayersAvailable()
   ) {
      throw std::runtime_error(
            "Validation layers requested, but not available!"
      );
   }

   // This data is optional, but it may provide some useful information
   // to the driver in order to optimize our specific application(e.g because
   // it uses a well-known graphics engine with certain special behavior).
   //
   // Example: Our game uses UE and nvidia launched a new driver that optimizes
   // a certain thing. So in that case, nvidia will know it can apply that
   // optimization verifying this info.
   VkApplicationInfo appInfo{};

   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = config::TITLE;
   appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.pEngineName = "No Engine";
   appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.apiVersion = VK_API_VERSION_1_0;

   // This data is not optional and tells the Vulkan driver which global
   // extensions and validation layers we want to use.
   VkInstanceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   std::vector<const char*> extensions = (
         extensionsUtils::getRequiredExtensions()
   );

   createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   // This variable is placed outside the if statement to ensure that it is
   // not destroyed before the vkCreateInstance call.
   VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {

      createInfo.enabledLayerCount = static_cast<uint32_t> (
            vLayersConfig::VALIDATION_LAYERS.size()
      );
      createInfo.ppEnabledLayerNames = vLayersConfig::VALIDATION_LAYERS.data();

      debugCreateInfo = vlManager::getDebugMessengerCreateInfo();

      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

   } else
   {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
   }

   // -------------------------------------------------------------
   // Paramet. 1 -> Pointer to struct with creation info.
   // Paramet. 2 -> Pointer to custom allocator callbacks.
   // Paramet. 3 -> Pointer to the variable that stores the handle to the
   //               new object.
   // -------------------------------------------------------------
   if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
      throw std::runtime_error("Failed to create Vulkan's instance!");
}

void Renderer::initVK()
{
   createVkInstance();

   vlManager::setupDebugMessenger(m_vkInstance, m_debugMessenger);
   m_windowM.createSurface(m_vkInstance);

   m_device.pickPhysicalDevice(
         m_vkInstance,
         m_qfIndices,
         m_windowM.getSurface(),
         m_swapchain
   );

   m_device.createLogicalDevice(m_qfIndices);
   
   m_qfHandles.setQueueHandles(m_device.getLogicalDevice(), m_qfIndices);

   m_swapchain.createSwapchain(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_windowM
   );

   m_swapchain.createAllImageViews(m_device.getLogicalDevice());

   m_renderPass.createRenderPass(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_swapchain.getImageFormat()
   );

   descriptorSetLayoutUtils::createDescriptorSetLayout(
         m_device.getLogicalDevice(),
         // Descriptor Types / Descriptor Bindings / Descriptor Stages
         {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
         },
         m_descriptorSetLayout
   );

   m_graphicsPipelineM.createGraphicsPipeline(
         m_device.getLogicalDevice(),
         m_swapchain.getExtent(),
         m_renderPass.getRenderPass(),
         m_descriptorSetLayout
   );

   m_depthBuffer.createDepthBuffer(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_swapchain.getExtent()
   );

   m_swapchain.createFramebuffers(
         m_device.getLogicalDevice(),
         m_renderPass.getRenderPass(),
         m_depthBuffer
   );

   // Command Pool #1
   // (improve this)
   const uint32_t cmdPoolIndex = 0;
   CommandPool newCommandPool;
   newCommandPool.createCommandPool(
         m_device.getLogicalDevice(),
         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         m_qfIndices
   );
   m_commandPools.push_back(newCommandPool);

   for (auto& model : m_models)
   {
      // Vertex buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            m_commandPools[cmdPoolIndex],
            m_device.getPhysicalDevice(),
            m_device.getLogicalDevice(),
            model->vertices,
            m_qfHandles.graphicsQueue,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            model->vertexMemory,
            model->vertexBuffer
      );

      // Index Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            m_commandPools[cmdPoolIndex],
            m_device.getPhysicalDevice(),
            m_device.getLogicalDevice(),
            model->indices,
            m_qfHandles.graphicsQueue,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            model->indexMemory,
            model->indexBuffer
      );
      
      // Create Textures
      model->createTexture(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         VK_FORMAT_R8G8B8A8_SRGB,
         m_commandPools[0],
         m_qfHandles.graphicsQueue
      );

      // Uniform Buffers
      model->ubo.createUniformBuffers(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         config::MAX_FRAMES_IN_FLIGHT
      );
   }
   

   // Descriptor Pool
   // (Calculates the total size of the pool depending of the descriptors
   // we send as parameter and the number of descriptor SETS defined)
   m_descriptorPool.createDescriptorPool(
         m_device.getLogicalDevice(),
         // Type of descriptors / Count of each type of descriptor in the pool.
         {
            {
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               static_cast<uint32_t> (
                     m_models.size() * config::MAX_FRAMES_IN_FLIGHT
               )
            },
            { 
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               static_cast<uint32_t> (
                     m_models.size() * config::MAX_FRAMES_IN_FLIGHT
               )
            }
         },
         // Descriptor SETS count.
         m_models.size() * config::MAX_FRAMES_IN_FLIGHT
   );
  
   // Descriptor Sets(of each type of model)
   for (auto& model : m_models)
   {
      model->descriptorSets.createDescriptorSets(
            m_device.getLogicalDevice(),
            model->texture.getTextureImageView(),
            model->texture.getTextureSampler(),
            model->ubo.getUniformBuffers(),
            m_descriptorSetLayout,
            m_descriptorPool
      );
   }
   
   // Allocates all the command buffers in the command Pool #1
   m_commandPools[cmdPoolIndex].allocAllCommandBuffers();

   createSyncObjects();

}

void Renderer::recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const VkRenderPass& renderPass,
      const VkExtent2D& extent,
      const VkPipeline& graphicsPipeline,
      const VkPipelineLayout& pipelineLayout,
      const uint32_t currentFrame,
      VkCommandBuffer& commandBuffer,
      CommandPool& commandPool
) {

   // Specifies some details about the usage of this specific command
   // buffer.
   commandPool.beginCommandBuffer(0, commandBuffer);
   
      VkRenderPassBeginInfo renderPassInfo{};
      commandPool.createRenderPassBeginInfo(
            renderPass,
            framebuffer,
            extent,
            clearValues,
            renderPassInfo
      );
      
      //--------------------------------RenderPass-----------------------------

      // The final parameter controls how the drawing commands between the
      // render pass will be provided:
      //    -VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be
      //    embedded in the primary command buffer itself and no secondary
      //    command buffers will be executed.
      //    -VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass
      //    commands will be executed from secondary command buffers.
      vkCmdBeginRenderPass(
            commandBuffer,
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE
      );

      commandUtils::STATE::bindPipeline(
            graphicsPipeline,
            commandBuffer
      );
      // Set Dynamic States
      commandUtils::STATE::setViewport(
            0.0f,
            0.0f,
            extent,
            0.0f,
            1.0f,
            0,
            1,
            commandBuffer
      );
      commandUtils::STATE::setScissor(
            {0, 0},
            extent,
            0,
            1,
            commandBuffer
      );

      for (const auto& model : m_models)
      {
         commandUtils::STATE::bindVertexBuffers(
               {model->vertexBuffer},
               {0},
               0,
               1,
               commandBuffer
         );
         commandUtils::STATE::bindIndexBuffer(
               model->indexBuffer,
               0,
               VK_INDEX_TYPE_UINT32,
               commandBuffer
         );

         commandUtils::STATE::bindDescriptorSets(
               pipelineLayout,
               // Index of first descriptor set.
               0,
               {model->getDescriptorSet(currentFrame)},
               // Dynamic offsets.
               {},
               commandBuffer
         );

         commandUtils::ACTION::drawIndexed(
               model->indices.size(),
               // Instance Count
               1,
               // First index.
               0,
               // Vertex Offset.
               0,
               // First Intance.
               0,
               commandBuffer
         );
      }

      vkCmdEndRenderPass(commandBuffer);

   commandPool.endCommandBuffer(commandBuffer);
}

void Renderer::drawFrame(uint8_t& currentFrame)
{
   // Waits until the previous frame has finished.
   //    - 2 param. -> FenceCount.
   //    - 4 param. -> waitAll.
   //    - 5 param. -> timeOut.
   vkWaitForFences(
         m_device.getLogicalDevice(),
         1,
         &m_inFlightFences[currentFrame],
         VK_TRUE,
         UINT64_MAX
   );

   // After waiting, we need to manually reset the fence.
   vkResetFences(
         m_device.getLogicalDevice(),
         1,
         &m_inFlightFences[currentFrame]
   );

   //------------------------Updates uniform buffer----------------------------
   updateUniformBuffer1(
         m_device.getLogicalDevice(),
         currentFrame,
         m_swapchain.getExtent(),
         m_models[0]->ubo.getUniformBufferMemories()
   );
   //updateUniformBuffer2(
   //      m_device.getLogicalDevice(),
   //      currentFrame,
   //      m_swapchain.getExtent(),
   //      m_models[1]->ubo.getUniformBufferMemories()
   //);

   //--------------------Acquires an image from the swapchain------------------
   uint32_t imageIndex;
   vkAcquireNextImageKHR(
         m_device.getLogicalDevice(),
         m_swapchain.getSwapchain(),
         UINT64_MAX,
         // Specifies synchr. objects that have to be signaled when the
         // presentation engine is finished using the image.
         m_imageAvailableSemaphores[currentFrame],
         VK_NULL_HANDLE,
         &imageIndex
   );

   //---------------------Records all the command buffer-----------------------


   // Resets the command buffer to be able to be recorded.
   m_commandPools[0].resetCommandBuffer(currentFrame);
   recordCommandBuffer(
         m_swapchain.getFramebuffer(imageIndex),
         m_renderPass.getRenderPass(),
         m_swapchain.getExtent(),
         m_graphicsPipelineM.getGraphicsPipeline(),
         m_graphicsPipelineM.getPipelineLayout(),
         currentFrame,
         m_commandPools[0].getCommandBuffer(currentFrame),
         m_commandPools[0]
   );

   // Draws imgui
   imguiRender(currentFrame, imageIndex);

   //----------------------Submits the command buffer--------------------------

   VkSubmitInfo submitInfo{};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   VkSemaphore waitSemaphores[] = {
      m_imageAvailableSemaphores[currentFrame]
   };
   VkPipelineStageFlags waitStages [] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
   };
   submitInfo.waitSemaphoreCount = 1;
   // Specifies which semaphores to wait on before execution begins.
   submitInfo.pWaitSemaphores = waitSemaphores;
   // Specifies which stage/s of the pipeline to wait.
   submitInfo.pWaitDstStageMask = waitStages;
   // These two commands specify which command buffers to actualy submit for
   // execution.
   ///////////////////////MAKE IT CUSTOM -> IMGUI!////////////////////////////
   std::array<VkCommandBuffer, 2> submitCommandBuffers = {
      m_commandPools[0].getCommandBuffer(currentFrame),
      m_commandBuffersImgui[currentFrame]
   };
   submitInfo.commandBufferCount = 2;
   submitInfo.pCommandBuffers = submitCommandBuffers.data();
   // Specifies which semaphores to signal once the command buffer/s have
   // finished execution.
   VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = signalSemaphores;

   auto status = vkQueueSubmit(
         m_qfHandles.graphicsQueue,
         1,
         &submitInfo,
         m_inFlightFences[currentFrame]
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to submit draw command buffer!");

   //-------------------Presentation of the swapchain image--------------------
   VkPresentInfoKHR presentInfo{};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   // Specifies which semaphores to wait on before the presentation can happen.
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = signalSemaphores;

   // Specifies the swapchains to present images to and the index of the
   // image for each swapchain.
   VkSwapchainKHR swapchains[] = {m_swapchain.getSwapchain()};
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = swapchains;
   presentInfo.pImageIndices = &imageIndex;
   // Allows us to specify an array of VkResult values to check for every
   // individual swapchain if presentation was successful.
   // Optional
   presentInfo.pResults = nullptr;

   vkQueuePresentKHR(m_qfHandles.presentQueue, &presentInfo);

   // Updates the frame
   currentFrame = (currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::imguiRender(const uint8_t currentFrame, const uint8_t imageIndex)
{
   // - Resets a command pool and starts to record command into a command buffer
   // (because the UI can change...e.g buttons will be added on the fly, window
   // resizing, etc)
   {
      //vkResetCommandPool(m_device.getLogicalDevice(), m_commandPoolImgui.getCommandPool(), 0);
      vkResetCommandBuffer(m_commandBuffersImgui[currentFrame], 0);
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(m_commandBuffersImgui[currentFrame], &info);
   }

   // Starts the render pass
   {
      VkRenderPassBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass = m_renderPassImgui;
      info.framebuffer = m_framebuffersImgui[imageIndex];
      info.renderArea.extent.width = m_swapchain.getExtent().width;
      info.renderArea.extent.height = m_swapchain.getExtent().height;
      info.clearValueCount = 1;
      // no se si usar imageIndex or currentFrame
      info.pClearValues = &(clearValues[currentFrame]);
      vkCmdBeginRenderPass(
            m_commandBuffersImgui[currentFrame],
            &info,
            VK_SUBPASS_CONTENTS_INLINE
      );

         ImGui_ImplVulkan_RenderDrawData(
               ImGui::GetDrawData(),
               m_commandBuffersImgui[currentFrame]
         );
      vkCmdEndRenderPass(m_commandBuffersImgui[currentFrame]);
      vkEndCommandBuffer(m_commandBuffersImgui[currentFrame]);

   }
}

void Renderer::mainLoop()
{
   // Tells us in which frame we are,
   // between 1 <= frame <= MAX_FRAMES_IN_FLIGHT
   uint8_t currentFrame = 0;
   while (m_windowM.isWindowClosed() == false)
   {
      m_windowM.pollEvents();

      // Draws Imgui
      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      ImGui::ShowDemoWindow();
      //{
      //   static float f = 0.0f;
      //   static int counter = 0;
      //   ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
      //   bool show_demo_window;
      //   
      //   ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
      //   
      //   ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      //   ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      //   
      //   ImGui::SliderFloat("float1", &rotX, -100.0f, 360.0f);
      //   ImGui::SliderFloat("float2", &rotY, -100.0f, 360.0f);
      //   ImGui::SliderFloat("float3", &rotZ, -100.0f, 360.0f);
      //   ImGui::SliderFloat("float4", &movX, -30.0f, 10.0f);
      //   ImGui::SliderFloat("float5", &movY, -30.0f, 10.0f);
      //   ImGui::SliderFloat("float6", &movZ, -30.0f, 10.0f);

      //   ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
      //   
      //   if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
      //       counter++;
      //   ImGui::SameLine();
      //   ImGui::Text("counter = %d", counter);
      //   
      //   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      //   ImGui::End();

      //}
      ImGui::Render();
      drawFrame(currentFrame);
   }
   vkDeviceWaitIdle(m_device.getLogicalDevice());
}

void Renderer::destroySyncObjects()
{
   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
   {
      vkDestroySemaphore(
            m_device.getLogicalDevice(),
            m_imageAvailableSemaphores[i],
            nullptr
      );
      vkDestroySemaphore(
            m_device.getLogicalDevice(),
            m_renderFinishedSemaphores[i],
            nullptr
      );
      vkDestroyFence(
            m_device.getLogicalDevice(),
            m_inFlightFences[i],
            nullptr
      );
   }
}

void Renderer::cleanup()
{

   // DepthBuffer
   m_depthBuffer.destroyDepthBuffer(m_device.getLogicalDevice());

   // Framebuffers
   m_swapchain.destroyFramebuffers(m_device.getLogicalDevice());

   // ViewImages of the images from the Swapchain
   m_swapchain.destroyImageViews(m_device.getLogicalDevice());
   // Swapchain
   m_swapchain.destroySwapchain(m_device.getLogicalDevice());

   // Graphics Pipeline
   m_graphicsPipelineM.destroyGraphicsPipeline(m_device.getLogicalDevice());

   // Graphics Pipeline Layout
   m_graphicsPipelineM.destroyPipelineLayout(m_device.getLogicalDevice());

   // Render pass
   m_renderPass.destroyRenderPass(m_device.getLogicalDevice());

   // UBO(with their uniform buffers and uniform memories)
   for (auto& model : m_models)
      model->ubo.destroyUniformBuffersAndMemories(m_device.getLogicalDevice());

   // Descriptor Pool
   m_descriptorPool.destroyDescriptorPool(m_device.getLogicalDevice());

   // Textures
   for (auto& model : m_models)
      model->texture.destroyTexture(m_device.getLogicalDevice());
   
   // Descriptor Set Layout
   descriptorSetLayoutUtils::destroyDescriptorSetLayout(
         m_device.getLogicalDevice(),
         m_descriptorSetLayout
   );
   
   // Bufferss
   for (auto& model : m_models)
   {
      bufferManager::destroyBuffer(
            m_device.getLogicalDevice(),
            model->vertexBuffer
      );
      bufferManager::destroyBuffer(
            m_device.getLogicalDevice(),
            model->indexBuffer
      );
   
      // Buffer Memories
      bufferManager::freeMemory(
            m_device.getLogicalDevice(),
            model->vertexMemory
      );
      bufferManager::freeMemory(
            m_device.getLogicalDevice(),
            model->indexMemory
      );
   }
   
   // Sync objects
   destroySyncObjects();

   // Command Pool
   for (auto& commandPool : m_commandPools)
      commandPool.destroyCommandPool();

   
   // Logical Device
   vkDestroyDevice(m_device.getLogicalDevice(), nullptr);
   
   // Validation Layers
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {
         vlManager::destroyDebugUtilsMessengerEXT(
               m_vkInstance,
               m_debugMessenger,
               nullptr
         );
   }

   // Window Surface
   m_windowM.destroySurface(m_vkInstance);
   
   // Vulkan's instance
   vkDestroyInstance(m_vkInstance, nullptr);

   // GLFW
   m_windowM.destroyWindow();
}

void Renderer::updateUniformBuffer1(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent,
         std::vector<VkDeviceMemory>& uniformBufferMemories
) {
   static auto startTime = std::chrono::high_resolution_clock::now();

   auto currentTime = std::chrono::high_resolution_clock::now();
   float time = std::chrono::duration<float, std::chrono::seconds::period>(
         currentTime - startTime
   ).count();

   DescriptorTypes::UniformBufferObject ubo{};
   
   ubo.model = glm::scale(
         glm::mat4(1.0f),
         glm::vec3(0.05f)
   );
   ubo.model = glm::rotate(
         ubo.model,
         glm::radians(rotX),
         glm::vec3(1.0f, 0.0f, 0.0f)
   );
   ubo.model = glm::rotate(
         ubo.model,
         glm::radians(rotY),
         glm::vec3(0.0f, 1.0f, 0.0f)
   );
   ubo.model = glm::rotate(
         ubo.model,
         glm::radians(rotZ),
         glm::vec3(0.0f, 0.0f, 1.0f)
   );

   ubo.model = glm::translate(
         ubo.model,
         glm::vec3(movX, movY, movZ)
   );
   

   ubo.view = glm::lookAt(
         // Eye position
         glm::vec3(2.0, 2.0f, 2.0f),
         // Center position
         glm::vec3(0.0f, 0.0f, 0.0f),
         // Up Axis
         glm::vec3(0.0, 0.0f, 1.0f)
   );
   ubo.proj = glm::perspective(
         // VFOV
         glm::radians(45.0f),
         // Aspect
         (
          extent.width /
          (float)extent.height
         ),
         // Near plane
         0.1f,
         // Far plane
         10.0f
   );

   // GLM was designed for OpenGl, where the Y coordinate of the clip coord. is
   // inverted. To compensate for that, we have to flip the sign on the scaling
   // factor of the Y axis.
   ubo.proj[1][1] *= -1;

   void* data;
   vkMapMemory(
         logicalDevice,
         uniformBufferMemories[currentFrame],
         0,
         sizeof(ubo),
         0,
         &data
   );
      memcpy(data, &ubo, sizeof(ubo));
   vkUnmapMemory(
         logicalDevice,
         uniformBufferMemories[currentFrame]
   );
}

void Renderer::updateUniformBuffer2(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent,
         std::vector<VkDeviceMemory>& uniformBufferMemories
) {
   static auto startTime = std::chrono::high_resolution_clock::now();

   auto currentTime = std::chrono::high_resolution_clock::now();
   float time = std::chrono::duration<float, std::chrono::seconds::period>(
         currentTime - startTime
   ).count();

   DescriptorTypes::UniformBufferObject ubo{};
   //ubo.model = glm::rotate(
   //      glm::mat4(1.0f),
   //      glm::radians(time * 90.0f),
   //      glm::vec3(0.0f, 0.0f, 1.0f)
   //);
   ubo.model = glm::mat4(1.0f);
   ubo.model = glm::scale(
         ubo.model,
         glm::vec3(0.003f)
   );
   ubo.model = glm::translate(
         ubo.model,
         glm::vec3(-700.0f, 0.0f, 0.0f)
   );
   ubo.model = glm::rotate(
         ubo.model,
         glm::radians(time * 90.0f),
         glm::vec3(0.0f, 1.0f, 1.0f)
   );
   ubo.view = glm::lookAt(
         // Eye position
         glm::vec3(2.0, 2.0f, 2.0f),
         // Center position
         glm::vec3(0.0f, 0.0f, 0.0f),
         // Up Axis
         glm::vec3(0.0, 0.0f, 1.0f)
   );
   ubo.proj = glm::perspective(
         // VFOV
         glm::radians(45.0f),
         // Aspect
         (
          extent.width /
          (float)extent.height
         ),
         // Near plane
         0.1f,
         // Far plane
         10.0f
   );

   // GLM was designed for OpenGl, where the Y coordinate of the clip coord. is
   // inverted. To compensate for that, we have to flip the sign on the scaling
   // factor of the Y axis.
   ubo.proj[1][1] *= -1;

   void* data;
   vkMapMemory(
         logicalDevice,
         uniformBufferMemories[currentFrame],
         0,
         sizeof(ubo),
         0,
         &data
   );
      memcpy(data, &ubo, sizeof(ubo));
   vkUnmapMemory(
         logicalDevice,
         uniformBufferMemories[currentFrame]
   );
}
