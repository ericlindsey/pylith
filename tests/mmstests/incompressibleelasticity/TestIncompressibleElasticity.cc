// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
// Brad T. Aagaard, U.S. Geological Survey
// Charles A. Williams, GNS Science
// Matthew G. Knepley, University at Buffalo
//
// This code was developed as part of the Computational Infrastructure
// for Geodynamics (http://geodynamics.org).
//
// Copyright (c) 2010-2021 University of California, Davis
//
// See LICENSE.md for license information.
//
// ----------------------------------------------------------------------
//

#include <portinfo>

#include "TestIncompressibleElasticity.hh" // Implementation of class methods

#include "pylith/problems/TimeDependent.hh" // USES TimeDependent
#include "pylith/materials/IncompressibleElasticity.hh" // USES IncompressibleElasticity

#include "pylith/materials/Query.hh" // USES Query

#include "pylith/topology/Mesh.hh" // USES Mesh
#include "pylith/topology/MeshOps.hh" // USES MeshOps::nondimensionalize()
#include "pylith/topology/Field.hh" // USES Field
#include "pylith/topology/VisitorMesh.hh" // USES VecVisitorMesh
#include "pylith/topology/FieldQuery.hh" // USES FieldQuery
#include "pylith/feassemble/AuxiliaryFactory.hh" // USES AuxiliaryFactory
#include "pylith/problems/SolutionFactory.hh" // USES SolutionFactory
#include "pylith/meshio/MeshIOAscii.hh" // USES MeshIOAscii
#include "pylith/bc/DirichletUserFn.hh" // USES DirichletUserFn
#include "pylith/utils/error.hh" // USES PYLITH_METHOD_BEGIN/END
#include "pylith/utils/journals.hh" // pythia::journal

#include "spatialdata/spatialdb/UserFunctionDB.hh" // USES UserFunctionDB
#include "spatialdata/geocoords/CoordSys.hh" // USES CoordSys
#include "spatialdata/spatialdb/GravityField.hh" // USES GravityField
#include "spatialdata/units/Nondimensional.hh" // USES Nondimensional

// ---------------------------------------------------------------------------------------------------------------------
// Setup testing data.
void
pylith::mmstests::TestIncompressibleElasticity::setUp(void) {
    MMSTest::setUp();

    _material = new pylith::materials::IncompressibleElasticity;CPPUNIT_ASSERT(_material);
    _bcDisplacement = new pylith::bc::DirichletUserFn;CPPUNIT_ASSERT(_bcDisplacement);
    _bcPressure = new pylith::bc::DirichletUserFn;CPPUNIT_ASSERT(_bcPressure);
    _data = NULL;
} // setUp


// ---------------------------------------------------------------------------------------------------------------------
// Deallocate testing data.
void
pylith::mmstests::TestIncompressibleElasticity::tearDown(void) {
    delete _material;_material = NULL;
    delete _bcDisplacement;_bcDisplacement = NULL;
    delete _bcPressure;_bcPressure = NULL;
    delete _data;_data = NULL;

    MMSTest::tearDown();
} // tearDown


// ---------------------------------------------------------------------------------------------------------------------
// Initialize objects for test.
void
pylith::mmstests::TestIncompressibleElasticity::_initialize(void) {
    PYLITH_METHOD_BEGIN;

    CPPUNIT_ASSERT(_mesh);
    pylith::meshio::MeshIOAscii iohandler;
    CPPUNIT_ASSERT(_data->meshFilename);
    iohandler.filename(_data->meshFilename);
    iohandler.read(_mesh);CPPUNIT_ASSERT(_mesh);

    CPPUNIT_ASSERT_MESSAGE("Test mesh does not contain any cells.",
                           pylith::topology::MeshOps::getNumCells(*_mesh) > 0);
    CPPUNIT_ASSERT_MESSAGE("Test mesh does not contain any vertices.",
                           pylith::topology::MeshOps::getNumVertices(*_mesh) > 0);

    // Set up coordinates.
    _mesh->setCoordSys(_data->cs);
    CPPUNIT_ASSERT(_data->normalizer);
    pylith::topology::MeshOps::nondimensionalize(_mesh, *_data->normalizer);

    // Set up material
    CPPUNIT_ASSERT(_material);
    _material->setAuxiliaryFieldDB(_data->auxDB);

    for (int i = 0; i < _data->numAuxSubfields; ++i) {
        const pylith::topology::FieldBase::Discretization& info = _data->auxDiscretizations[i];
        _material->setAuxiliarySubfieldDiscretization(_data->auxSubfields[i], info.basisOrder, info.quadOrder,
                                                      _data->spaceDim, pylith::topology::FieldBase::DEFAULT_BASIS,
                                                      info.feSpace, info.isBasisContinuous);
    } // for

    // Set up problem.
    CPPUNIT_ASSERT(_problem);
    CPPUNIT_ASSERT(_data->normalizer);
    _problem->setNormalizer(*_data->normalizer);
    _problem->setGravityField(_data->gravityField);
    pylith::materials::Material* materials[1] = { _material };
    _problem->setMaterials(materials, 1);
    pylith::bc::BoundaryCondition* bcs[2] = { _bcDisplacement, _bcPressure };
    _problem->setBoundaryConditions(bcs, 2);
    _problem->setStartTime(_data->startTime);
    _problem->setEndTime(_data->endTime);
    _problem->setInitialTimeStep(_data->timeStep);

    // Set up solution field.
    CPPUNIT_ASSERT( (!_data->isExplicit && 2 == _data->numSolnSubfields) ||
                    (_data->isExplicit && 3 == _data->numSolnSubfields) );
    CPPUNIT_ASSERT(_data->solnDiscretizations);

    CPPUNIT_ASSERT(!_solution);
    _solution = new pylith::topology::Field(*_mesh);CPPUNIT_ASSERT(_solution);
    _solution->setLabel("solution");
    pylith::problems::SolutionFactory factory(*_solution, *_data->normalizer);
    int iField = 0;
    factory.addDisplacement(_data->solnDiscretizations[iField++]);
    if (_data->isExplicit) {
        factory.addVelocity(_data->solnDiscretizations[iField++]);
    } // if
    factory.addPressure(_data->solnDiscretizations[iField++]);
    _problem->setSolution(_solution);

    pylith::testing::MMSTest::_initialize();

    PYLITH_METHOD_END;
} // _initialize


// ---------------------------------------------------------------------------------------------------------------------
// Constructor
pylith::mmstests::TestIncompressibleElasticity_Data::TestIncompressibleElasticity_Data(void) :
    spaceDim(0),
    meshFilename(NULL),
    boundaryLabel(NULL),
    cs(NULL),
    gravityField(NULL),
    normalizer(new spatialdata::units::Nondimensional),

    startTime(0.0),
    endTime(0.0),
    timeStep(0.0),

    numSolnSubfields(0),
    solnDiscretizations(NULL),

    numAuxSubfields(0),
    auxSubfields(NULL),
    auxDiscretizations(NULL),
    auxDB(new spatialdata::spatialdb::UserFunctionDB),

    isExplicit(false) {
    CPPUNIT_ASSERT(normalizer);

    CPPUNIT_ASSERT(auxDB);
    auxDB->setDescription("auxiliary field spatial database");
} // constructor


// ---------------------------------------------------------------------------------------------------------------------
// Destructor
pylith::mmstests::TestIncompressibleElasticity_Data::~TestIncompressibleElasticity_Data(void) {
    delete cs;cs = NULL;
    delete gravityField;gravityField = NULL;
    delete normalizer;normalizer = NULL;
    delete auxDB;auxDB = NULL;
} // destructor


// End of file
