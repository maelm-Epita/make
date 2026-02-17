## Summary
A subset of GNU make written in C over one week

## Build instructions
run ``make`` in repository root

## Usage
| Command           | Usage             |
|-------------------|-------------------|
| ``./minimake -h`` | Print help string |
| ``./minimake -f {file}`` | Run minimake on 'file' (by default, searches for 'Makefile' or 'makefile' in current directory) |
| ``./minimake -p`` | Only print parsed makefile, don't run any target |
| ``./minimake {target}`` | Run 'target' (by default, runs the first target) |

## Technical stack
- C
- Bash (functional testing)
- Criterion (unit testing)
