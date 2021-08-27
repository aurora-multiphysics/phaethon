[Mesh]
  type = FileMesh
  file = 'simple_run.inp'
  allow_renumbering = false
[]

[Problem]
  type = AscotProblem
  sync_variable = fi_heat_flux
[]

[AuxVariables]
  [fi_heat_flux]
  []
[]

[Executioner]
  type = Steady
[]
