# Traffic Violation Record Management System

A modular command-line application written in C that lets a traffic office record, search, update and report on traffic violations and their fines.

- **Engineering domain:** Transportation
- **Language:** C11 (application), C++17 (Google Test files only)
- **Repository:** https://github.com/MeetG-071/Traffic-Violation-Record-Management-System

## Problem statement

In city traffic offices, violation records are often kept on paper or in unstructured spreadsheets. Officers find it slow to look up a vehicle's history, fines are applied inconsistently (especially for repeat offenders), unpaid fines are noticed late, and duplicate or invalid entries go unnoticed. This project provides a validated, file-backed record system with automatic fine calculation, overdue detection and summary reports.

## Proposed solution

The application stores violation records in a text file and offers a console menu to manage them. It validates every input, calculates the fine from the violation type (and speed excess for speeding), applies a penalty for repeat offenders, tracks payment, flags fines that are unpaid for more than 30 days, and produces summaries and sorted listings.

## Features

- Add violation records with full input validation and duplicate detection
- Automatic fine calculation (speed-based fine for speeding, fixed fines otherwise, 2x fine from the 4th violation of a vehicle)
- List records, search by record ID, view the full history of a vehicle
- Mark fines as paid, delete records (with confirmation)
- Overdue-fine detection (unpaid for more than 30 days)
- Records sorted by fine amount (highest first)
- Summary report (totals, collected / outstanding fines, count per violation type) with export to a text file
- Persistent storage in a human-readable text file, automatically saved after every change
- Safe handling of invalid input, missing files and corrupt data files

## Functional requirements summary

| ID | Requirement |
|----|-------------|
| FR-01 | Add a violation record |
| FR-02 | Validate vehicle number, driver name, location, date, violation type and speeds |
| FR-03 | Reject duplicate records and report when storage is full |
| FR-04 | Calculate the fine automatically (including the repeat-offender rule) |
| FR-05 | List all records |
| FR-06 | Search a record by ID |
| FR-07 | Show the violation history of a vehicle |
| FR-08 | Mark a fine as paid |
| FR-09 | Delete a record |
| FR-10 | Identify overdue unpaid fines |
| FR-11 | List records sorted by fine |
| FR-12 | Generate a summary report and export it to a file |
| FR-13 | Load records at start-up and save after changes |

The complete requirement list with verification methods is in [`docs/project_proposal.md`](docs/project_proposal.md).

## Software architecture

![Software architecture](docs/architecture.png)

`main.c` only controls the high-level flow (load data, show menu, dispatch, save). The `ui` module handles console input and menu actions; `violation`, `validation` and `date_utils` hold the business rules; `report` produces output; `file_handler` owns persistence. Every function returns a `Status_t` code from `status.h`. The detailed design is in [`docs/detailed_design.md`](docs/detailed_design.md).

## Module descriptions

| Module | Files | Responsibility |
|--------|-------|----------------|
| Main application | `src/main.c` | Load data, run the menu loop, save after changes |
| User interface | `src/ui.c`, `include/ui.h` | Safe line input, menu display, one handler per menu action |
| Violation logic | `src/violation.c`, `include/violation.h` | Data structures, add / find / delete / pay, fine calculation, overdue status |
| Validation | `src/validation.c`, `include/validation.h` | Field validation and strict number parsing |
| Date utilities | `src/date_utils.c`, `include/date_utils.h` | Leap years, date parsing, day differences, today's date |
| File handler | `src/file_handler.c`, `include/file_handler.h` | Load and save the text database with error detection |
| Report | `src/report.c`, `include/report.h` | Summary statistics, sorting, filtering, table printing, report export |
| Status codes | `src/status.c`, `include/status.h` | Shared `Status_t` enum and messages |
| Configuration | `include/config.h` | All named constants (limits, fine rules, file names) |

## Folder structure

```
Traffic-Violation-Record-Management-System/
|-- src/            main.c, ui.c, violation.c, validation.c, date_utils.c,
|                   file_handler.c, report.c, status.c
|-- include/        config.h, status.h, violation.h, validation.h, date_utils.h,
|                   file_handler.h, report.h, ui.h
|-- tests/          test_helpers.hpp, test_validation.cpp, test_date_utils.cpp,
|                   test_violation.cpp, test_file_handler.cpp, test_report.cpp
|-- data/           sample_data.txt
|-- docs/           project_proposal.md, detailed_design.md, architecture.png
|-- Makefile
|-- README.md
|-- .gitignore
`-- LICENSE
```

## C concepts used

| Concept | Where and why |
|---------|---------------|
| Structures (`ViolationRecord_t`, `RecordStore_t`, `ReportSummary_t`) | Group the data of one violation, the whole database and report totals |
| Enumerations (`ViolationType_t`, `PaymentStatus_t`, `Status_t`) | Fixed categories and consistent error codes instead of magic numbers |
| Arrays and strings | Fixed-capacity record array, bounded strings, safe copying with `snprintf` |
| Pointers and `const` | Output parameters, read-only access to records, NULL checks in every function |
| Constants (`#define` in `config.h`) | Limits, fine rules and file names are named and explained |
| File handling | `fopen`, `fgets`, `fprintf`, `fclose` with every result checked |
| Dynamic memory | `malloc` / `free` for filtered and sorted result lists whose size is only known at run time |
| `qsort` with a comparator | Sorting records by fine |
| Header guards and `extern "C"` | Modular headers that C++ test code can include |

## Coding standards followed

- 4-space indentation, no tabs, one statement per line, consistent brace placement
- Descriptive camelCase function names (`findRecordIndexById`), `_t` suffix for types, UPPER_CASE constants
- Each function has one responsibility, checks pointer arguments and returns a `Status_t`
- No unsafe input functions (`gets`, `scanf("%s")`); all input goes through `fgets`-based `readLine`
- No global variables, no magic numbers, no commented-out code
- Compiled with `-std=c11 -Wall -Wextra -Wpedantic` with zero warnings

## Prerequisites

- GCC and G++ (C11 / C++17 support), GNU Make
- Google Test (`libgtest-dev`)

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install build-essential libgtest-dev
```

On Ubuntu releases that ship only the Google Test sources, build the libraries once:

```bash
cd /usr/src/googletest && sudo cmake . && sudo make && sudo cp lib/*.a /usr/lib
```

(On Windows use WSL or MSYS2; on macOS `brew install googletest`.)

## Build instructions

```bash
git clone https://github.com/MeetG-071/Traffic-Violation-Record-Management-System.git
cd Traffic-Violation-Record-Management-System
make all
```

The executable is created at `build/tvrms`.

## Run instructions

```bash
make run
```

On the first run the sample data is copied to `data/violations.txt`, which becomes your working database (`sample_data.txt` is never modified). You can also run the program with another data file: `./build/tvrms path/to/file.txt`. Delete `data/violations.txt` to reset to the sample data.

## Test instructions

```bash
make test
```

## Other Makefile targets

| Target | Action |
|--------|--------|
| `make all` | Compile all modules and link `build/tvrms` |
| `make run` | Build and start the application |
| `make test` | Build and run the Google Test suite |
| `make clean` | Remove the `build/` directory and the exported report |

## Data file format

One record per line, fields separated by `|`. Lines starting with `#` and blank lines are ignored.

```
id|vehicle|driver|location|date|type|speed|limit|fine|payment
type:    0=Speeding 1=Red light 2=No helmet 3=No seatbelt 4=Illegal parking
payment: 0=Unpaid 1=Paid
```

## Fine rules

| Violation | Fine (Rs) |
|-----------|-----------|
| Speeding | 1000 + 100 for every km/h above the limit |
| Red light | 1000 |
| No helmet | 500 |
| No seatbelt | 500 |
| Illegal parking | 300 |
| Repeat offender (3 or more earlier violations of the same vehicle) | fine x 2 |

A fine is overdue when it is unpaid and more than 30 days have passed since the violation date.

## Sample input

Adding a violation (menu option 1):

```
Vehicle number (e.g. MH12AB1234): mh12ab1234
Driver name: Rahul Patil
Location: Karve Road
Date YYYY-MM-DD (Enter for today): 2026-11-10
Select type: 2
```

## Sample output

```
Record added. ID: 11, Fine: Rs 2000.00
```

(The base red-light fine is Rs 1000; it is doubled because this vehicle already has three earlier violations.)

Overdue fines (menu option 7):

```
Overdue fines as of 2026-11-11 (unpaid for more than 30 days):
ID    Vehicle     Date       Violation        Location                Fine (Rs)  Status
2     MH14CD5678  2026-09-12 Red light        Pimpri Chowk              1000.00  Unpaid
3     MH12AB1234  2026-09-20 Speeding         Mumbai-Pune Expressway    4000.00  Unpaid
5     MH14GH3456  2026-10-05 Illegal parking  Akurdi Station Road        300.00  Unpaid
6     MH12AB1234  2026-10-10 Red light        Hinjewadi Phase 1         1000.00  Unpaid
```

Summary report (menu option 9):

```
Total records     : 10
Unpaid records    : 7
Total fines       : Rs 15200.00
Collected fines   : Rs 4000.00
Outstanding fines : Rs 11200.00
Violations by type:
  Speeding        : 3
  Red light       : 2
  No helmet       : 2
  No seatbelt     : 1
  Illegal parking : 2
```

Invalid input is rejected with a message and no data is changed:

```
Enter choice: abc
Unsupported menu choice. Enter a number from 0 to 9.
Error: Invalid vehicle number (6-10 uppercase letters/digits)
Error: Invalid speed (speed must exceed the limit, 1-250 km/h)
```

## Unit-testing summary

Google Test, 73 tests in 5 files, all run by `make test`.

| Test file | Module tested | Highlights |
|-----------|---------------|------------|
| `test_validation.cpp` | `validation.c` | Valid / invalid / boundary values for vehicle, name, location, date, speed, fine; strict number parsing |
| `test_date_utils.cpp` | `date_utils.c` | Leap years, month lengths, day differences across month / year / leap day |
| `test_violation.cpp` | `violation.c` | Fine calculation, repeat-offender boundary, add / duplicate / storage-full, find / delete / pay, overdue boundary (30 vs 31 days) |
| `test_file_handler.cpp` | `file_handler.c` | Save / load round trip, missing file, comments, malformed lines, duplicate ids, over-long line, unwritable path |
| `test_report.cpp` | `report.c` | Summary totals, sorting and ties, vehicle history, overdue filtering, report export |

Tests are independent, use no console input and delete any temporary files they create.

## Error-handling summary

- Every function returns a `Status_t`; return values are always checked
- NULL pointers, invalid arguments and out-of-range numbers are rejected instead of crashing
- Over-long console input is discarded safely; end of input exits the program cleanly
- Duplicate records, missing records, empty database and full storage have dedicated status codes
- Missing data file: the program starts with an empty database. Corrupt data file: the program refuses to start (so it never overwrites your data) and says which problem was found
- File open, read, write and close failures are detected and reported
- Memory-allocation failure when building result lists is reported with `STATUS_MEMORY_FAILURE`

## Student information

| Field | Details |
|-------|---------|
| Student name | Meet Gupta |
| Roll number | 612507071 |
| Division / batch | EnTC Div 1 |
| Repository | Traffic-Violation-Record-Management-System |

## Known limitations

- Maximum of 500 records (`MAX_RECORDS`)
- Command-line interface only; a single user at a time
- Data is stored in a plain text file without encryption
- Fine amounts and rules are compiled in (`config.h`) and cannot be changed at run time
- Location and driver names accept only plain ASCII characters

## Future enhancements

- Editing an existing record
- Search by driver name and date range
- Configurable fine table loaded from a file
- Dynamic storage with no fixed record limit
- Binary or database storage for larger data sets

## GitHub repository link

https://github.com/MeetG-071/Traffic-Violation-Record-Management-System
