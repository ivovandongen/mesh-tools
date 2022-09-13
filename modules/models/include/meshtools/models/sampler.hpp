#pragma once

namespace meshtools::models {

struct Sampler {
    int minFilter = -1;
    int magFilter = -1;
    int wrapS = -1;
    int wrapT = -1;
};

} // namespace meshtools::models