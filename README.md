# ESP32 Github Integration

This project integrates a esp32 micro-controller with github using HTTPS and
the github REST API. Github acts as a file server host which runs an automation
periodically updating `data/current.csv` from a selection of images available
in `data/images/`. The automations are implemented using github workflows.
The esp32 requests periodically the contents of `data/current.csv` to update a
tft display with the image content.

## File Structure

### main branch

```Shell
.
└── firmware
    ├── app # Main app that implements all features
    │   └── app.ino
    └── examples # Directory of code examples
        ├── file_fetch_example
        │   └── file_fetch_example.ino
        ├── hex_csv_to_int
        │   └── hex_csv_to_int.ino
        └── tft_adafruitlib_image_print
            └── tft_adafruitlib_image_print.ino
```

### image-data branch

```Shell
.
├── data # folder containing all data 
│   ├── current.csv # main file read by micro-controller
│   ├── images # Directory of all current available images (subject to change)
│   │   ├── alien_romulus.csv
│   │   ├── jaws.csv
│   │   └── scream.csv
│   └── lastfile.txt # record entry to prevent repeat image
└── scripts
    ├── random_file.sh # main script that gets called by workflow
    └── test.sh
```

## Image Content Files

The image content files are generated using [mischianti](https://mischianti.org/rgb-image-to-byte-array-converter-for-arduino-tft-displays/)
a free online RGB image to byte array converter. The tft screen used is a 1.14"
with dimensions of 135x240 pixels. The output format is in 565 color scheme and
using hex format. The project them converts this byte sequence into a csv file
that is pre-sanitised for whitespaces and control characters like `\n`, `\r`,
etc. The byte order is also in little-endian. The images have to be rotated 90
degrees anti-clockwise prior using the converter tool as the tft display driver
and libraries have difficulty aligning correctly with the right corner.
The files can be sanitised using AI or in my case I created a simple [partyrock
app](https://partyrock.aws/u/Matas-Noreika/RSjejiB7f/CSV-Whitespace-and-Control-Remover) to perform the sanitisation for me.

## Github Automations

Using github workflows I am able to achieve self updating file content without
needing any intervention. The github workflow default cron scheduling is poor
but can be worked around using [cron-job](https://console.cron-job.org/) to
call a workflow dispatch using my personal access token with partial permissions.
The automations only update the image-data branch to prevent any difficulty with
production on the main branch. This isolates the two environments while still
maintaining the benefits of github.
