#!/usr/bin/env nemesis
#
# ======================================================================
#
# Brad T. Aagaard, U.S. Geological Survey
# Charles A. Williams, GNS Science
# Matthew G. Knepley, University at Buffalo
#
# This code was developed as part of the Computational Infrastructure
# for Geodynamics (http://geodynamics.org).
#
# Copyright (c) 2010-2022 University of California, Davis
#
# See LICENSE.md for license information.
#
# ======================================================================
#
# @file tests/pytests/problems/TestPhysics.py
#
# @brief Unit testing of Python Physics object.

import unittest

from pylith.testing.UnitTestApp import TestAbstractComponent
from pylith.problems.Physics import Physics


class TestPhysics(TestAbstractComponent):
    """Unit testing of Physics object.
    """
    _class = Physics


if __name__ == "__main__":
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(TestPhysics))
    unittest.TextTestRunner(verbosity=2).run(suite)


# End of file
