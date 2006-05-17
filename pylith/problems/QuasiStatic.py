#!/usr/bin/env python
#
# ----------------------------------------------------------------------
#
#                           Brad T. Aagaard
#                        U.S. Geological Survey
#
# <LicenseText>
#
# ----------------------------------------------------------------------
#

## @file pyre/problems/QuasiStatic.py
## @brief Python QuasiStatic application driver

from Problem import Problem

# QuasiStatic class
class QuasiStatic(Problem):
  """Python QuasiStatic for quasi-static crustal dynamics simulations."""

  # INVENTORY //////////////////////////////////////////////////////////

  class Inventory(Problem.Inventory):
    """Python object for managing QuasiStatic facilities and properties."""

    ## @class Inventory
    ## Python object for managing QuasiStatic facilities and properties.
    ##
    ## \b Properties
    ## @li None
    ##
    ## \b Facilities
    ## @li \b solver Algebraic solver.
    ## @li \b disp Displacement at time t
    ## @li \b dispIncr Displacement increment at time t (t -> t+dt)

    import pyre.inventory

    from pylith.solver.SolverTSI import SolverTSI
    solver = pyre.inventory.facility("solver", factory=SolverTSI)
    solver.meta['tip'] = "Algebraic solver."

    from pylith.feassemble.Field import Field
    disp = pyre.inventory.facility("disp", factory=Field,
                                   args=["disp"])
    disp.meta['tip'] = "Displacement at time t."

    dispIncr = pyre.inventory.facility("disp_incr", factory=Field,
                                       args=["dispincr"])
  
  # PUBLIC METHODS /////////////////////////////////////////////////////

  def initialize(self):
    Problem.initialize(self)
    self.disp.initialize()
    self.dispIncr.initialize()
    return


  def step(self, dt):
    raise NotImplementedError, "QuasiStatic::step() not implemented."
    return


  def poststep(self):
    raise NotImplementedError, "QuasiStatic::poststep() not implemented."
    return


  def stableTimestep(self):
    #raise NotImplementedError, "QuasiStatic::stableTimestep() not implemented."
    return 1.0


  def checkpoint(self):
    raise NotImplementedError, "QuasiStatic::checkpoint() not implemented."
    return
  

  def __init__(self, name="quasistatic"):
    """Constructor."""
    Problem.__init__(self, name)
    return


  # PRIVATE METHODS /////////////////////////////////////////////////////

  def _configure(self):
    """Set members based using inventory."""
    Problem._configure(self)
    self.solver = self.inventory.solver
    self.disp = self.inventory.disp
    self.dispIncr = self.inventory.dispIncr
    return

  def _calcResidual(self):
    """Compute solution residual."""
    return


  def _calcJacobian(self):
    """Calculation Jacobian."""
    return


  def _convergenceTest(self):
    """Test for convergence."""
    return
  

# version
__id__ = "$Id$"

# End of file 
