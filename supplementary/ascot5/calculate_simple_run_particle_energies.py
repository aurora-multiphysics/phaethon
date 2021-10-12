#!/usr/bin/env python
# coding: utf-8

# The particle energy values returned by the ASCOT5 Python interface are not accurate for some reason. There is some internal conversion going on that results in results that are off in about the 4th significant digit and causing unit tests to fail. The code below is to generate the energy values independently from the mass and velocity values. Empirically, these have matched the results from the MOOSE App (`AscotProblem`). 

# In[ ]:


from a5py.ascot5io import ascot5
import numpy as np
from scipy import constants
import pprint


# In[ ]:


a5file = ascot5.Ascot('simple_run.h5')


# In[ ]:


endstate = a5file.active.endstate
velocities = np.array([endstate[index] for index in ('vr', 'vphi', 'vz')])
mass = endstate['mass']


# In[ ]:


velocities = velocities.T


# In[ ]:


def calculate_rel_energy(mass, velocities):
    c_sqrd = constants.c**2
    gamma = 1 / np.sqrt(1 - np.square(velocities).sum() / c_sqrd)
    return (gamma - 1) * mass * c_sqrd


# In[ ]:


energies = [calculate_rel_energy(mass[i], velocities[i]) for i in range(mass.size)]
pprint.pprint(energies)


# And save to file for use in tests.

# In[ ]:


np.savetxt('simple_run_energies.csv', energies, newline=', ')

