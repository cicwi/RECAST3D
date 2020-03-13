#pragma once

#include <Eigen/Eigen>

#include "astra/Float32ProjectionData3DGPU.h"
#include "astra/ParallelProjectionGeometry3D.h"
#include "astra/ParallelVecProjectionGeometry3D.h"

namespace slicerecon {

std::unique_ptr<astra::CParallelVecProjectionGeometry3D>
parallel_beam_to_vec(astra::CParallelProjectionGeometry3D* parallel_geom);

} // namespace slicerecon
