#pragma once

#include <array>

using Voxel = unsigned char;

class VoxelSpace {
public:
    const static size_t n = 32;

    Voxel GetVoxel(size_t x, size_t y, size_t z);
    void SetVoxel(size_t x, size_t y, size_t z, Voxel voxel);

    size_t VoxelCount();

private:
    size_t CoordToIndex(size_t x, size_t y, size_t z);

    std::array<Voxel, n* n* n> voxels;
};