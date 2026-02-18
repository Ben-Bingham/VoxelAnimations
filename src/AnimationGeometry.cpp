#include "AnimationGeometry.h"

AnimationGeometry::AnimationGeometry(VoxelAnimation anim, Shape shape)
    : m_Animation(anim), m_Shape(shape), m_ElementCount(shape.Size()), m_Vbo(shape.vertices), m_Ebo(shape.indices) {

    for (auto& frame : anim.frames) {
        m_Vaos.push_back(new RenderingUtilities::VertexAttributeObject{ });
        m_Vaos.back()->Bind();

        m_Vbo.Bind();
        m_Ebo.Bind();
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

        m_InstanceBuffers.push_back(new InstanceBuffer{ offsets });

        m_InstanceBuffers.back()->Bind();
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 1 * sizeof(unsigned int), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1); // The divisor of 1 means 1 of these per instance

        m_Vaos.back()->Unbind();
        m_Vbo.Unbind();
        m_Ebo.Unbind();
    }

    m_Vbo.Unbind();
    m_Ebo.Unbind();
}

void AnimationGeometry::Bind(size_t frame) {
    m_Vaos[frame]->Bind();
}

size_t AnimationGeometry::ElementCount() {
    return m_ElementCount;
}

size_t AnimationGeometry::PrimitiveCount(size_t frame) {
    return m_Animation.frames[frame].voxelCount;
}

size_t AnimationGeometry::FrameCount() {
    return m_Animation.frameCount;
}