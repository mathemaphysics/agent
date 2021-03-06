This TODO list is automatically generated from the cookiecutter-cpp-project template.
The following tasks need to be done to get a fully working project:


* Push to your remote repository for the first time by doing `git push origin main`.
* Make sure that the following software is installed on your computer:
  * A C++-17-compliant C++ compiler
  * CMake `>= 3.9`
* Enable the integration of Readthedocs with your Git hoster. In the case of Github, this means
  that you need to login at [Read the Docs](https://readthedocs.org) and click the button
  *Import a Project*.
* Make sure that doxygen is installed on your system, e.g. by doing `sudo apt install doxygen`
  on Debian or Ubuntu.
* Edit the parameters of `setup()` in `setup.py` file to contain the necessary information
  about your project, such as your email adress, PyPI classifiers and a short project description.
* Add the secret variables `TESTPYPI_API_TOKEN` and `PYPI_API_TOKEN` to your GitHub project.
  These variables can be generated by heading to `https://test.pypi.org/` and `https://pypi.org`,
  adding a new project and generating these tokens.
* Enable the integration with `codecov.io` by heading to the [Codecov.io Website](https://codecov.io),
  log in (e.g. with your Github credentials) and enable integration for your repository. This will
  allow you to have automatic coverage reports on pull requests, but is not necessary to display
  the coverage badge in the README.