#include "SensorPolar3D.h"
#include "obcore/base/System.h"
#include "obcore/math/mathbase.h"
#include "obcore/base/Logger.h"
#include "obcore/base/tools.h"

#include <math.h>
#include <string.h>

namespace obvious
{

SensorPolar3D::SensorPolar3D(unsigned int beams, double thetaRes, double thetaMin, double phiRes) : Sensor(3)
{
  _Pose = new Matrix(4, 4);
  _Pose->setIdentity();

  _beams = beams;
  _size = beams * 1000;
  _data = new double[_size];
  _mask = new bool[_size];
  for(unsigned int i=0; i<_size; i++)
    _mask[i] = true;

  _thetaRes = thetaRes;
  _thetaMin = thetaMin;

  if(_thetaMin>=M_PI)
  {
    LOGMSG(DBG_ERROR, "Valid minimal angle < M_PI");
  }

  _phiRes = phiRes;
  _planes = M_PI/_phiRes;
  System<double>::allocate(beams, _planes, _distanceMap);
  System<int>::allocate(beams, _planes, _indexMap);
}

SensorPolar3D::~SensorPolar3D()
{
  delete _Pose;
  delete [] _data;
  delete [] _mask;
  System<double>::deallocate(_distanceMap);
  System<int>::deallocate(_indexMap);
}

unsigned int SensorPolar3D::getBeams()
{
  return _beams;
}

unsigned int SensorPolar3D::getPlanes()
{
  return _planes;
}

void SensorPolar3D::calcRayFromCurrentPose(unsigned int beam, unsigned int plane, double ray[3])
{
  Matrix Rh(4, 1);
  double theta = _thetaMin + ((double)beam) * _thetaRes;
  double x = sin(theta);
  double z = cos(theta);

  double phi = ((double)plane) / ((double)_planes) * M_PI - M_PI;

  Rh[0][0] = cos(phi) * x;
  Rh[1][0] = sin(phi) * x;
  Rh[2][0] = z;
  Rh[3][0] = 0.0;
  Rh = (*_Pose) * Rh;
  ray[0] = Rh[0][0];
  ray[1] = Rh[1][0];
  ray[2] = Rh[2][0];
}

void SensorPolar3D::setDistanceMap(vector<float> phi, vector<float> dist)
{
  LOGMSG(DBG_DEBUG, "SensorPolar3D::setDistanceMap");

  if((_beams*phi.size()) != dist.size())
  {
    LOGMSG(DBG_WARN, "SensorPolar3D::setDistanceMap: invalid size of vectors ... skipping");
    return;
  }

  double phi_corr = (M_PI / (double)phi.size() / _beams) * 270.0/360.0;
  for(unsigned int i=0; i<_planes*_beams; i++)
  {
    _distanceMap[0][i] = -1.0;
    _indexMap[0][i] = -1;
  }

  for(unsigned int c=0; c<phi.size(); c++)
  {
    unsigned int cp = phi[c] / _phiRes;
    for(unsigned int r=0; r<_beams; r++)
    {
      unsigned int cpr = cp + (unsigned int)(phi_corr/_phiRes * (double)r);
      if(cpr>_planes) continue;
      _distanceMap[r][cpr] = dist[c*_beams+r];
      _indexMap[r][cpr] = c*_beams+r;
    }
  }

  /*char filename[128];
  static int cnt = 0;
  sprintf(filename, "/tmp/map%05d.pbm", cnt++);
  unsigned char* map = new unsigned char[_planes*_beams];
  for(unsigned int r=0; r<_beams; r++)
    for(unsigned int c=0; c<_planes; c++)
      map[r*_planes+c] = (_indexMap[r][c]!=-1 ? 0 : 255);
  serializePBM(filename, map, _planes, _beams);
  delete [] map;*/
}

void SensorPolar3D::backProject(Matrix* M, int* indices)
{
  Timer t;
  Matrix PoseInv = (*_Pose);
  PoseInv.invert();
  //PoseInv.print();
  gsl_matrix* coords3D = gsl_matrix_alloc(4, M->getRows());

  gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, PoseInv.getBuffer(), M->getBuffer(), 0.0, coords3D);

  double* x = gsl_matrix_ptr(coords3D, 0, 0);
  double* y = gsl_matrix_ptr(coords3D, 1, 0);
  double* z = gsl_matrix_ptr(coords3D, 2, 0);

  for(unsigned int i=0; i<M->getRows(); i++)
  {
    double phi = atan2(*(y+i), *(x+i)) - M_PI;
    if(phi>M_PI) phi -= M_PI;
    if(phi<-M_PI) phi += M_PI;

    double r = sqrt(*(x+i) * *(x+i) + *(y+i) * *(y+i) + *(z+i) * *(z+i));
    double theta = acos(*(z+i) / r);
    if(*(y+i)>0)
      theta = -theta;

    double t = theta-_thetaMin;
    if(t>0)
    {
      unsigned int r = round(t / _thetaRes);
      if(r<_beams)
      {
        unsigned int c = (unsigned int)((M_PI+phi) / M_PI * (double)_planes);
        indices[i] = _indexMap[r][c];
      }
      else
        indices[i] = -1;
    }
    else
      indices[i] = -1;
  }
  gsl_matrix_free(coords3D);
}

}