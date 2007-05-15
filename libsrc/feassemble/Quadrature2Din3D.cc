// -*- C++ -*-
//
// ======================================================================
//
//                           Brad T. Aagaard
//                        U.S. Geological Survey
//
// {LicenseText}
//
// ======================================================================
//

#include <portinfo>

#include "Quadrature2Din3D.hh" // implementation of class methods

#include <math.h> // USES fabs()

#include <assert.h> // USES assert()
#include <stdexcept> // USES internal_error

// ----------------------------------------------------------------------
// Constructor
pylith::feassemble::Quadrature2Din3D::Quadrature2Din3D(void)
{ // constructor
} // constructor

// ----------------------------------------------------------------------
// Destructor
pylith::feassemble::Quadrature2Din3D::~Quadrature2Din3D(void)
{ // destructor
} // destructor
  
// ----------------------------------------------------------------------
// Copy constructor.
pylith::feassemble::Quadrature2Din3D::Quadrature2Din3D(const Quadrature2Din3D& q) :
  Quadrature(q)
{ // copy constructor
} // copy constructor

// ----------------------------------------------------------------------
// Compute geometric quantities for a cell at quadrature points.
void
pylith::feassemble::Quadrature2Din3D::computeGeometry(
		       const ALE::Obj<Mesh>& mesh,
		       const ALE::Obj<real_section_type>& coordinates,
		       const Mesh::point_type& cell)
{ // computeGeometry
  assert(2 == _cellDim);
  assert(3 == _spaceDim);

  _resetGeometry();

  // Get coordinates of cell's vertices
  const real_section_type::value_type* vertCoords = 
    mesh->restrict(coordinates, cell);
  assert(3 == coordinates->getFiberDimension(*mesh->depthStratum(0)->begin()));

  // Loop over quadrature points
  for (int iQuadPt=0; iQuadPt < _numQuadPts; ++iQuadPt) {
    
    // Compute coordinates of quadrature point in cell
    // x = sum[i=0,n-1] (Ni * xi)
    // y = sum[i=0,n-1] (Ni * yi)
    // z = sum[i=0,n-1] (Ni * zi)
    for (int iBasis=0; iBasis < _numBasis; ++iBasis) {
      const double basis = _basis[iQuadPt*_numBasis+iBasis];
      for (int iDim=0; iDim < _spaceDim; ++iDim)
	_quadPts[iQuadPt*_spaceDim+iDim] += 
	  basis * vertCoords[iBasis*_spaceDim+iDim];
    } // for
    
    // Compute Jacobian at quadrature point
    // J = [dx/dp dx/dq]
    //     [dy/dp dy/dq]
    //     [dz/dp dz/dq]
    // dx/dp = sum[i=0,n-1] (dNi/dp * xi)
    // dx/dq = sum[i=0,n-1] (dNi/dq * xi)
    // dy/dp = sum[i=0,n-1] (dNi/dp * yi)
    // dy/dq = sum[i=0,n-1] (dNi/dq * yi)
    // dz/dp = sum[i=0,n-1] (dNi/dp * zi)
    // dz/dq = sum[i=0,n-1] (dNi/dq * zi)
    for (int iBasis=0; iBasis < _numBasis; ++iBasis)
      for (int iCol=0; iCol < _cellDim; ++iCol) {
	const double deriv = 
	  _basisDeriv[iQuadPt*_numBasis*_cellDim+iBasis*_cellDim+iCol];
	for (int iRow=0; iRow < _spaceDim; ++iRow)
	  _jacobian[iQuadPt*_cellDim*_spaceDim+iRow*_cellDim+iCol] +=
	    deriv * vertCoords[iBasis*+_spaceDim+iRow];
      } // for
    
    // Compute determinant of Jacobian at quadrature point
    // |J| = sqrt(transpose(J) J)
    const int iJ = iQuadPt*_cellDim*_spaceDim;
    const int i00 = iJ + 0*_cellDim + 0;
    const int i01 = iJ + 0*_cellDim + 1;
    const int i10 = iJ + 1*_cellDim + 0;
    const int i11 = iJ + 1*_cellDim + 1;
    const int i20 = iJ + 2*_cellDim + 0;
    const int i21 = iJ + 2*_cellDim + 1;
    // JJ = transpose(J) J 
    const double jj00 = 
      _jacobian[i00]*_jacobian[i00] +
      _jacobian[i10]*_jacobian[i10] +
      _jacobian[i20]*_jacobian[i20];
    const double jj10 =
      _jacobian[i00]*_jacobian[i01] +
      _jacobian[i10]*_jacobian[i11] +
      _jacobian[i20]*_jacobian[i21];
    const double jj01 = jj10;
    const double jj11 = 
      _jacobian[i01]*_jacobian[i01] +
      _jacobian[i11]*_jacobian[i11] +
      _jacobian[i21]*_jacobian[i21];
    const double det = sqrt(jj00*jj11 - jj01*jj10);
    _checkJacobianDet(det);
    _jacobianDet[iQuadPt] = det;
    
    // Compute inverse of Jacobian at quadrature point
    const double d10 = 
      _jacobian[i00]*_jacobian[i11] - 
      _jacobian[i01]*_jacobian[i10];
    const double d21 = 
      _jacobian[i10]*_jacobian[i21] - 
      _jacobian[i11]*_jacobian[i20];
    const double d20 = 
      _jacobian[i00]*_jacobian[i21] - 
      _jacobian[i01]*_jacobian[i20];
    if (fabs(d20) > _minJacobian) {
      // Jinv00 = 1/d10 * J11
      // Jinv10 = 1/d10 * -J10
      // Jinv02 = 1/d10 * -J01
      // Jinv11 = 1/d10 * J00
      _jacobianInv[iJ+0] =  _jacobian[i11] / d10; // Jinv00
      _jacobianInv[iJ+1] = -_jacobian[i10] / d10; // Jinv10
      _jacobianInv[iJ+2] = -_jacobian[i01] / d10; // Jinv01
      _jacobianInv[iJ+3] =  _jacobian[i00] / d10; // Jinv11
      if (fabs(d21) > _minJacobian) {
	// Jinv02 = 1/d21 -J11
	// Jinv12 = 1/d21 J10
	_jacobianInv[iJ+4] = -_jacobian[i11] / d21; // Jinv02
	_jacobianInv[iJ+5] =  _jacobian[i10] / d21; // Jinv12
	
      } else if (fabs(d20) > _minJacobian) {
	// Jinv02 = 1/d20 -J01
	// Jinv12 = 1/d20 J00
	_jacobianInv[iJ+4] = -_jacobian[i01] / d20; // Jinv02
	_jacobianInv[iJ+5] =  _jacobian[i00] / d20; // Jinv12
      } else {
	_jacobianInv[iJ+4] = 0.0; // Jinv02
	_jacobianInv[iJ+5] = 0.0; // Jinv12
      } // if/else
    } else if (fabs(d20) > _minJacobian) {
      // Jinv00 = 1/d20 * J21
      // Jinv01 = 1/d20 * -J20
      // Jinv20 = 1/d20 * -J01
      // Jinv21 = 1/d20 * J00
      _jacobianInv[iJ+0] =  _jacobian[i21] / d20; // Jinv00
      _jacobianInv[iJ+1] = -_jacobian[i20] / d20; // Jinv10
      _jacobianInv[iJ+4] = -_jacobian[i01] / d20; // Jinv02
      _jacobianInv[iJ+5] =  _jacobian[i00] / d20; // Jinv12
      if (fabs(d21) > _minJacobian) {
	// Jinv10 = 1/d21 J21
	// Jinv11 = 1/d21 -J20
	_jacobianInv[iJ+2] = -_jacobian[i21] / d21; // Jinv01
	_jacobianInv[iJ+3] =  _jacobian[i20] / d21; // Jinv11
      } else {
	_jacobianInv[iJ+2] = 0.0; // Jinv01
	_jacobianInv[iJ+3] = 0.0; // Jinv11
      } // if/else
    } else if (fabs(d21) > _minJacobian) {
      _jacobianInv[iJ+0] = 0.0; // Jinv00
      _jacobianInv[iJ+1] = 0.0; // Jinv10
      // Jinv01 = 1/d21 J21
      // Jinv11 = 1/d21 -J20
      // Jinv02 = 1/d21 -J11
      // Jinv12 = 1/d21 J10
      _jacobianInv[iJ+2] =  _jacobian[i21] / d21; // Jinv01
      _jacobianInv[iJ+3] = -_jacobian[i20] / d21; // Jin11
      _jacobianInv[iJ+4] = -_jacobian[i11] / d21; // Jinv02
      _jacobianInv[iJ+5] =  _jacobian[i10] / d21; // Jinv12
    } else
      throw std::runtime_error("Could not invert Jacobian.");
  } // for
} // computeGeometry


// End of file 
