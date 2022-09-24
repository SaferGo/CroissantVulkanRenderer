#include <VulkanToyRenderer/GUI/GUI.h>

#include <cstring>

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
#include <VulkanToyRenderer/Model/Model.h>

GUI::GUI(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkInstance& vkInstance,
      const Swapchain& swapchain,
      const uint32_t& graphicsFamilyIndex,
      const VkQueue& graphicsQueue,
      Window& window
) : m_opSwapchain(&swapchain), m_opLogicalDevice(&logicalDevice) {

   // - Descriptor Pool
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

   // - RenderPass
   createRenderPass();
   
   // - Imgui init
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   (void)io;

   //ImGui::StyleColorsDark();
   applyStyle();

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
   m_commandPool = CommandPool(
         logicalDevice,
         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         graphicsFamilyIndex
   );
   m_commandPool.allocCommandBuffers(config::MAX_FRAMES_IN_FLIGHT);

   uploadFonts(graphicsQueue);
   
   createFrameBuffers();
}

void GUI::applyStyle()
{
   auto& style = ImGui::GetStyle();
   style.FrameRounding = 4.0f;
   style.WindowBorderSize = 0.0f;
   style.PopupBorderSize = 0.0f;
   style.GrabRounding = 4.0f;
   
   ImVec4* colors = ImGui::GetStyle().Colors;
   colors[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
   colors[ImGuiCol_TextDisabled]         = ImVec4(0.73f, 0.75f, 0.74f, 1.00f);
   colors[ImGuiCol_WindowBg]             = ImVec4(0.09f, 0.09f, 0.09f, 0.94f);
   colors[ImGuiCol_ChildBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
   colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
   colors[ImGuiCol_Border]               = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
   colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
   colors[ImGuiCol_FrameBg]              = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
   colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.84f, 0.66f, 0.66f, 0.40f);
   colors[ImGuiCol_FrameBgActive]        = ImVec4(0.84f, 0.66f, 0.66f, 0.67f);
   colors[ImGuiCol_TitleBg]              = ImVec4(0.47f, 0.22f, 0.22f, 0.67f);
   colors[ImGuiCol_TitleBgActive]        = ImVec4(0.47f, 0.22f, 0.22f, 1.00f);
   colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.47f, 0.22f, 0.22f, 0.67f);
   colors[ImGuiCol_MenuBarBg]            = ImVec4(0.34f, 0.16f, 0.16f, 1.00f);
   colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
   colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
   colors[ImGuiCol_CheckMark]            = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
   colors[ImGuiCol_SliderGrab]           = ImVec4(0.71f, 0.39f, 0.39f, 1.00f);
   colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.84f, 0.66f, 0.66f, 1.00f);
   colors[ImGuiCol_Button]               = ImVec4(0.47f, 0.22f, 0.22f, 0.65f);
   colors[ImGuiCol_ButtonHovered]        = ImVec4(0.71f, 0.39f, 0.39f, 0.65f);
   colors[ImGuiCol_ButtonActive]         = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
   colors[ImGuiCol_Header]               = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
   colors[ImGuiCol_HeaderHovered]        = ImVec4(0.84f, 0.66f, 0.66f, 0.65f);
   colors[ImGuiCol_HeaderActive]         = ImVec4(0.84f, 0.66f, 0.66f, 0.00f);
   colors[ImGuiCol_Separator]            = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
   colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
   colors[ImGuiCol_SeparatorActive]      = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
   colors[ImGuiCol_ResizeGrip]           = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
   colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
   colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
   colors[ImGuiCol_Tab]                  = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
   colors[ImGuiCol_TabHovered]           = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
   colors[ImGuiCol_TabActive]            = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
   colors[ImGuiCol_TabUnfocused]         = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
   colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
   colors[ImGuiCol_PlotLines]            = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
   colors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
   colors[ImGuiCol_PlotHistogram]        = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
   colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
   colors[ImGuiCol_DragDropTarget]       = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
   colors[ImGuiCol_NavHighlight]         = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
   colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
   colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
   colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

/*
 * -Uploads the fonts to the GPU.
 */
void GUI::uploadFonts(const VkQueue& graphicsQueue)
{
   // (one time command buffer)
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

void GUI::createFrameBuffers()
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
            *m_opLogicalDevice,
            &info,
            nullptr,
            &m_framebuffers[i]
      );
   }
}

void GUI::createRenderPass()
{
   // - Attachments
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

   VkAttachmentReference colorAttachment = {};
   attachmentUtils::createAttachmentReference(
         0,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorAttachment
   );

   // This vector is neccessary because if not, it will crash.
   // (some mysterious bug....)
   std::vector<VkAttachmentReference> allAttachments = {
      colorAttachment
   };

   // - Subpass
   VkSubpassDescription subpass = {};
   subPassUtils::createSubPassDescription(
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         allAttachments,
         nullptr,
         subpass
   );

   // -Synch. between this render pass and the one from the renderer.
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

   m_renderPass = RenderPass(
         *m_opLogicalDevice,
         {attachment},
         {subpass},
         {dependency}
   );
}

void GUI::recordCommandBuffer(
      const uint8_t currentFrame,
      const uint8_t imageIndex,
      const std::vector<VkClearValue>& clearValues
) {
   const VkCommandBuffer& commandBuffer = (
         m_commandPool.getCommandBuffer(currentFrame)
   );

   m_commandPool.resetCommandBuffer(currentFrame);
   m_commandPool.beginCommandBuffer(
         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         currentFrame
   );
   
      m_renderPass.begin(
            m_framebuffers[imageIndex],
            m_opSwapchain->getExtent(),
            {clearValues[currentFrame]},
            commandBuffer,
            VK_SUBPASS_CONTENTS_INLINE
      );

         ImGui_ImplVulkan_RenderDrawData(
               ImGui::GetDrawData(),
               commandBuffer
         );

      m_renderPass.end(commandBuffer);
   m_commandPool.endCommandBuffer(commandBuffer);
}

const VkCommandBuffer& GUI::getCommandBuffer(const uint32_t index) const
{
   return m_commandPool.getCommandBuffer(index);
}

void GUI::draw(
      std::vector<std::shared_ptr<Model>> models,
      glm::fvec3& cameraPos
) {

   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

      modelsWindow(models);
      cameraWindow(cameraPos);

   ImGui::Render();
}

void GUI::modelsWindow(std::vector<std::shared_ptr<Model>> models)
{
   ImGui::Begin("Models");

      for (auto& model : models)
      {
         std::string modelName = model.get()->getName();
         if (ImGui::TreeNode(modelName.c_str()))
         {
            if (ImGui::TreeNode(
                     ("Position##TRANSLATION::" + modelName).c_str()
            ) ){
               ImGui::SliderFloat(
                     ("X##TRANSLATION::" + modelName).c_str(),
                     &(model.get()->actualPos.x),
                     -10.0f,
                     10.0f
               );
               ImGui::SliderFloat(
                     ("Y##TRANSLATION::" + modelName).c_str(),
                     &(model.get()->actualPos.y),
                     -10.0f,
                     10.0f
               );
               ImGui::SliderFloat(
                     ("Z##TRANSLATION::" + modelName).c_str(),
                     &(model.get()->actualPos.z),
                     -10.0f,
                     10.0f
               );
               ImGui::TreePop();
               ImGui::Separator();
            }
            if (ImGui::TreeNode(
                     ("Size##SIZE::" + modelName).c_str()
            ) ) {
               ImGui::SliderFloat(
                     ("X##SCALE::" + modelName).c_str(),
                     &(model.get()->actualSize.x),
                     0.0f,
                     1.0f
               );
               ImGui::SliderFloat(
                     ("Y##SCALE" + modelName).c_str(),
                     &(model.get()->actualSize.y),
                     0.0f,
                     1.0f
               );
               ImGui::SliderFloat(
                     ("Z##SCALE" + modelName).c_str(),
                     &(model.get()->actualSize.z),
                     0.0f,
                     1.0f
               );
               ImGui::TreePop();
               ImGui::Separator();
            }
            if (ImGui::TreeNode(
                     ("Rotation##ROTATION::" + modelName).c_str()
            ) ) {
               ImGui::SliderFloat(
                     ("X##ROTATION::" + modelName).c_str(),
                     &(model.get()->actualRot.x),
                     -5.0f,
                     5.0f
               );
               ImGui::SliderFloat(
                     ("Y##ROTATION" + modelName).c_str(),
                     &(model.get()->actualRot.y),
                     -5.0f,
                     5.0f
               );
               ImGui::SliderFloat(
                     ("Z##ROTATION" + modelName).c_str(),
                     &(model.get()->actualRot.z),
                     -5.0f,
                     5.0f
               );
               ImGui::TreePop();
               ImGui::Separator();
            }

            ImGui::TreePop();
            ImGui::Separator();
         }
      }

      ImGui::End();
}

void GUI::cameraWindow(glm::fvec3& cameraPos)
{
   ImGui::Begin("Camera");
      ImGui::SliderFloat(
            "X##POSITION::CAMERA",
            &cameraPos.x,
            -5.0f,
            5.0f
      );
      ImGui::SliderFloat(
            "Y##POSITION::CAMERA",
            &cameraPos.y,
            -5.0f,
            5.0f
      );
      ImGui::SliderFloat(
            "Z##POSITION::CAMERA",
            &cameraPos.z,
            -5.0f,
            5.0f
      );

   ImGui::End();
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
