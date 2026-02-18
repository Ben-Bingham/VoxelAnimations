#include "VoxelSpace.h"

Voxel VoxelSpace::GetVoxel(size_t x, size_t y, size_t z) {
    return voxels[CoordToIndex(x, y, z)];
}

void VoxelSpace::SetVoxel(size_t x, size_t y, size_t z, Voxel voxel) {
    voxels[CoordToIndex(x, y, z)] = voxel;
}

size_t VoxelSpace::VoxelCount() {
    size_t count = 0;
    for (size_t x = 0; x < VoxelSpace::n; ++x) {
        for (size_t y = 0; y < VoxelSpace::n; ++y) {
            for (size_t z = 0; z < VoxelSpace::n; ++z) {
                if (GetVoxel(x, y, z) == 1) ++count;
            }
        }
    }

    return count;
}

size_t VoxelSpace::CoordToIndex(size_t x, size_t y, size_t z) {
    return (z * VoxelSpace::n * VoxelSpace::n) + (y * VoxelSpace::n) + x;
}