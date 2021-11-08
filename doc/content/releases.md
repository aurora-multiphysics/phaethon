# Phaethon Release Process

1. Choose a version number. Look at previous releases and use [semantic versioning](https://semver.org/) to determine what the next number should be.
2. Update the `CHANGELOG` at the top level of the repository for this version, summarising any major changes made since the last release. Use **Fixed**, **Added**, and **Changed** sections.
3. Make sure you are on the main branch and there are no other uncommitted changes.
4. Commit the changelog and mark as a release: `git commit -m "REL: vX.Y.Z"`
5. Tag the commit: `git tag -a vX.Y.Z` and then enter brief description.
6. Push the tag and commit to remote: `git push origin main && git push origin vX.Y.Z`
7. Build the Docker image: `docker build --file Dockerfile --target phaethon --tag phaethon:vX.Y.Z .`
8. Save the Docker image to a compressed tar archive: `docker save phaethon:vX.Y.Z | gzip > phaethon-ubuntu-vX_Y_Z.tar.gz`
9. Upload tar archive to [STEP OneDrive](https://ukaeauk.sharepoint.com/:f:/s/STEP_DigitalEnablers_B_3_2/Ei608KfIoLBNlSjtyUf1XMEB-V8nUKx0iFX5p07Z7VE9xw?e=eNWutU)

TODO step 4 onwards could be automated by a script or Makefile with the version number as the input