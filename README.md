# fuzz
A template for fuzzing C projects using LLVMs LibFuzzer

## Usage

### Build Fuzz Binary

* Add required packages: `sudo apt install clang clang++ llvm make gdb`
* Check out library/app `<proj_under_test>` to fuzz and this project into the same parent folder
* Identify .c and .h files via `find <proj_under_test> -name "*.[ch]"`
* In `fuzz/Makefile`, add folders containing .c files and .c filenames w/o folder; add .h folders
* If required, add DEFINES and/or LINK_OPTS
* In `fuzz/fuzz.c`, #include header file(s) of `<proj_under_test>` which contain the function(s) to fuzz
* In `fuzz/fuzz.c`, invoke the function(s) in `LLVMFuzzerTestOneInput()`, passing the `Data` buffer directly or something else, depending what the the fuzzed function requires
* Compile running `make` in `fuzz`folder

### Execute fuzz tests

* Execute fuzz tests by `make test`
* If fuzzing stops with a crash, continue with 

### Triage/Analyze a crash

* Run `fuzz <crashfile>` to repeat the crash. 
* `fuzz -minimize_crash=1 <crashfile>` will create smaller crashfiles.
* Debug the smallest crashfile, either in gdb directly, or via vscode, opening the fuzz folder, adapting `.vscode/launch.json`, running the vscode debugger front-end

### Determine Coverage

* `make coverage` generates a coverage report showing the code lines the fuzzer executed so far
