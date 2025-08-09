# Safecast bGeigie Transition: From Nano to Zen

## Executive Summary

The transition from the Safecast bGeigieNano to bGeigieZen represents a significant evolution in citizen science radiation monitoring technology, spanning over a decade of development and community feedback. This transition addresses critical challenges in accessibility, usability, and technological advancement while maintaining the core mission of open-source environmental monitoring.[^1][^2][^3]

## Historical Context and Development Timeline

### Background: The Foundation (2011-2013)

Safecast originated immediately following the Fukushima Daiichi nuclear disaster on March 11, 2011, founded by Sean Bonner, Pieter Franken, and Joi Ito. The organization emerged from a critical need for independent radiation monitoring and transparent data sharing when commercially available Geiger counters became unavailable and official information was perceived as insufficient.[^1][^4]

The bGeigieNano, released in mid-2013, became Safecast's primary workhorse for radiation mapping efforts. As a mobile, GPS-enabled logging radiation sensor designed for mounting on car windows, bicycles, trains, and planes, it enabled widespread citizen-led radiation mapping across the globe. More than 1,500 bGeigie Nano kits were built and deployed worldwide, contributing to what would become over 250 million environmental measurements in Safecast's open dataset.[^5][^6][^7]

### The Nano Era: Success and Limitations (2013-2022)

The bGeigieNano proved highly successful in its mission, with almost all data visible on Safecast's online map collected using this device. The kit was professionally tested in laboratories across Japan, Germany, and the US, winning the Good Design award in 2013. In 2015, Safecast extended the Nano with a Bluetooth interface (BLEBee) enabling iOS and Android connectivity.[^7]

However, by 2022, critical challenges emerged that would ultimately necessitate the transition to a new device:

**Parts Availability Crisis**: By July 2022, essential components became increasingly difficult to procure, including the GM pancake tube (LND-7317) and high-voltage power supply (iRover from International Medcom). The distributor Kithub.cc folded, eliminating the primary source for complete kits.[^8][^9]

**Assembly Complexity**: The Nano required 60 components and 4-5 hours of assembly time, including extensive soldering, which created barriers for many potential users. This complexity particularly deterred individuals without technical electronics experience.[^2][^7]

**Technological Limitations**: The Arduino Fio processor provided only 32KB flash memory and 2KB RAM, limiting potential for advanced features and future enhancements. The monochrome 128×64 OLED display provided limited information display capabilities.[^2]

### The Zen Development Journey (2021-2025)

#### Initial Announcement and Prototyping (2021)

In April 2021, Safecast announced the development of bGeigieZen, describing it as "an updated version of the bGeigieNano with a lot of build-in additional features like wireless uploading of data, wireless charging, replaceable LiPo battery". The device was introduced as being based on the M5Stack modular IoT platform, utilizing the SafePulse sensor board developed specifically for this project.[^1]

The first public demonstration occurred at FabCafe Tokyo on April 22, 2021, during an Earth Day event where the bGeigieZen prototype was showcased alongside other environmental monitoring devices.[^10]

#### Extended Development Period (2021-2024)

The development process proved more complex and time-consuming than initially anticipated. Unlike the Nano's relatively straightforward Arduino-based design, the Zen required developing entirely new hardware and software architectures:

**Hardware Challenges**: The team needed to develop the SafePulse high-voltage power supply system as an open-source alternative to the proprietary iRover. This involved creating a compact 4-layer PCB design based on HEX CMOS inverter oscillator, MOSFET switch, and Cockroft-Walton multiplier architecture.[^11]

**Software Framework Development**: Building on the M5Stack platform required developing new firmware from scratch, including GPS management, real-time data transmission, touchscreen interface, and power optimization systems.[^12]

**Integration Challenges**: Coordinating multiple modern technologies - M5Stack CoreS3 processor, advanced GPS modules, wireless connectivity, and touchscreen interface - required extensive testing and refinement.[^12]

#### Production and Launch (2024-2025)

**Early 2024**: Software framework establishment with core functionality including GPS, real-time clock, SD card functionality, and wireless capabilities.[^12]

**February 2024**: First production PCB boards (V3.0.3) manufactured and initial units assembled.[^12]

**April 2024**: First customer shipments began with initial batch of 20 units.[^12]

**August 2024**: First production run sold out, with transition beginning to V4.x.x generation featuring improved design and 3D-printed tube covers.[^13][^12]

**October 2024**: Official public announcement of the bGeigieZen as the successor to the bGeigieNano.[^3]

**January 2025**: 100th bGeigieZen delivered with 100% manufacturing success rate.[^12]

## Technical Evolution and Improvements

### Fundamental Design Philosophy

The bGeigieZen embodies a "Zen" philosophy of simplified design and enhanced user experience. This is immediately evident in the dramatic reduction from 60 components in the Nano to just 18 components in the Zen, reducing assembly time from 4-5 hours to approximately 30 minutes.[^2][^3]

### Hardware Specifications Comparison

| Specification | bGeigie Nano      | bGeigie Zen                  | Improvement Factor            |
| :------------ | :---------------- | :--------------------------- | :---------------------------- |
| Components    | 60                | 18                           | 70% reduction                 |
| Assembly Time | 4-5 hours         | 30 minutes                   | 87% reduction                 |
| Flash Memory  | 32KB              | 16MB                         | 500x increase                 |
| RAM Memory    | 2KB               | 8MB                          | 4,000x increase               |
| Display       | 128×64 Monochrome | 320×240 Color Touchscreen    | 6x resolution, color, touch   |
| Battery       | 2000mAh fixed     | 4000mAh removable            | 2x capacity, user-serviceable |
| Connectivity  | Optional BLE      | Integrated Wi-Fi, BLE, USB-C | Native wireless capabilities  |

### Advanced Features and Capabilities

**Enhanced GPS Performance**: The Zen utilizes the Bzgnss BZ-251GPS module with M10 series chipset, providing access to approximately 10 additional satellites compared to the Nano's Adafruit Ultimate GPS, significantly improving location accuracy.[^2][^12]

**Real-time Monitoring**: Unlike the Nano, which required additional modules for wireless data transmission, the Zen includes integrated real-time monitoring capabilities, automatically uploading data every 5 minutes in fixed mode or every 5 seconds in mobile mode.[^3]

**Modern User Interface**: The 2-inch IPS LCD color touchscreen with 320×240 resolution provides intuitive interaction, displaying QR codes for easy smartphone access to online data, satellite tracking information, and comprehensive device status.[^2]

**Power Management**: The removable 18650 battery system allows for continuous operation through battery swapping, with charging time reduced from 20 hours to 4 hours through high-current charging capabilities. Wireless charging support through optional cradle further enhances user convenience.[^12][^2]

**Firmware Updates**: Multiple update pathways including Over-The-Air (OTA), Wi-Fi, Bluetooth, and USB-C connections eliminate the technical barriers that previously required FTDI connections for Nano updates.[^2]

## Development Challenges and Solutions

### Technical Challenges

**GPS Accuracy Optimization**: Extensive development effort focused on improving GPS performance, particularly for airborne applications where standard modules fail due to altitude and speed restrictions. The team developed specialized firmware configurations and testing protocols to optimize satellite acquisition and retention.[^12][^14][^15]

**Power Consumption Optimization**: Achieving extended battery life required sophisticated software optimization across different operating modes. Through firmware refinements, battery life was extended from an initial 16 hours to up to 45 hours depending on operational mode.[^15][^12]

**Manufacturing Scalability**: Transitioning from the Nano's component-by-component assembly to the Zen's integrated approach required developing new manufacturing processes, quality control procedures, and supply chain management.[^12]

### Community Integration Challenges

**API Compatibility**: Ensuring seamless integration with Safecast's existing data infrastructure required extensive testing and refinement of data formats and transmission protocols.[^16]

**User Education**: The transition from familiar Arduino-based architecture to M5Stack platform necessitated developing new documentation, training materials, and support resources.[^12]

**Cost Management**: Balancing advanced features with accessibility required careful component selection and design optimization to maintain reasonable pricing while significantly enhancing capabilities.[^3]

## Current State and Production Status

### Manufacturing and Availability

As of August 2025, Safecast has successfully delivered over 100 bGeigieZen units with a 100% manufacturing success rate. The device is currently available in two configurations:[^12]

**Standard Model**: Includes TP4056 battery controller module for comprehensive charging management, priced at \$475 for kit form and \$750 for assembled units.[^17][^18]

**NFW (Not For Wimp) Model**: Minimalist design without TP4056 controller, relying on wireless charging and M5Stack Core USB-C charging for a cleaner aesthetic.[^18][^17]

### Ongoing Development

**Version 4.x.x Generation**: Current production focuses on the fourth-generation design featuring enhanced PCB layout, 3D-printed tube covers replacing metal grids, and improved component integration based on user feedback from earlier versions.[^12]

**Software Enhancements**: Regular firmware updates continue adding features including:

- Dual WiFi network support for seamless connectivity[^12]
- Enhanced GPS memory retention for faster startup[^12]
- Improved power optimization across all operational modes[^12]
- Audio feedback system providing authentic Geiger counter sounds[^12]
- Special aviation mode for high-altitude data collection[^12]

### Community Adoption and Applications

**Educational Integration**: Multiple universities including Amsterdam University of Applied Sciences and Humboldt-Universität zu Berlin have adopted the bGeigieZen for teaching and research applications.[^12]

**Research Applications**: Researchers are utilizing the device for various projects including UAV-mounted radiation mapping and environmental monitoring around nuclear facilities.[^2]

**Global Deployment**: Units have been deployed worldwide, including specialized installations for real-time monitoring near nuclear facilities such as the Pickering Nuclear Generating Station in Canada.[^12]

## Future Outlook and Roadmap

### Hardware Evolution

**Next-Generation Development**: Early alpha development of Version 5.x.x is underway, featuring potential dual 18650 battery configurations and advanced counting chip integration.[^12]

**Connectivity Expansion**: Future versions may include 4G and satellite connectivity capabilities for enhanced data transmission in remote areas.[^3]

**Sensor Integration**: The Grove I2C connector allows for easy integration of additional environmental sensors, expanding monitoring capabilities beyond radiation detection.[^2]

### Software and Platform Development

**Mobile Application**: The bGeigie-Drive Android application is nearing release, providing direct smartphone interface for data visualization and management, with iOS version planned for later release.[^12]

**Real-time Infrastructure**: Continued development of real-time data infrastructure including Grafana dashboards and improved API integration for immediate data availability.[^12]

**Open Source Commitment**: Ongoing transition to fully open-source hardware and software ensures long-term community sustainability and eliminates proprietary dependencies.[^14]

## Conclusion

The transition from bGeigieNano to bGeigieZen represents a paradigmatic shift in citizen science instrumentation, successfully addressing the critical challenges that threatened the sustainability of Safecast's radiation monitoring mission. Through innovative engineering, community-driven design, and sustained development effort, the bGeigieZen has evolved from a promising concept announced in 2021 to a production-ready device that significantly surpasses its predecessor in capabilities, usability, and accessibility.

The Zen's success is measured not only in technical specifications - with 500x increase in memory capacity, 87% reduction in assembly time, and integrated wireless capabilities - but in its proven ability to maintain Safecast's core mission of open, accessible environmental monitoring while adapting to contemporary technological standards and user expectations.

This transition demonstrates the resilience and adaptability of open-source citizen science projects, showing how community feedback, technological advancement, and persistent development can overcome significant challenges to produce solutions that serve both immediate practical needs and long-term scientific goals. As the bGeigieZen continues to evolve through ongoing development cycles, it establishes a foundation for the next decade of citizen-led environmental monitoring, ensuring that Safecast's mission of transparency, accessibility, and data democratization remains viable and effective in an increasingly complex technological landscape.

The successful completion of this transition, marked by the delivery of over 100 units with perfect manufacturing reliability, validates both the technical approach and the community-driven development model that has characterized Safecast since its founding in response to the Fukushima disaster. The bGeigieZen stands as evidence that citizen science can not only adapt to technological change but can drive innovation in service of public interest and environmental stewardship.

<div style="text-align: center">⁂</div>

[^1]: https://github.com/Safecast/bGeigieNanoKit

[^2]: https://bgeigiezen.safecast.jp

[^3]: https://safecast.org/devices/

[^4]: https://en.wikipedia.org/wiki/Safecast

[^5]: https://bgeigiezen.safecast.jp/features/

[^6]: https://www.saveecobot.com/en/platform/savednipro-and-safecast

[^7]: https://safecast.org/history-of-safecast/

[^8]: https://github.com/Safecast/bGeigieZen

[^9]: https://www.hackster.io/rob-oudendijk/m5stack-geiger-counter-gps-https-bgeigiezen-safecast-jp-7edd7d

[^10]: https://safecast.org/devices/bgeigie-nano/

[^11]: https://safecast.org/bgeigiezen-ongoing-improvements/

[^12]: https://safecast.org

[^13]: https://pmc.ncbi.nlm.nih.gov/articles/PMC11679441/

[^14]: https://github.com/Safecast/bGeigie-Drive

[^15]: https://www.sciencedirect.com/science/article/abs/pii/S0265931X20301077

[^16]: https://ui.adsabs.harvard.edu/abs/2023EPJST.232.1465K/abstract

[^17]: https://safecast.org/devices/bgeigie-zen/

[^18]: https://www.globalgiving.org/projects/safecast/reports/

[^19]: https://groups.google.com/g/safecast-devices/c/esA75hxIJ7Q

[^20]: https://fabcafe.com/events/tokyo/read-the-air-with-safecast

[^21]: https://safecast.org/safecast-introduces-our-latest-open-source-geiger-counter-the-bgeigiezen/

[^22]: https://safecast.org/wp-content/uploads/2017/10/safecastreport2017-part1safecastproject-final-171004011228.pdf

[^23]: https://paragraph.com/@safecast/bgeigiezen-ongoing-improvements-safecast

[^24]: https://safecast.org/frequently-asked-questions/about-calibration-and-the-bgeigie-nano/

[^25]: https://bgeigiezen.safecast.jp/news-updates/

[^26]: https://www.linkedin.com/posts/roboudendijk_interested-search-result-of-chatgpt-when-activity-7259396969834401793-foKB

[^27]: https://www.academia.edu/25071817/THE_SAFECAST_REPORT_Vol2

[^28]: https://registrydocumentsprd.blob.core.windows.net/commentsblob/project-88771/comment-62155/Bruce_C%20IPD%20Bertrand.pdf

[^29]: https://github.com/Safecast/safepulse

[^30]: https://fablabbcn.org/wp-content/uploads/2020/09/Fab-City-The-Mass-Distribution-of-Almost-Everything.pdf

[^31]: https://safecast.org/news/

[^32]: https://core.ac.uk/download/420848445.pdf

[^33]: https://bgeigiezen.safecast.jp/store-2/bgeigiezen-assembled/

[^34]: https://forum.digikey.com/t/m5stack-empowering-a-new-era-of-modular-iot-development-platform/47140

[^35]: https://bgeigiezen.safecast.jp/store-2/bgeigiezen-kit/

[^36]: https://community.platformio.org/t/m5stack-doesnt-compile/4789

[^37]: https://www.linkedin.com/posts/safecast_we-sold-out-all-our-bgeigiezen-thanks-activity-7228768187595599873-oxK_

[^38]: https://www.youtube.com/watch?v=LPdh39GiYSA

[^39]: https://community.platformio.org/t/m5stack-paper-help-helloworld-does-not-execute-default-platform-ini-seems-broken-i-have-a-working-guess/28186

[^40]: https://www.youtube.com/watch?v=yJ0mVvhxbIw

[^41]: https://groups.google.com/g/safecast-devices/c/Lbw-cPNv-10

[^42]: https://openelab.io/blogs/learn/everything-you-need-to-know-about-m5stack-core2

[^43]: https://bgeigiezen.safecast.jp/ukraine/

[^44]: https://github.com/Safecast/bGeigieZen/issues

[^45]: https://groups.google.com/g/safecast-devices/c/2extZt-8fTA

[^46]: https://bgeigiezen.safecast.jp/store-2/bgeigiecast-kit-for-bgeigienano/

[^47]: https://groups.google.com/g/safecast-devices

[^48]: https://bgeigiezen.safecast.jp/store-2/bgeigienano/

[^49]: https://academic.oup.com/rpd/article/199/8-9/775/7177427?rss=1

[^50]: https://bgeigiezen.safecast.jp/documents/

[^51]: https://nl.linkedin.com/in/jlvillarbardanca/en

[^52]: https://safecast.org/frequently-asked-questions/

[^53]: https://ppl-ai-code-interpreter-files.s3.amazonaws.com/web/direct-files/2e480666f18d5563628aaf06fbc3e0ec/a87a6787-5659-446e-a114-0f308acc445d/823011c5.csv

[^54]: https://ppl-ai-code-interpreter-files.s3.amazonaws.com/web/direct-files/2e480666f18d5563628aaf06fbc3e0ec/5e622b1e-a21f-46bf-9c39-c50f4bfbd801/798cd636.csv
