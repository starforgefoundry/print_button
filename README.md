# Print Button

M5Stack AtomS3 Lite firmware that shows a Bambuddy printer's state and clears the plate on press.

| LED | Meaning |
| --- | --- |
| Red | Printing |
| Yellow | Print finished, waiting for the plate to be cleared. Press to clear. |
| Green | Idle |
| Blue pulse | Connecting to Wi-Fi, or setup hotspot is up |
| Cyan pulse | No printer selected |
| Dim magenta | Printer or Bambuddy unreachable |
| Magenta blink | Bambuddy rejected the API key |
| White blink | Identify (10 s) |
| White flash | Plate clear accepted |

## Build and flash

```sh
cp firmware/secrets.ini.example firmware/secrets.ini   # optional defaults
mise run flash                                          # PORT=/dev/cu.usbserial-xxx mise run flash
```

`secrets.ini` values are defaults baked into every button. Settings saved in the web UI take priority.

## Setup

1. Flash every button with the same `secrets.ini` so they all join Wi-Fi.
2. Open any button at `http://printbutton-xxxxxx.local`. The "Buttons on this network" card lists the others.
3. On each button, press **Identify** to find it, then pick its printer and save.

Without Wi-Fi credentials, or if the network can't be joined within 20 s, the button starts a `printbutton-xxxxxx` hotspot with a captive portal.

The API key needs the **Control printer** permission in Bambuddy.

## UI development

```sh
DEVICE_URL=http://<button-ip> mise run ui:dev
```
