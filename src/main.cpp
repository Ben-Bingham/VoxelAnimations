#include <iostream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/ext/matrix_clip_space.hpp>

#include <imgui.h>

#include <utility/OpenGl/Shader.h>
#include <utility/OpenGl/VertexAttributeObject.h>
#include <utility/OpenGl/Buffer.h>
#include <utility/OpenGl/RenderTarget.h>

#include <utility/Camera.h>
#include <utility/TimeScope.h>
#include <utility/Transform.h>
#include <utility/Geometry.h>

#include "MoveCamera.h"
#include "Boilerplate.h"
#include "AnimationExamples.h"

using namespace RenderingUtilities;

int main() {
    GLFWwindow* window = InitGraphics();

    glm::ivec2 defaultFramebufferSize{ 1600, 900 };
    glm::ivec2 lastFrameViewportSize{ defaultFramebufferSize };

    RenderTarget rendererTarget{ defaultFramebufferSize };

    Shader phongShader{
        "assets\\shaders\\phong.vert",
        "assets\\shaders\\phong.frag"
    };

    Camera camera{ };

    VoxelAnimation anim = ExpandingSphereAnimation();

    std::vector<VertexAttributeObject*> frameVAOs{ }; // TODO should not be raw pointers
    using InstanceBuffer = GlBuffer<unsigned int, GL_ARRAY_BUFFER>;
    std::vector<InstanceBuffer*> instanceBuffers{};

    Shape cube = GetCube();
    VertexBufferObject vbo{ cube.vertices };
    ElementBufferObject ebo{ cube.indices };

    // TODO pull out into its own function
    for (auto& frame : anim.frames) {
        frameVAOs.push_back(new VertexAttributeObject{ });
        frameVAOs.back()->Bind();

        vbo.Bind();
        ebo.Bind();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        std::vector<unsigned int> offsets{ };

        for (size_t x = 0; x < VoxelSpace::n; ++x) {
            for (size_t y = 0; y < VoxelSpace::n; ++y) {
                for (size_t z = 0; z < VoxelSpace::n; ++z) {
                    if (frame.voxels.GetVoxel(x, y, z) > 0) {
                        unsigned int offset = (z * VoxelSpace::n * VoxelSpace::n) + (y * VoxelSpace::n) + x;
                        
                        offsets.push_back(offset);
                    }
                }
            }
        }

        instanceBuffers.push_back(new InstanceBuffer{ offsets });

        instanceBuffers.back()->Bind();
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 1 * sizeof(unsigned int), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); // The divisor of 1 means 1 of these per instance

        frameVAOs.back()->Unbind();
        vbo.Unbind();
        ebo.Unbind();
    }

    vbo.Unbind();
    ebo.Unbind();

    Transform transform{ };
    transform.position = glm::vec3{ (float)VoxelSpace::n / -2.0f }; // Center the scene at the origin
    transform.position.z -= (float)VoxelSpace::n; // Slide it backwards into view

    std::chrono::duration<double> frameTime{ };
    std::chrono::duration<double> renderTime{ };

    bool mouseOverViewPort{ false };
    glm::ivec2 viewportOffset{ 0, 0 };

    size_t currentAnimationFrame = 5;
    float animationFrameRate = 30.0f;
    float timePerAnimationFrame = 1.0f / animationFrameRate; // Time in seconds

    float lastAnimationFrameStartTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        TimeScope frameTimeScope{ &frameTime };

        glfwPollEvents();

        glm::ivec2 mousePositionWRTViewport{ mousePosition.x - viewportOffset.x, lastFrameViewportSize.y - (viewportOffset.y - mousePosition.y) };

        MoveCamera(camera, window, static_cast<float>(frameTime.count()), mousePositionWRTViewport, lastFrameViewportSize, mouseOverViewPort);

        if (lastAnimationFrameStartTime >= timePerAnimationFrame) {
            ++currentAnimationFrame;

            lastAnimationFrameStartTime = 0.0f;
        }

        if (currentAnimationFrame >= anim.frameCount) {
            currentAnimationFrame = 0;
        }

        {
            TimeScope renderingTimeScope{ &renderTime };

            rendererTarget.Bind();

            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            phongShader.Bind();
            phongShader.SetVec3("color", glm::vec3{ 1.0f, 0.0f, 0.0f });
            phongShader.SetVec3("cameraPosition", camera.position);
            phongShader.SetInt("voxelSpaceSize", VoxelSpace::n);

            frameVAOs[currentAnimationFrame]->Bind();

            glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)rendererTarget.GetSize().x / (float)rendererTarget.GetSize().y, camera.nearPlane, camera.farPlane);
            transform.CalculateMatrix();
            glm::mat4 mvp = projection * camera.View() * transform.matrix;

            phongShader.SetMat4("mvp", mvp);
            phongShader.SetMat4("model", transform.matrix);

            glDrawElementsInstanced(GL_TRIANGLES, cube.Size(), GL_UNSIGNED_INT, nullptr, anim.frames[currentAnimationFrame].voxelCount);

            rendererTarget.Unbind();
        }

        ImGuiStartFrame();

        //ImGui::ShowDemoWindow();
        //ImGui::ShowMetricsWindow();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // Keep track of this so that we can make these changes after the imgui frame is finished
        size_t changedPointLightIndex{ 0 };

        glm::ivec2 newViewportSize{ };

        { ImGui::Begin("Viewport");
            // Needs to be the first call after "Begin"
            newViewportSize = glm::ivec2{ ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };

            // Display the frame with the last frames viewport size (The same size it was rendered with)
            ImGui::Image((ImTextureID)rendererTarget.GetTexture().Get(), ImVec2{ (float)lastFrameViewportSize.x, (float)lastFrameViewportSize.y }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

            mouseOverViewPort = ImGui::IsItemHovered();

            viewportOffset = glm::ivec2{ (int)ImGui::GetCursorPos().x, (int)ImGui::GetCursorPos().y };

        } ImGui::End(); // Viewport

        ImGuiEndFrame();

        // After ImGui has rendered its frame, we resize the framebuffer if needed for next frame
        if (newViewportSize != lastFrameViewportSize) {
            rendererTarget.Resize(newViewportSize);
        }

        lastFrameViewportSize = newViewportSize;

        lastAnimationFrameStartTime += frameTime.count();

        glfwSwapBuffers(window);
    }

    DestroyGraphics();
}
