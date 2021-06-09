[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
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
