# Capstone project


## Build instructions

### Requirements
- [Cmake](https://cmake.org/) or visual studio with cmake integration for project generation.
- [Git](https://git-scm.com/) to download the repo and dependencies.s

### Steps
1) Download repo with `git clone https://github.com/Moneyl/Capstone-Fall2021.git`
2) Run `git submodule update --init --recursive` in the folder of the newly cloned repo
3) Generate a project with cmake. The commands for this depend on your system. Newer visual studio versions can open cmake projects directly. You can also generate a visual studio 2019 project with `cmake -G "Visual Studio 16 2019".
4) Open the generated project and build it. For visual studio it will output a .sln file in one of the subfolders.