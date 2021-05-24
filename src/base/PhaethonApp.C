#include "PhaethonApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
PhaethonApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

PhaethonApp::PhaethonApp(InputParameters parameters) : MooseApp(parameters)
{
  PhaethonApp::registerAll(_factory, _action_factory, _syntax);
}

PhaethonApp::~PhaethonApp() {}

void
PhaethonApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"PhaethonApp"});
  Registry::registerActionsTo(af, {"PhaethonApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
PhaethonApp::registerApps()
{
  registerApp(PhaethonApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
PhaethonApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PhaethonApp::registerAll(f, af, s);
}
extern "C" void
PhaethonApp__registerApps()
{
  PhaethonApp::registerApps();
}
