# TestScript
An incomplete VM and compiler created for learning purpose.

## Install
Clone this repository
```console
git clone --recursive https://github.com/Kevar/TestScript.git
```

Download premake5, place it in the repository next to the premake5.lua script and use it to create a solution:
```console
premake5.exe vs2022
```

Open the solution, compile it and launch the TestApp project (should be already set as default project and command line arguments should be properly defined)

## Command line arguments
-h: display an help
-tok: display tokens from tokenization phase
-ast: display Abstract Syntax Tree from parser phase
-sem: display semantic data from analyzis phase
-ops: display bytecode operation emitted from compilation phase
-stacks: display the status of memory and execution stacks at the end of execution
-noexe: doesn't execute the script, allowing to check data from various steps (previous options)
