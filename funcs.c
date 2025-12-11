#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "funcs.h"

#define MAX_MEASUREMENTS 1000
#define MAX_SENSORS 3

typedef struct {
    char  name[16];
    char  unit[8];
    float vref;
    int   resolution;
    float scale;
    float offset;
    float threshold;
} Sensor;

typedef struct {
    const Sensor *sensor;
    float values[MAX_MEASUREMENTS];
    int   count;
} MeasurementSet;

static Sensor         g_sensors[MAX_SENSORS];
static int            g_sensor_count = 0;
static MeasurementSet g_data;
static int            g_initialised = 0;

/* internal helpers */
static void init_system(void);
static void print_sensor_list(void);
static int  get_int(const char *prompt, int min, int max);
static float convert_adc(const Sensor *s, int adc);
static int  have_data(void);
static void calc_stats(float *min, float *max, float *mean, int *above);
static void show_plot(void);
static void batch_from_file(void);
static void save_results(void);
static void load_results(void);
static const Sensor *find_sensor(const char *name);
static void run_tests(void);
static void show_help(void);

/* -------------------- menu handlers -------------------- */

void menu_item_1(void)
{
    int done = 0;

    init_system();

    while (!done)
    {
        int choice;

        printf("\n--- Sensor setup and overview ---\n");
        if (g_data.sensor)
        {
            printf("Current sensor : %s (%s)\n",
                   g_data.sensor->name, g_data.sensor->unit);
        }
        else
        {
            printf("Current sensor : (none)\n");
        }
        printf("Stored samples : %d\n", g_data.count);
        printf("\n");
        printf("1) Choose sensor\n");
        printf("2) Show sensor details\n");
        printf("3) Clear stored measurements\n");
        printf("4) Back to main menu\n");

        choice = get_int("Select option: ", 1, 4);

        switch (choice)
        {
            case 1:
            {
                int index;
                print_sensor_list();
                index = get_int("Select sensor: ", 1, g_sensor_count);
                g_data.sensor = &g_sensors[index - 1];
                printf("Now using sensor: %s (%s)\n",
                       g_data.sensor->name, g_data.sensor->unit);
                break;
            }
            case 2:
                if (g_data.sensor)
                {
                    const Sensor *s = g_data.sensor;
                    printf("\nSensor details\n");
                    printf("  Name      : %s\n", s->name);
                    printf("  Unit      : %s\n", s->unit);
                    printf("  Vref      : %.2f V\n", s->vref);
                    printf("  Resolution: %d\n", s->resolution);
                    printf("  Scale     : %.3f\n", s->scale);
                    printf("  Offset    : %.3f\n", s->offset);
                    printf("  Threshold : %.2f %s\n",
                           s->threshold, s->unit);
                }
                else
                {
                    printf("No sensor selected.\n");
                }
                break;
            case 3:
                g_data.count = 0;
                printf("Measurements cleared.\n");
                break;
            default:
                done = 1;
                break;
        }
    }
}

void menu_item_2(void)
{
    int adc;
    float value;

    init_system();

    if (!g_data.sensor)
    {
        printf("No sensor selected. Use menu 1 first.\n");
        return;
    }

    printf("\n--- Single ADC conversion ---\n");
    printf("Sensor: %s (%s)\n",
           g_data.sensor->name, g_data.sensor->unit);

    adc = get_int("Enter ADC value: ", 0, g_data.sensor->resolution);
    value = convert_adc(g_data.sensor, adc);

    printf("ADC %d -> %.2f %s\n",
           adc, value, g_data.sensor->unit);

    if (g_data.count < MAX_MEASUREMENTS)
    {
        g_data.values[g_data.count++] = value;
    }
    else
    {
        printf("Buffer full, value not stored.\n");
    }
}

void menu_item_3(void)
{
    init_system();

    if (!g_data.sensor)
    {
        printf("No sensor selected. Use menu 1 first.\n");
        return;
    }

    printf("\n--- Batch conversion from file ---\n");
    batch_from_file();
}

void menu_item_4(void)
{
    int done = 0;

    init_system();

    while (!done)
    {
        int choice;

        printf("\n--- Results, tests and help ---\n");
        if (g_data.sensor)
        {
            printf("Current sensor : %s (%s)\n",
                   g_data.sensor->name, g_data.sensor->unit);
        }
        else
        {
            printf("Current sensor : (none)\n");
        }
        printf("Stored samples : %d\n", g_data.count);
        printf("\n");
        printf("1) Save measurements to file\n");
        printf("2) Load measurements from file\n");
        printf("3) Run conversion tests\n");
        printf("4) Show help\n");
        printf("5) Back to main menu\n");

        choice = get_int("Select option: ", 1, 5);

        switch (choice)
        {
            case 1:
                save_results();
                break;
            case 2:
                load_results();
                break;
            case 3:
                run_tests();
                break;
            case 4:
                show_help();
                break;
            default:
                done = 1;
                break;
        }
    }
}

/* -------------------- helper functions -------------------- */

static void init_system(void)
{
    if (g_initialised)
        return;

    g_sensor_count = 3;

    /* temperature sensor */
    strcpy(g_sensors[0].name, "Temp");
    strcpy(g_sensors[0].unit, "C");
    g_sensors[0].vref       = 3.3f;
    g_sensors[0].resolution = 1023;
    g_sensors[0].scale      = 50.0f;
    g_sensors[0].offset     = -40.0f;
    g_sensors[0].threshold  = 30.0f;

    /* pressure sensor */
    strcpy(g_sensors[1].name, "Press");
    strcpy(g_sensors[1].unit, "kPa");
    g_sensors[1].vref       = 3.3f;
    g_sensors[1].resolution = 1023;
    g_sensors[1].scale      = 90.0f;
    g_sensors[1].offset     = 0.0f;
    g_sensors[1].threshold  = 200.0f;

    /* light sensor */
    strcpy(g_sensors[2].name, "Light");
    strcpy(g_sensors[2].unit, "lux");
    g_sensors[2].vref       = 3.3f;
    g_sensors[2].resolution = 1023;
    g_sensors[2].scale      = 300.0f;
    g_sensors[2].offset     = 0.0f;
    g_sensors[2].threshold  = 200.0f;

    g_data.sensor = &g_sensors[0];
    g_data.count  = 0;

    g_initialised = 1;
}

static void print_sensor_list(void)
{
    int i;
    printf("\nAvailable sensors:\n");
    for (i = 0; i < g_sensor_count; ++i)
    {
        printf("  %d) %s (%s)\n",
               i + 1, g_sensors[i].name, g_sensors[i].unit);
    }
}

/* integer input using fgets + strtol */
static int get_int(const char *prompt, int min, int max)
{
    char buf[64];
    long value;
    char *endptr;

    for (;;)
    {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin))
        {
            printf("Input error.\n");
            return min;
        }
        buf[strcspn(buf, "\r\n")] = '\0';

        if (buf[0] == '\0')
        {
            printf("Please enter a number.\n");
            continue;
        }

        value = strtol(buf, &endptr, 10);
        if (*endptr != '\0')
        {
            printf("Enter an integer value.\n");
            continue;
        }
        if (value < min || value > max)
        {
            printf("Enter a value between %d and %d.\n", min, max);
            continue;
        }
        return (int)value;
    }
}

static float convert_adc(const Sensor *s, int adc)
{
    float voltage;
    float physical;

    if (!s) return 0.0f;

    if (adc < 0) adc = 0;
    if (adc > s->resolution) adc = s->resolution;

    voltage  = (float)adc / (float)s->resolution * s->vref;
    physical = s->scale * voltage + s->offset;
    return physical;
}

static int have_data(void)
{
    if (g_data.count <= 0)
    {
        printf("No measurement data available.\n");
        return 0;
    }
    return 1;
}

static void calc_stats(float *min, float *max, float *mean, int *above)
{
    int i;
    float mn, mx;
    double sum = 0.0;
    int cnt_above = 0;

    if (g_data.count <= 0)
        return;

    mn = g_data.values[0];
    mx = g_data.values[0];

    for (i = 0; i < g_data.count; ++i)
    {
        float v = g_data.values[i];
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        sum += v;
        if (g_data.sensor && v > g_data.sensor->threshold)
            cnt_above++;
    }

    if (min)   *min   = mn;
    if (max)   *max   = mx;
    if (mean)  *mean  = (float)(sum / g_data.count);
    if (above) *above = cnt_above;
}

static void show_plot(void)
{
    int rows = 10;
    int max_points = 40;
    int step;
    int i, r;
    float mn, mx, mean;
    int above;

    if (!have_data())
        return;

    calc_stats(&mn, &mx, &mean, &above);

    if (g_data.count > max_points)
        step = (g_data.count + max_points - 1) / max_points;
    else
        step = 1;

    printf("\nASCII plot for %s (%s)\n",
           g_data.sensor ? g_data.sensor->name : "unknown",
           g_data.sensor ? g_data.sensor->unit : "");

    for (r = rows; r >= 1; --r)
    {
        float level = mn + (mx - mn) * (float)r / (float)rows;
        printf("%7.2f | ", level);
        for (i = 0; i < g_data.count; i += step)
        {
            float v = g_data.values[i];
            if (v >= level)
                printf("*");
            else
                printf(" ");
        }
        printf("\n");
    }

    printf("         +");
    for (i = 0; i < g_data.count; i += step)
        printf("-");
    printf("\n");
}

static void batch_from_file(void)
{
    char filename[128];
    FILE *f;
    int adc;
    int loaded = 0;
    float mn, mx, mean;
    int above;

    printf("Enter filename containing ADC values: ");
    if (!fgets(filename, sizeof(filename), stdin))
    {
        printf("Input error.\n");
        return;
    }
    filename[strcspn(filename, "\r\n")] = '\0';

    f = fopen(filename, "r");
    if (!f)
    {
        printf("Could not open '%s'.\n", filename);
        return;
    }

    g_data.count = 0;

    while (fscanf(f, "%d", &adc) == 1)
    {
        if (g_data.count >= MAX_MEASUREMENTS)
        {
            printf("Buffer full, remaining values ignored.\n");
            break;
        }
        g_data.values[g_data.count++] = convert_adc(g_data.sensor, adc);
        loaded++;
    }

    fclose(f);

    printf("Loaded %d samples.\n", loaded);

    if (!have_data())
        return;

    calc_stats(&mn, &mx, &mean, &above);

    printf("\nStatistics for %s (%s)\n",
           g_data.sensor->name, g_data.sensor->unit);
    printf("  Samples           : %d\n", g_data.count);
    printf("  Min               : %.2f %s\n", mn, g_data.sensor->unit);
    printf("  Max               : %.2f %s\n", mx, g_data.sensor->unit);
    printf("  Mean              : %.2f %s\n", mean, g_data.sensor->unit);
    printf("  Above threshold   : %d (threshold %.2f %s)\n",
           above, g_data.sensor->threshold, g_data.sensor->unit);

    show_plot();
}

static void save_results(void)
{
    char filename[128];
    FILE *f;
    int i;
    time_t t;

    if (!have_data())
        return;

    printf("Enter filename to save results: ");
    if (!fgets(filename, sizeof(filename), stdin))
    {
        printf("Input error.\n");
        return;
    }
    filename[strcspn(filename, "\r\n")] = '\0';

    f = fopen(filename, "w");
    if (!f)
    {
        printf("Could not open '%s' for writing.\n", filename);
        return;
    }

    t = time(NULL);

    fprintf(f, "EnviroSense Results\n");
    fprintf(f, "Timestamp: %ld\n", (long)t);
    fprintf(f, "Sensor: %s\n", g_data.sensor ? g_data.sensor->name : "unknown");
    fprintf(f, "Unit: %s\n",   g_data.sensor ? g_data.sensor->unit  : "");
    fprintf(f, "Count: %d\n",  g_data.count);
    fprintf(f, "Values:\n");
    for (i = 0; i < g_data.count; ++i)
    {
        fprintf(f, "%.4f\n", g_data.values[i]);
    }

    fclose(f);
    printf("Saved %d values to '%s'.\n", g_data.count, filename);
}

static const Sensor *find_sensor(const char *name)
{
    int i;
    for (i = 0; i < g_sensor_count; ++i)
    {
        if (strcmp(g_sensors[i].name, name) == 0)
            return &g_sensors[i];
    }
    return NULL;
}

static void load_results(void)
{
    char filename[128];
    FILE *f;
    char line[128];
    char sensor_name[16] = "";
    char unit[8] = "";
    int count = 0;
    const Sensor *s;
    int loaded = 0;

    printf("Enter filename to load results: ");
    if (!fgets(filename, sizeof(filename), stdin))
    {
        printf("Input error.\n");
        return;
    }
    filename[strcspn(filename, "\r\n")] = '\0';

    f = fopen(filename, "r");
    if (!f)
    {
        printf("Could not open '%s'.\n", filename);
        return;
    }

    /* simple header parsing */
    if (!fgets(line, sizeof(line), f)) { fclose(f); printf("File is empty.\n"); return; }
    if (!fgets(line, sizeof(line), f)) { fclose(f); printf("Unexpected format.\n"); return; }
    if (fgets(line, sizeof(line), f))  sscanf(line, "Sensor: %15s", sensor_name);
    if (fgets(line, sizeof(line), f))  sscanf(line, "Unit: %7s", unit);
    if (fgets(line, sizeof(line), f))  sscanf(line, "Count: %d", &count);
    if (!fgets(line, sizeof(line), f)) { fclose(f); printf("Unexpected format.\n"); return; }

    s = find_sensor(sensor_name);
    if (!s)
    {
        printf("Warning: sensor '%s' not recognised.\n", sensor_name);
    }
    g_data.sensor = s;
    g_data.count  = 0;

    while (fgets(line, sizeof(line), f))
    {
        float v;
        if (sscanf(line, "%f", &v) == 1)
        {
            if (g_data.count < MAX_MEASUREMENTS)
            {
                g_data.values[g_data.count++] = v;
                loaded++;
            }
        }
    }
    fclose(f);

    printf("Loaded %d values from '%s'.\n", loaded, filename);
}

static void run_tests(void)
{
    Sensor *s;
    int adc_values[3];
    int i;
    int passed = 0;

    init_system();
    s = &g_sensors[0]; /* temperature sensor */

    adc_values[0] = 0;
    adc_values[1] = s->resolution / 2;
    adc_values[2] = s->resolution;

    for (i = 0; i < 3; ++i)
    {
        float a = convert_adc(s, adc_values[i]);
        float v = (float)adc_values[i] / (float)s->resolution * s->vref;
        float e = s->scale * v + s->offset;
        float diff = fabsf(a - e);

        printf("Test %d: ADC=%d expected=%.3f actual=%.3f diff=%.5f ",
               i + 1, adc_values[i], e, a, diff);

        if (diff < 0.001f)
        {
            printf("PASS\n");
            passed++;
        }
        else
        {
            printf("FAIL\n");
        }
    }

    printf("Summary: %d/%d tests passed.\n", passed, 3);
}

static void show_help(void)
{
    printf("\nEnviroSense help\n");
    printf("This program converts ADC readings from simple sensors into\n");
    printf("engineering units (C, kPa, lux) and provides basic statistics\n");
    printf("and an ASCII plot.\n\n");
    printf("Typical usage:\n");
    printf("  1) Use menu item 1 to select a sensor and clear old data.\n");
    printf("  2) Use item 2 for quick single conversions.\n");
    printf("  3) Use item 3 to load a file of ADC values and view stats.\n");
    printf("  4) Use item 4 to save or load results, run tests or view this help.\n");
}
