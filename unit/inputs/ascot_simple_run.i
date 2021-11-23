[Mesh]
  type = FileMesh
  file = 'simple_run.inp'
  allow_renumbering = false
[]

[Problem]
  type = AscotProblem
  sync_variable = fi_heat_flux
  ascot5_file = 'simple_run_test'
[]

[AuxVariables]
  [fi_heat_flux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Executioner]
  type = Steady
[]
