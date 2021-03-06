#include "ClosedFormEstimator2D.h"
#include <iostream>
#include "obcore/base/System.h"
#include "obcore/math/mathbase.h"

using namespace obvious;

namespace obvious
{

ClosedFormEstimator2D::ClosedFormEstimator2D()
{
  _rms = 0.0;
  _cm[0] = 0.0;
  _cm[1] = 0.0;
  _cs[0] = 0.0;
  _cs[1] = 0.0;
  _pairs = NULL;
  _iterations = 0;
}

ClosedFormEstimator2D::~ClosedFormEstimator2D()
{

}

void ClosedFormEstimator2D::setModel(double** model, unsigned int size, double** normals)
{
  _model = model;
}

void ClosedFormEstimator2D::setScene(double** scene, unsigned int size, double** normals)
{
  _scene = scene;
}

void ClosedFormEstimator2D::setPairs(std::vector<StrCartesianIndexPair>* pairs)
{
  _pairs = pairs;

  _rms = 0.0;

  // Compute centroids
  _cm[0] = 0.0;
  _cm[1] = 0.0;
  _cs[0] = 0.0;
  _cs[1] = 0.0;

  unsigned int size = pairs->size();

  for (unsigned int i = 0; i < size; i++)
  {
    StrCartesianIndexPair pair = (*pairs)[i];
    double* pointModel = _model[pair.indexFirst];
    double* pointScene = _scene[pair.indexSecond];
    _cm[0] += pointModel[0];
    _cm[1] += pointModel[1];
    _cs[0] += pointScene[0];
    _cs[1] += pointScene[1];
    _rms += distSqr2D(pointModel, pointScene);
  }
  double dSizeInv = 1.0 / (double) size;
  _rms *= dSizeInv;
  _cm[0] *= dSizeInv;
  _cm[1] *= dSizeInv;
  _cs[0] *= dSizeInv;
  _cs[1] *= dSizeInv;
}

double ClosedFormEstimator2D::getRMS()
{
  return _rms;
}

unsigned int ClosedFormEstimator2D::getIterations(void)
{
  return _iterations;
}

void ClosedFormEstimator2D::estimateTransformation(Matrix* T)
{
  double dNominator = 0.0, dDenominator = 0.0;

  for (unsigned int i = 0; i < (*_pairs).size(); i++)
  {
    double dXFCm = _model[(*_pairs)[i].indexFirst][0]  - _cm[0];
    double dYFCm = _model[(*_pairs)[i].indexFirst][1]  - _cm[1];
    double dXSCs = _scene[(*_pairs)[i].indexSecond][0] - _cs[0];
    double dYSCs = _scene[(*_pairs)[i].indexSecond][1] - _cs[1];
    dNominator   += dYFCm * dXSCs - dXFCm * dYSCs;
    dDenominator += dXFCm * dXSCs + dYFCm * dYSCs;
  }
  _iterations++;

  // compute rotation
  double dDeltaTheta = atan2(dNominator, dDenominator);

  double dCosDeltaTheta = cos(dDeltaTheta);
  double dSinDeltaTheta = sin(dDeltaTheta);

  // compute translation
  double dDeltaX = (_cm[0] - (dCosDeltaTheta * _cs[0] - dSinDeltaTheta * _cs[1]));
  double dDeltaY = (_cm[1] - (dCosDeltaTheta * _cs[1] + dSinDeltaTheta * _cs[0]));

  // fill result matrix
  T->setIdentity();
  (*T)(0,0) = dCosDeltaTheta;
  (*T)(0,1) = -dSinDeltaTheta;
  (*T)(0,3) = dDeltaX;

  (*T)(1,0) = dSinDeltaTheta;
  (*T)(1,1) = dCosDeltaTheta;
  (*T)(1,3) = dDeltaY;

  (*T)(2,3) = 0;
}

}
