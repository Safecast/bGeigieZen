bGeigie-Zen firmware (ESP32)
============================

Quick Start
-----------

Pre-requisites: python3

1. Install [Platform IO CLI](https://docs.platformio.org/en/latest/core/installation.html)
  
        python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"
  I haven't tried this step myself, but if it fails, try to follow the instructions on platform.io's website. Rob is using platform.io within VS Code. The link for that is [here](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start). Advantage of VSCode/VSCodium is the interaction for code development. https://vscodium.com/
  
3. Clone the repository
  
        git clone git@github.com:Safecast/bGeigieZen.git
  
4. Change branch to Robin dev
  
        cd bGeigie-Raku
        git checkout bGeigieZen-Dev
  
5. Change directory
  
        cd bgeigie_firmware_pio
  
6. Connect the device using USB, compile, and upload
  
        pio run -t upload
  
