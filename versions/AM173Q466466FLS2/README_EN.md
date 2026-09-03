<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 1.73″ AMOLED 466×466 (CO5300 · MIPI)</h1>

<p align="center"><b>Round AMOLED module · MIPI · CO5300 · capacitive touch</b></p>

<p align="center"><a href="./README.md">简体中文</a> | English · <a href="../../README_EN.md">Family index</a></p>

<p align="center">
  <img alt="Size: 1.73 inch" src="https://img.shields.io/badge/Size-1.73%22-3498DB?style=flat-square" />
  <img alt="Resolution: 466x466" src="https://img.shields.io/badge/Resolution-466%C3%97466-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: CO5300" src="https://img.shields.io/badge/Driver-CO5300-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 1.73 inch 466×466 AMOLED MIPI module (CO5300) product image" src="./images/product.png" width="640" /></p>

## Contents

- [Overview](#overview)
- [Specifications](#specifications)
- [Sample projects](#sample-projects)
- [Repository layout](#repository-layout)
- [Resources](#resources)
- [Buy](#buy)
- [Support](#support)

---

## Overview

OSPTEK **1.73″ 466×466 AMOLED** is a round **MIPI** color display module driven by **CO5300**, with capacitive touch (**CST9217**). Suited to wearables, round gauges, and compact circular HMI.

Spec ID (repository name): `1.73-amoled-466x466-mipi-co5300`

Current module version: **AM173Q466466FLS2**. Electrical and mechanical details follow [`docs/AM_173_Q466466_FLS_2_39c9366494.pdf`](./docs/AM_173_Q466466_FLS_2_39c9366494.pdf).

## Specifications

| Item | Spec |
| ---- | ---- |
| Size | 1.73 inch |
| Type | AMOLED (color, round) |
| Resolution | 466×466 |
| Interface | MIPI |
| Driver IC | CO5300 |
| Touch driver | CST9217 |

> Full outline, FPC definition, power, and timing follow the product datasheet / driver IC datasheet.

## Sample projects

| Description | Path |
| ---- | ---- |
| ESP32-P4 · CO5300 MIPI + esp-lvgl-port / LVGL9 | [`examples/P4-IDF_CO5300-MIPI_ESP-LVGL-PORT_V9/`](./examples/P4-IDF_CO5300-MIPI_ESP-LVGL-PORT_V9/) |
| ESP32-P4 · LVGL common demo + TE (IDF5) | [`examples/with-te/p4-idf_co5300-mipi_lvgl_common_demo/`](./examples/with-te/p4-idf_co5300-mipi_lvgl_common_demo/) |
| ESP32-P4 · LVGL common demo + TE (IDF6) | [`examples/with-te/p4-idf6_co5300-mipi_lvgl-common-demo/`](./examples/with-te/p4-idf6_co5300-mipi_lvgl-common-demo/) |
| ESP32-P4 · EAF player | [`examples/eaf/p4-idf_co5300-mipi_esp-lv-eaf-player/`](./examples/eaf/p4-idf_co5300-mipi_esp-lv-eaf-player/) |
| ESP32-P4 · FreeType font | [`examples/freetype/p4-idf_co5300-mipi_lvgl-freetype-font/`](./examples/freetype/p4-idf_co5300-mipi_lvgl-freetype-font/) |
| ESP32-P4 · Lottie player | [`examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player/`](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player/) |
| ESP32-P4 · Lottie batch loop | [`examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_batch-loop/`](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_batch-loop/) |
| ESP32-P4 · Lottie touch switch | [`examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_touch-switch/`](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_touch-switch/) |
| ESP32-P4 · MJPEG decode | [`examples/mjpeg/p4-idf_co5300-mipi_mjpeg-decode_lvgl-v9/`](./examples/mjpeg/p4-idf_co5300-mipi_mjpeg-decode_lvgl-v9/) |
| ESP32-P4 · Image decode | [`examples/image-decoder/p4-idf_co5300-mipi_lvgl-decode-image/`](./examples/image-decoder/p4-idf_co5300-mipi_lvgl-decode-image/) |
| ESP32-P4 · CO5300 MIPI display test | [`examples/display-touch-test/co5300_mipi_dsi/`](./examples/display-touch-test/co5300_mipi_dsi/) |
| ESP32-P4 · CST9217 touch I2C test | [`examples/display-touch-test/P4-IDF_CST9217-I2C/`](./examples/display-touch-test/P4-IDF_CST9217-I2C/) |
| Raspberry Pi 5 · CO5300 466×466 panel / DT overlay (display only) | [`examples/rpi5-panel-co5300-466x466/`](./examples/rpi5-panel-co5300-466x466/) |
| Raspberry Pi 5 · CST9217 touch / DT overlay (touch only) | [`examples/rpi5-touch-cst9217/`](./examples/rpi5-touch-cst9217/) |
| Raspberry Pi 5 · CO5300 display + CST9217 touch / DT overlay | [`examples/rpi5-panel-co5300-cst9217-466x466/`](./examples/rpi5-panel-co5300-cst9217-466x466/) |
| Raspberry Pi 5 · CO5300 + CST9217 · LVGL | [`examples/rpi5-lvgl-co5300-cst9217-466x466/`](./examples/rpi5-lvgl-co5300-cst9217-466x466/) |

## Repository layout

```text
1.73-amoled-466x466-mipi-co5300/                                # repo root (nav: ../../README_EN.md)
└── versions/
    └── AM173Q466466FLS2/                                # full materials for this part number
        ├── README.md
        ├── README_EN.md
        ├── images/
        ├── docs/
        └── examples/
```

## Resources

### Product files

| Resource | Link |
| ---- | ---- |
| Product datasheet (AM173Q466466FLS2) | [`docs/AM_173_Q466466_FLS_2_39c9366494.pdf`](./docs/AM_173_Q466466_FLS_2_39c9366494.pdf) |
| Driver IC datasheet (CO5300) | [`docs/CO_5300_Datasheet_V0_00_20230328_07edb82936.pdf`](./docs/CO_5300_Datasheet_V0_00_20230328_07edb82936.pdf) |
| 1.73″ AMOLED adapter schematic | [`docs/1.73寸AMOLED转接板原理图.png`](./docs/1.73%E5%AF%B8AMOLED%E8%BD%AC%E6%8E%A5%E6%9D%BF%E5%8E%9F%E7%90%86%E5%9B%BE.png) |
| 1.73″ AMOLED MIPI adapter board (V2.0) | [`docs/PCB-1.73寸AMOLED屏MIPI转接板V2.0.pdf`](./docs/PCB-1.73%E5%AF%B8AMOLED%E5%B1%8FMIPI%E8%BD%AC%E6%8E%A5%E6%9D%BFV2.0.pdf) |
| Connector datasheet (OK-14F024-04) | [`docs/OK-14F024-04.pdf`](./docs/OK-14F024-04.pdf) |

### Samples

- [ESP32-P4 CO5300 MIPI + LVGL9](./examples/P4-IDF_CO5300-MIPI_ESP-LVGL-PORT_V9/)
- [ESP32-P4 LVGL + TE (IDF5)](./examples/with-te/p4-idf_co5300-mipi_lvgl_common_demo/)
- [ESP32-P4 LVGL + TE (IDF6)](./examples/with-te/p4-idf6_co5300-mipi_lvgl-common-demo/)
- [ESP32-P4 EAF](./examples/eaf/p4-idf_co5300-mipi_esp-lv-eaf-player/)
- [ESP32-P4 FreeType](./examples/freetype/p4-idf_co5300-mipi_lvgl-freetype-font/)
- [ESP32-P4 Lottie](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player/)
- [ESP32-P4 Lottie batch loop](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_batch-loop/)
- [ESP32-P4 Lottie touch switch](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_touch-switch/)
- [ESP32-P4 MJPEG](./examples/mjpeg/p4-idf_co5300-mipi_mjpeg-decode_lvgl-v9/)
- [ESP32-P4 image decode](./examples/image-decoder/p4-idf_co5300-mipi_lvgl-decode-image/)
- [ESP32-P4 display test](./examples/display-touch-test/co5300_mipi_dsi/)
- [ESP32-P4 CST9217 touch test](./examples/display-touch-test/P4-IDF_CST9217-I2C/)
- [Raspberry Pi 5 CO5300 panel (display only)](./examples/rpi5-panel-co5300-466x466/)
- [Raspberry Pi 5 CST9217 touch (touch only)](./examples/rpi5-touch-cst9217/)
- [Raspberry Pi 5 CO5300 display + CST9217 touch](./examples/rpi5-panel-co5300-cst9217-466x466/)
- [Raspberry Pi 5 CO5300 + CST9217 · LVGL](./examples/rpi5-lvgl-co5300-cst9217-466x466/)

## Buy

<p align="center">
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="AliExpress store" src="https://img.shields.io/badge/AliExpress-Official_Store-FF6A00?style=for-the-badge" /></a>
  &nbsp;&nbsp;
  <a href="https://shop110742373.taobao.com/"><img alt="Taobao store" src="https://img.shields.io/badge/Taobao-Official_Store-FF6A00?style=for-the-badge" /></a>
</p>

**Overseas (AliExpress)**

- Store: [OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

**China (Taobao)**

- Store: [鱼鹰光电工厂店](https://shop110742373.taobao.com/)

## Support

- Technical support / product inquiry: <luyu@osptek.com>
- QQ group (China): **985881096**
- Website: <https://osptek.com/>
- Feel free to open an Issue in this repository if you have any questions

---

<p align="center"><sub>© 2026 OSPTEK · Materials in this repository are licensed under CC BY 4.0</sub></p>
