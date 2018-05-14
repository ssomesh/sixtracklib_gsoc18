from __future__ import absolute_import, print_function
import numpy as np
import pyopencl as cl

import sixtracklib as sl

particles=sl.Particles(nparticles=2560,ndim=10)
elements=sl.Elements()

elements.add_drift(3.3)
elements.add_multipole([0,-0.01])






