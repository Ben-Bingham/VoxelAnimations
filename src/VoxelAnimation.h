#pragma once

#include <vector>

#include "VoxelSpace.h"

struct VoxelFrame {
    VoxelSpace voxels{ };

    size_t voxelCount{ };
};

struct VoxelAnimation {
    std::vector<VoxelFrame> frames{ };

    float frameRate{ };
    size_t frameCount{ };
};