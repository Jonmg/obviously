#ifndef __MAT_D__
#define __MAT_D__

#include <ostream>

#include "AbstractMat.h"

namespace xmlpp {
class Node;
}

namespace obvious {

class MatD : public AbstractMat<double>
{
public:
    //! default constructor
    MatD(const unsigned int cols = 0, const unsigned int rows = 0);

    //! constructor it will be init by an xml node
    /*
      read the xml node and construct a matrix from the included data.
    */
    MatD(const xmlpp::Node* node);

    //! copy constructor
    /*!
      makes a deep copy of mat
    */
    MatD(const MatD& mat);

    //! destructor
    virtual ~MatD(void);

    void createXml(xmlpp::Node* node) const;

    double& at(const unsigned int col, const unsigned int row);
    double at(const unsigned int col, const unsigned int row) const;

    //! assignment operator
    /*!
      he dosen't make a deep copy. Both Mats works on the same data. For a explicit deep copy use the copy constructor or the funciton copyTo()
    */
    MatD& operator=(MatD& mat);

    //! compute a marix matrix product
    MatD operator*(const MatD& mat);
};

}

std::ostream& operator<<(std::ostream& os, const obvious::MatD& mat);

#endif