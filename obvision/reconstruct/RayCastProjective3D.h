#ifndef RAYCASTPROJECTIVE3D_H
#define RAYCASTPROJECTIVE3D_H

#include <vector>
#include "obcore/math/Matrix.h"
#include "TsdSpace.h"
#include "SensorProjective3D.h"
#include "RayCast3D.h"

namespace obvious
{

/**
 * @class RayCastProjective3D
 * @brief Implementation of projective ray casting method
 * @author Philipp Koch, Stefan May
 */
class RayCastProjective3D : public RayCast3D
{
public:

  /**
   *
   */
  RayCastProjective3D(const unsigned int cols, const unsigned int rows, SensorProjective3D* sensor, TsdSpace* space);

  /**
   *
   */
  ~RayCastProjective3D();

  /**
   *
   */
  void calcCoordsFromCurrentView(double* coords, double* normals, unsigned char* rgb, unsigned int* ctr, unsigned int subsampling=1);

  void calcCoordsFromCurrentViewMask(double* coords, double* normals, unsigned char* rgb, bool* mask);

private:

  unsigned int _cols;

  unsigned int _rows;

  SensorProjective3D* _sensor;
};

}

#endif //RAYCASTPROJECTIVE3D