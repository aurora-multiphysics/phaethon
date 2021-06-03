# Phaethon

## Installation

`Phaethon` relies on both submodules and conda environments for installation.
Issue the following commands from the command line:

```
git clone --recurse-submodules git@github.com:aurora-multiphysics/phaethon.git
cd phaethon
conda env create --file conda_env_freeze.yml
conda activate phaethon
make -j <appropriate_#_of_processors>
```

## Building on Phaethon 

You can fork `Phaethon` to create a new MOOSE-based application.

For more information see: [http://mooseframework.org/create-an-app/](http://mooseframework.org/create-an-app/)

