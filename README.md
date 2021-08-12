# Phaethon

## Installation

The easiest way to get Phaethon running is with Docker as this will handle all
dependencies for you.  Issue the following commands from the command line:

```
git clone --recurse-submodules git@github.com:aurora-multiphysics/phaethon.git
cd phaethon
docker build --tag phaethon:dev --file Dockerfile .
docker run -it phaethon:dev /bin/bash
make -j <appropriate_#_of_processors>
```

### Building from Source

In short, take a look at the `Dockerfile` to see what dependencies are necessary.

TODO fill this out

## Building on Phaethon 

You can fork `Phaethon` to create a new MOOSE-based application.

For more information see: [http://mooseframework.org/create-an-app/](http://mooseframework.org/create-an-app/)

