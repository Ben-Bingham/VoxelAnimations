#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <utility/OpenGl/Buffer.h>
#include <utility/Geometry.h>
#include <utility/OpenGl/VertexAttributeObject.h>

#include "VoxelAnimation.h"

class AnimationGeometry {
public:
    using InstanceBuffer = RenderingUtilities::GlBuffer<unsigned int, GL_ARRAY_BUFFER>;

    AnimationGeometry(VoxelAnimation anim, Shape shape);

    void Bind(size_t frame);

    size_t ElementCount();

    size_t PrimitiveCount(size_t frame);

    size_t FrameCount();

private:
    VoxelAnimation m_Animation;

    std::vector<RenderingUtilities::VertexAttributeObject*> m_Vaos{ };
    std::vector<InstanceBuffer*> m_InstanceBuffers{ };

    Shape m_Shape;
    size_t m_ElementCount;
    RenderingUtilities::VertexBufferObject m_Vbo;
    RenderingUtilities::ElementBufferObject m_Ebo;
};