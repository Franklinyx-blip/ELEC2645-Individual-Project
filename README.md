# EnviroSense – Sensor Data Converter and Analyser

ELEC2645 Unit 2 C programming project

Author: **Your Name**
Student ID: **Your ID**

---

## 1. Overview

EnviroSense is a small menu-driven C program based on the ELEC2645 Unit 2 template.

The program:

* models a few simple sensors (temperature, pressure, light);
* converts raw ADC readings into engineering units (°C, kPa, lux);
* stores converted values in memory;
* can process a whole file of ADC values;
* prints basic statistics and a simple ASCII plot;
* can save and reload measurement results;
* includes a few basic tests for the conversion function.

---

## 2. Files

* `main.c`
  Uses the provided menu framework.
  Prints the main menu, gets the user’s choice and calls `menu_item_1`–`menu_item_4`.

* `funcs.h`
  Header file with prototypes for the four menu handlers.

* `funcs.c`
  Main project logic:

  * sensor definitions and initialisation;
  * storage for converted measurements;
  * ADC → physical conversion;
  * statistics (min / max / mean, count above threshold);
  * ASCII plot in the terminal;
  * save / load of results to text files;
  * simple automated tests and a help screen.

* `Makefile`
  Used to build and clean the project and to run the simple test script.

* `test.sh`
  Script supplied with the module.
  It only checks that `main.out` exists and is executable (no functional tests).

* `LICENSE`
  Licence file supplied with the template materials.

---

## 3. Build

On Linux / macOS / lab machines:

```bash
make
```

This runs:

```bash
gcc main.c funcs.c -o main.out -lm
```

and produces `main.out`.

Clean build artefacts:

```bash
make clean
```

Run the simple build check:

```bash
make test
# or
bash test.sh
```

---

## 4. Run

After building:

```bash
./main.out
```

Main menu:

1. **Sensor setup and overview**

   * choose between Temp / Press / Light;
   * show the current sensor parameters;
   * clear stored measurements.

2. **Single ADC conversion**

   * enter one ADC value;
   * see the converted result in the correct unit;
   * value is stored for later statistics and plots.

3. **Batch conversion + statistics + plot**

   * read a text file containing one ADC value per line;
   * convert all values;
   * print min / max / mean and how many are above the threshold;
   * show an ASCII plot of the data.

4. **Save/load, tests and help**

   * save current measurements to a text file;
   * load previously saved measurements;
   * run a few conversion tests;
   * display a short help message.

5. **Exit**

   * quit the program.

---

## 5. Example input

For batch conversion (menu item 3) the program expects a plain text file with one integer ADC reading per line, for example:

```text
0
100
512
800
1023
```

The save/load functions use a simple text format with a short header and then one converted value per line.
