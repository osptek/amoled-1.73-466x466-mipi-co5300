<p align="left"><img alt="OSPTEK" src="./images/logo.png" width="200" /></p>

<h1 align="center">OSPTEK 1.73″ AMOLED 466×466（CO5300 · MIPI）</h1>

<p align="center"><b>圆形 AMOLED 模组 · MIPI · CO5300 · 电容触摸</b></p>

<p align="center"><a href="./README_EN.md">English</a> | 简体中文 · <a href="../../README.md">规格族索引</a></p>

<p align="center">
  <img alt="Size: 1.73 inch" src="https://img.shields.io/badge/Size-1.73%22-3498DB?style=flat-square" />
  <img alt="Resolution: 466x466" src="https://img.shields.io/badge/Resolution-466%C3%97466-8E44AD?style=flat-square" />
  <img alt="Interface: MIPI" src="https://img.shields.io/badge/Interface-MIPI-27AE60?style=flat-square" />
  <img alt="Driver: CO5300" src="https://img.shields.io/badge/Driver-CO5300-E7352C?style=flat-square" />
</p>

<p align="center"><img alt="OSPTEK 1.73 寸 466×466 AMOLED MIPI 模组（CO5300）宣传图" src="./images/product.png" width="640" /></p>

## 目录

- [产品简介](#产品简介)
- [规格参数](#规格参数)
- [示例工程](#示例工程)
- [仓库结构](#仓库结构)
- [相关资料](#相关资料)
- [购买链接](#购买链接)
- [技术支持](#技术支持)

---

## 产品简介

OSPTEK **1.73 寸 466×466 AMOLED** 是一款 **MIPI** 接口圆形彩色显示模组，显示驱动为 **CO5300**，触摸驱动为 **CST9217**。适合穿戴表盘、圆形仪表与小型圆形 HMI 等场景。

规格标识（仓库名）：`1.73-amoled-466x466-mipi-co5300`

当前模组版本：**AM173Q466466FLS2**。电气与外形细节以 [`docs/AM_173_Q466466_FLS_2_39c9366494.pdf`](./docs/AM_173_Q466466_FLS_2_39c9366494.pdf) 为准。

## 规格参数

| 项目 | 规格 |
| ---- | ---- |
| 尺寸 | 1.73 英寸 |
| 类型 | AMOLED（彩色，圆形） |
| 分辨率 | 466×466 |
| 接口 | MIPI |
| 驱动 IC | CO5300 |
| 触摸驱动 | CST9217 |

> 完整外形尺寸、FPC 定义、供电与时序以产品规格书 / 驱动手册为准。

## 示例工程

| 说明 | 路径 |
| ---- | ---- |
| ESP32-P4 · CO5300 MIPI + esp-lvgl-port / LVGL9 | [`examples/P4-IDF_CO5300-MIPI_ESP-LVGL-PORT_V9/`](./examples/P4-IDF_CO5300-MIPI_ESP-LVGL-PORT_V9/) |
| ESP32-P4 · LVGL 通用演示 + TE（IDF5） | [`examples/with-te/p4-idf_co5300-mipi_lvgl_common_demo/`](./examples/with-te/p4-idf_co5300-mipi_lvgl_common_demo/) |
| ESP32-P4 · LVGL 通用演示 + TE（IDF6） | [`examples/with-te/p4-idf6_co5300-mipi_lvgl-common-demo/`](./examples/with-te/p4-idf6_co5300-mipi_lvgl-common-demo/) |
| ESP32-P4 · EAF 动画播放 | [`examples/eaf/p4-idf_co5300-mipi_esp-lv-eaf-player/`](./examples/eaf/p4-idf_co5300-mipi_esp-lv-eaf-player/) |
| ESP32-P4 · FreeType 字体 | [`examples/freetype/p4-idf_co5300-mipi_lvgl-freetype-font/`](./examples/freetype/p4-idf_co5300-mipi_lvgl-freetype-font/) |
| ESP32-P4 · Lottie 播放 | [`examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player/`](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player/) |
| ESP32-P4 · Lottie 批量循环 | [`examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_batch-loop/`](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_batch-loop/) |
| ESP32-P4 · Lottie 触摸切换 | [`examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_touch-switch/`](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_touch-switch/) |
| ESP32-P4 · MJPEG 解码 | [`examples/mjpeg/p4-idf_co5300-mipi_mjpeg-decode_lvgl-v9/`](./examples/mjpeg/p4-idf_co5300-mipi_mjpeg-decode_lvgl-v9/) |
| ESP32-P4 · 图片解码 | [`examples/image-decoder/p4-idf_co5300-mipi_lvgl-decode-image/`](./examples/image-decoder/p4-idf_co5300-mipi_lvgl-decode-image/) |
| ESP32-P4 · CO5300 MIPI 显示测试 | [`examples/display-touch-test/co5300_mipi_dsi/`](./examples/display-touch-test/co5300_mipi_dsi/) |
| ESP32-P4 · CST9217 触摸 I2C 测试 | [`examples/display-touch-test/P4-IDF_CST9217-I2C/`](./examples/display-touch-test/P4-IDF_CST9217-I2C/) |
| Raspberry Pi 5 · CO5300 466×466 面板驱动 / DT overlay（仅显示） | [`examples/rpi5-panel-co5300-466x466/`](./examples/rpi5-panel-co5300-466x466/) |
| Raspberry Pi 5 · CST9217 触摸驱动 / DT overlay（仅触摸） | [`examples/rpi5-touch-cst9217/`](./examples/rpi5-touch-cst9217/) |
| Raspberry Pi 5 · CO5300 显示 + CST9217 触摸 / DT overlay | [`examples/rpi5-panel-co5300-cst9217-466x466/`](./examples/rpi5-panel-co5300-cst9217-466x466/) |

## 仓库结构

```text
1.73-amoled-466x466-mipi-co5300/                                # 仓库根（导航见 ../../README.md）
└── versions/
    └── AM173Q466466FLS2/                                # 本料号完整资料
        ├── README.md
        ├── README_EN.md
        ├── images/
        ├── docs/
        └── examples/
```

## 相关资料

### 本产品资料

| 资料 | 链接 |
| ---- | ---- |
| 产品规格书（AM173Q466466FLS2） | [`docs/AM_173_Q466466_FLS_2_39c9366494.pdf`](./docs/AM_173_Q466466_FLS_2_39c9366494.pdf) |
| 驱动 IC 数据手册（CO5300） | [`docs/CO_5300_Datasheet_V0_00_20230328_07edb82936.pdf`](./docs/CO_5300_Datasheet_V0_00_20230328_07edb82936.pdf) |
| 1.73 寸 AMOLED 转接板原理图 | [`docs/1.73寸AMOLED转接板原理图.png`](./docs/1.73%E5%AF%B8AMOLED%E8%BD%AC%E6%8E%A5%E6%9D%BF%E5%8E%9F%E7%90%86%E5%9B%BE.png) |
| 1.73 寸 AMOLED MIPI 转接板（V2.0） | [`docs/PCB-1.73寸AMOLED屏MIPI转接板V2.0.pdf`](./docs/PCB-1.73%E5%AF%B8AMOLED%E5%B1%8FMIPI%E8%BD%AC%E6%8E%A5%E6%9D%BFV2.0.pdf) |
| 连接器规格书（OK-14F024-04） | [`docs/OK-14F024-04.pdf`](./docs/OK-14F024-04.pdf) |

### 示例工程

- [ESP32-P4 CO5300 MIPI + LVGL9](./examples/P4-IDF_CO5300-MIPI_ESP-LVGL-PORT_V9/)
- [ESP32-P4 LVGL + TE（IDF5）](./examples/with-te/p4-idf_co5300-mipi_lvgl_common_demo/)
- [ESP32-P4 LVGL + TE（IDF6）](./examples/with-te/p4-idf6_co5300-mipi_lvgl-common-demo/)
- [ESP32-P4 EAF](./examples/eaf/p4-idf_co5300-mipi_esp-lv-eaf-player/)
- [ESP32-P4 FreeType](./examples/freetype/p4-idf_co5300-mipi_lvgl-freetype-font/)
- [ESP32-P4 Lottie](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player/)
- [ESP32-P4 Lottie 批量循环](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_batch-loop/)
- [ESP32-P4 Lottie 触摸切换](./examples/lottie/p4-idf_co5300-mipi_lvgl-lottie-player_touch-switch/)
- [ESP32-P4 MJPEG](./examples/mjpeg/p4-idf_co5300-mipi_mjpeg-decode_lvgl-v9/)
- [ESP32-P4 图片解码](./examples/image-decoder/p4-idf_co5300-mipi_lvgl-decode-image/)
- [ESP32-P4 显示测试](./examples/display-touch-test/co5300_mipi_dsi/)
- [ESP32-P4 CST9217 触摸测试](./examples/display-touch-test/P4-IDF_CST9217-I2C/)
- [Raspberry Pi 5 CO5300 面板（仅显示）](./examples/rpi5-panel-co5300-466x466/)
- [Raspberry Pi 5 CST9217 触摸（仅触摸）](./examples/rpi5-touch-cst9217/)
- [Raspberry Pi 5 CO5300 显示 + CST9217 触摸](./examples/rpi5-panel-co5300-cst9217-466x466/)

## 购买链接

<p align="center">
  <a href="https://shop110742373.taobao.com/"><img alt="淘宝官方店铺" src="https://img.shields.io/badge/淘宝-官方店铺-FF6A00?style=for-the-badge" /></a>
  &nbsp;&nbsp;
  <a href="https://www.aliexpress.com/store/1105701619"><img alt="速卖通官方店铺" src="https://img.shields.io/badge/速卖通-官方店铺-FF6A00?style=for-the-badge" /></a>
</p>

**国内（淘宝）**

- 店铺：[鱼鹰光电工厂店](https://shop110742373.taobao.com/)

**海外（AliExpress）**

- 店铺：[OSPTEK Official Store](https://www.aliexpress.com/store/1105701619)

## 技术支持

- 技术支持 / 产品咨询：<luyu@osptek.com>
- QQ 技术交流群：**985881096**
- 公司官网：<https://osptek.com/>
- 有任何问题，都可以在本仓库 Issues 中提问

---

<p align="center"><sub>© 2026 OSPTEK 鱼鹰光电 · 本仓库资料采用 CC BY 4.0 许可</sub></p>
