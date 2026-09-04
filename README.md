# Smart Traffic Lights Plugin

[![Steam Workshop](https://img.shields.io/badge/Steam-Workshop-blue?logo=steam)](https://steamcommunity.com/sharedfiles/filedetails/?id=3795470803)

**Download from Steam Workshop:** [Smart Traffic Lights (ID: 3795470803)](https://steamcommunity.com/sharedfiles/filedetails/?id=3795470803)

A plugin for TesmioLoader (Workers & Resources: Soviet Republic) that implements intelligent traffic lights at road junctions.

## Features
* **Dynamic Green Phases**: The plugin monitors vehicles waiting at junctions that are operating in "Traffic Light" mode.
* **Wait-Time Reduction**: If a road currently has a green light but no cars are waiting on it, and there are cars waiting on other red roads, the traffic light will immediately fast-forward to the yellow phase to let the waiting cars go earlier.

## Installation
1. Ensure you have the TesmioLoader framework installed.
2. Download or compile the plugin, and copy `smart_traffic_lights.dll` to your game directory:
   `Steam\steamapps\common\SovietRepublic\tesmioloader\build\plugins\`
3. Activate the plugin via the `tesmiolauncher.exe` interface.
