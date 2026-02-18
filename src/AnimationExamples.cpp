#include "AnimationExamples.h"

#include <glm/glm.hpp>

VoxelAnimation ExpandingSphereAnimation() {
    VoxelAnimation anim{  };
    anim.frameCount = (size_t)(0.9 * (float)VoxelSpace::n);

    glm::vec3 center{ (float)(VoxelSpace::n / 2.0f) };

    for (size_t i = 0; i < anim.frameCount; ++i) {
        float r = (float)i;
        VoxelSpace voxels{ };

        size_t count{ 0 };
        for (size_t x = 0; x < VoxelSpace::n; ++x) {
            for (size_t y = 0; y < VoxelSpace::n; ++y) {
                for (size_t z = 0; z < VoxelSpace::n; ++z) {
                    glm::vec3 pos{ (float)x, (float)y, (float)z };

                    voxels.SetVoxel(x, y, z, 0);

                    if (glm::distance(pos, center) < r) {
                        voxels.SetVoxel(x, y, z, 1);

                        ++count;
                    }
                }
            }
        }

        anim.frames.push_back(VoxelFrame{ voxels, count });
    }

    return anim;
}
