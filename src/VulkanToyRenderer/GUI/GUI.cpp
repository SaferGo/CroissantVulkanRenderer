#include <VulkanToyRenderer/GUI/GUI.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imstb_rectpack.h>
#include <imstb_textedit.h>
#include <imstb_truetype.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/RenderPass/attachmentUtils.h>
#include <VulkanToyRenderer/RenderPass/subPassUtils.h>

GUI::GUI(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkInstance& vkInstance,
      const Swapchain& swapchain,
      const uint32_t& graphicsFamilyIndex,
      const VkQueue& graphicsQueue,
      Window& window
) : m_opSwapchain(&swapchain) {
   // -Descriptor Pool

   // (calculates the total size of the pool depending of the descriptors
   // we send as parameter and the number of descriptor SETS defined)
   m_descriptorPool.createDescriptorPool(
         logicalDevice,
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
   //{ TEST THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      VkAttachmentDescription attachment = {};
      attachmentUtils::createAttachmentDescriptionWithStencil(
            m_opSwapchain->getImageFormat(),
            VK_SAMPLE_COUNT_1_BIT,
            // Tells Vulkan to not clear the content of the framebuffer but to
            // draw over it instead.
            // (because we want the GUI to be drawn over our main rendering)
            VK_ATTACHMENT_LOAD_OP_LOAD,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            // We want optimal performance because we are going to draw some
            // stuff.
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            // Specifies that this render pass is the last one(because we want
            // to draw it over all the other render passes).
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            attachment
      );

      // -- Attachments
      VkAttachmentReference colorAttachment = {};
      attachmentUtils::createAttachmentReference(
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            colorAttachment
      );

      std::vector<VkAttachmentReference> allAttachments = {
         colorAttachment
      };

      // -- Subpass
      VkSubpassDescription subpass = {};
      subPassUtils::createSubPassDescription(
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            allAttachments,
            nullptr,
            subpass
      );
      std::vector<VkSubpassDescription> subpasses = {subpass};

      // --Synch. between this render pass and the one from the renderer.
      VkSubpassDependency dependency = {};
      subPassUtils::createSubPassDependency(
            // - Source parameters.
            // To create a dependency outside the current render pass.
            VK_SUBPASS_EXTERNAL,
            // Before drawing the GUI, we want our geometry to be already
            // renderer. That means we want the pixels to be already written to
            // the framebuffer(that's why we use 
            // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT).
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            // 0 means "nothing". Implicit sync. means Vulkan does it for us.
            0,
            // - Destination parameters.
            // Refers to our first and only subpass by its index.
            0,
            // Here we state when we want to draw(also the same thing that we
            // are waiting in srcStageMask).
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            dependency
      );
      std::vector<VkSubpassDependency> dependencies = {dependency};
  
      std::vector<VkAttachmentDescription> attachments = {attachment};
      m_renderPass = RenderPass(
            logicalDevice,
            attachments,
            subpasses,
            dependencies
      );
   //}

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   (void)io;

   ImGui::StyleColorsDark();

   ImGui_ImplGlfw_InitForVulkan(window.m_window, true);
   ImGui_ImplVulkan_InitInfo initInfo = {};
   initInfo.Instance = vkInstance;
   initInfo.PhysicalDevice = physicalDevice;
   initInfo.Device = logicalDevice;
   initInfo.QueueFamily = graphicsFamilyIndex;
   initInfo.Queue = graphicsQueue;
   initInfo.PipelineCache = VK_NULL_HANDLE;
   initInfo.DescriptorPool = m_descriptorPool.getDescriptorPool();
   initInfo.Allocator = nullptr;
   initInfo.MinImageCount = m_opSwapchain->getMinImageCount();
   initInfo.ImageCount = m_opSwapchain->getImageCount();
   initInfo.CheckVkResultFn = nullptr;
   ImGui_ImplVulkan_Init(&initInfo, m_renderPass.get());
   

   // -Creation of command buffers and command pool
   // Improve this!
   m_commandPool = CommandPool(
         logicalDevice,
         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         graphicsFamilyIndex
   );
   
   // (CHANGE THIS -> BEGIN COMMAND BUFFER TO UTILS! IT HAST NOTHING TO DO
   // WITH COMMANDPOOL.h)
   // (one time usage command buffer...)
   m_commandPool.allocCommandBuffers(config::MAX_FRAMES_IN_FLIGHT);
   {
      // AGREGAR UNA FUNCION QUE SEA PARA ALLOCATE ONE TIME CMD BUFFER
      // -Uploading the fonts to the GPU
      // (one time command buffer)
      // We can just use any commandBuffer.
      VkCommandBuffer newCommandBuffer;

      m_commandPool.allocCommandBuffer(newCommandBuffer, true);
      m_commandPool.beginCommandBuffer(
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            newCommandBuffer
      );

         ImGui_ImplVulkan_CreateFontsTexture(newCommandBuffer);

      m_commandPool.endCommandBuffer(newCommandBuffer);
      m_commandPool.submitCommandBuffer(
            graphicsQueue,
            newCommandBuffer
      );
   }

   // FrameBuffer
   {
      m_framebuffers.resize(m_opSwapchain->getImageCount());

      VkImageView attachment[1];
      VkFramebufferCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      info.renderPass = m_renderPass.get();
      info.attachmentCount = 1;
      info.pAttachments = attachment;
      info.width = m_opSwapchain->getExtent().width;
      info.height = m_opSwapchain->getExtent().height;
      // The layers is 1 because our imageViews are single images and not
      // arrays.
      info.layers = 1;
      for (uint32_t i = 0; i < m_opSwapchain->getImageCount(); i++)
      {
         attachment[0] = m_opSwapchain->getImageView(i);
         vkCreateFramebuffer(
               logicalDevice,
               &info,
               nullptr,
               &m_framebuffers[i]
         );
      }
   }
}

void GUI::recordCommandBuffer(
      const uint8_t currentFrame,
      const uint8_t imageIndex,
      const std::vector<VkClearValue>& clearValues
) {
   // - Resets a command pool and starts to record command into a command buffer
   // (because the UI can change...e.g buttons will be added on the fly, window
   // resizing, etc)
   {
      vkResetCommandBuffer(m_commandPool.getCommandBuffer(currentFrame), 0);
      VkCommandBufferBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(m_commandPool.getCommandBuffer(currentFrame), &info);
   }

   // Starts the render pass
   {
      VkRenderPassBeginInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass = m_renderPass.get();
      info.framebuffer = m_framebuffers[imageIndex];
      info.renderArea.extent.width = m_opSwapchain->getExtent().width;
      info.renderArea.extent.height = m_opSwapchain->getExtent().height;
      info.clearValueCount = 1;
      // no se si usar imageIndex or currentFrame
      info.pClearValues = &(clearValues[currentFrame]);
      vkCmdBeginRenderPass(
            m_commandPool.getCommandBuffer(currentFrame),
            &info,
            VK_SUBPASS_CONTENTS_INLINE
      );

         ImGui_ImplVulkan_RenderDrawData(
               ImGui::GetDrawData(),
               m_commandPool.getCommandBuffer(currentFrame)
         );
      vkCmdEndRenderPass(m_commandPool.getCommandBuffer(currentFrame));
      vkEndCommandBuffer(m_commandPool.getCommandBuffer(currentFrame));
   }
}

const VkCommandBuffer& GUI::getCommandBuffer(const uint32_t index) const
{
   return m_commandPool.getCommandBuffer(index);
}

void GUI::draw()
{
   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
   //ImGui::ShowDemoWindow();
   {
      static float f = 0.0f;
      static int counter = 0;
      ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
      bool show_demo_window;
      
      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
      
      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      
      //ImGui::SliderFloat("float1", &rotX, -100.0f, 360.0f);
      //ImGui::SliderFloat("float2", &rotY, -100.0f, 360.0f);
      //ImGui::SliderFloat("float3", &rotZ, -100.0f, 360.0f);
      //ImGui::SliderFloat("float4", &movX, -30.0f, 10.0f);
      //ImGui::SliderFloat("float5", &movY, -30.0f, 10.0f);
      //ImGui::SliderFloat("float6", &movZ, -30.0f, 10.0f);
   
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
      
      if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
          counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);
      
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
   
   }
   ImGui::Render();
}

void GUI::destroy(const VkDevice& logicalDevice)
{
   for (auto& framebuffer : m_framebuffers)
      vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

   m_renderPass.destroy(logicalDevice);
   m_commandPool.destroy();

   ImGui_ImplVulkan_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
   m_descriptorPool.destroyDescriptorPool(logicalDevice);

}

GUI::~GUI() {}
