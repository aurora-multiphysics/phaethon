#!/usr/bin/env python
# coding: utf-8

# In[ ]:


import a5py.ascot5io.ascot5 as ascot5
import numpy as np
from a5py.wallloads.calculate import wallLoad3DEndstate

hdf5 = ascot5.Ascot('simple_run.h5')
wall = hdf5.active.wall
endstate = hdf5.active.endstate
walltile = endstate.get('walltile')

A = wall.area()  # Triangle area
nWallTris = wall.getNumberOfElements()

# Pick the wall hit particles
wall_hits = walltile.nonzero()

mesh_elements = walltile[wall_hits] - 1
weights = endstate.get('weight')[wall_hits]
energies = endstate.get('energy')[wall_hits]

heat_fluxes = np.zeros(nWallTris)
powers = np.multiply(weights, energies)
for element, power in zip(mesh_elements, powers):
    heat_fluxes[element] += power

heat_fluxes = np.divide(heat_fluxes, A)

print(heat_fluxes)


# Compare this to the existing script in ascot5-python interface:

# In[ ]:


wall_load = wallLoad3DEndstate(hdf5)
print(wall_load)
print(np.equal(wall_load, heat_fluxes).all())


# These don't match because there is an indexing error in the existing script
# that needs to be fixed. My answer above is correct.
#
# Finally, save to a file for future use in unit tests

# In[ ]:


np.savetxt('simple_run_heat_fluxes.csv', heat_fluxes, newline=', ')


# And we should also save the walltile hit values and particle weights while we
# are here to complete the unit test data that is needed.

# In[ ]:


np.savetxt('weight.csv', endstate.get('weight'), newline=', ')
np.savetxt('walltile.csv', walltile, newline=', ', fmt='%1d')
