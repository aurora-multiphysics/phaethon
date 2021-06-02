!# /usr/bin/env python3
"""Command line program to convert ASCOT5 wall mesh to Gmsh format
"""

from a5py.ascot5io import ascot5
import gmsh
import click


@click.command()
@click.argument('filename', type=click.Path(exists=True))
def a5wall_to_gmsh(filename):
    """Convert the ASCOT5 wall mesh in an HDF5 input file to Gmsh format and
    write out

    Parameters
    ----------

    """
    click.echo(click.format_filename(filename))


if __name__ == "__main__":
    a5wall_to_gmsh()
