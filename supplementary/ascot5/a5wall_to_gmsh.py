#! /usr/bin/env python3
"""Command line program to convert ASCOT5 wall mesh to Exodus format
"""

from a5py.ascot5io import ascot5
import meshio
import pathlib
import click


@click.command()
@click.argument('filename', type=click.Path(exists=True, path_type=pathlib.Path))
def a5wall_to_gmsh(filename):
    """Convert the ASCOT5 wall mesh in an HDF5 input file to Exodus format and
    write out.

    The output file that is written will have the same basename (stem) as the
    input file but with the ``.e`` extension, and this file will be written in
    the directory from which the script is run.

    Parameters
    ----------
    filename : str
        Input HDF5 file that holds the ASCOT5 wall mesh. It is assumed that the
        wall mesh is under the `wall/active` node of the HDF5 file.

    """
    
    # Read HDF5 ASCOT5 input file and get points and vertices
    a5file = ascot5.Ascot(fn=str(filename))
    points, vertices = a5file.wall.active.getAspointsAndVertices()

    # Define the cells from the vertices
    cells = [("triangle", vertices)]

    mesh = meshio.Mesh(points, cells)
    mesh.write(str(filename.stem) + '.e')


if __name__ == "__main__":
    a5wall_to_gmsh()
