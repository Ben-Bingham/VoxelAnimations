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

using namespace RenderingUtilities;

using Voxel = unsigned int;

class VoxelSpace {
public:
    const static size_t n = 8;

    Voxel GetVoxel(size_t x, size_t y, size_t z) {
        return voxels[x][y][z];
    }

    void SetVoxel(size_t x, size_t y, size_t z, Voxel voxel) {
        voxels[x][y][z] = voxel;
    }

private:
    std::array<std::array<std::array<Voxel, n>, n>, n> voxels;
};

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

    size_t animationFrameCount = 10;
    std::vector<VoxelSpace> frames{ };

    glm::vec3 center{ (float)(VoxelSpace::n / 2.0f) };

    for (size_t i = 0; i < animationFrameCount; ++i) {
        float r = (float)i;
        VoxelSpace voxels{ };

        for (size_t x = 0; x < VoxelSpace::n; ++x) {
            for (size_t y = 0; y < VoxelSpace::n; ++y) {
                for (size_t z = 0; z < VoxelSpace::n; ++z) {
                    glm::vec3 pos{ (float)x, (float)y, (float)z };

                    voxels.SetVoxel(x, y, z, 0);

                    if (glm::distance(pos, center) < r) {
                        voxels.SetVoxel(x, y, z, 1);
                    }
                }
            }
        }

        frames.push_back(voxels);
    }

    VertexAttributeObject vao{ };

    Shape cube = GetCube();

    std::vector<float> offsets{
        0.0f,
        20.0f
    };

    VertexBufferObject instanceBufferObject{ offsets };

    VertexBufferObject vbo{ cube.vertices };

    ElementBufferObject ebo{ cube.indices };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    instanceBufferObject.Bind();
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1); // The divisor of 1 means 1 of these per instance

    vao.Unbind();
    instanceBufferObject.Unbind();
    vbo.Unbind();
    ebo.Unbind();

    Transform transform{ };
    transform.position = glm::vec3{ 0.0f, 0.0f, 5.0f };

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

        if (currentAnimationFrame >= animationFrameCount) {
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

            vao.Bind();

            for (size_t x = 0; x < VoxelSpace::n; ++x) {
                for (size_t y = 0; y < VoxelSpace::n; ++y) {
                    for (size_t z = 0; z < VoxelSpace::n; ++z) {
                        if (frames[currentAnimationFrame].GetVoxel(x, y, z) <= 0) continue;

                        transform.position = glm::vec3{ (float)x, (float)y, (float)z };

                        glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)rendererTarget.GetSize().x / (float)rendererTarget.GetSize().y, camera.nearPlane, camera.farPlane);
                        transform.CalculateMatrix();
                        glm::mat4 mvp = projection * camera.View() * transform.matrix;

                        phongShader.SetMat4("mvp", mvp);
                        phongShader.SetMat4("model", transform.matrix);

                        glDrawElementsInstanced(GL_TRIANGLES, cube.Size(), GL_UNSIGNED_INT, nullptr, 2);
                    }
                }
            }

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
