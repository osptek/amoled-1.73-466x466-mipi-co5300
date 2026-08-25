# Hynitron CST9217 Touchscreen Driver

Minimal Linux kernel driver for Hynitron (海栎创) CST9217 capacitive touch controller.

本例程来源于鱼鹰光电的工程师的开源分享 [Github.com/osptek](https://github.com/osptek)，欢迎提出改进意见。

---

## 1. 文件说明

| 文件 | 说明 |
|------|------|
| `hynitron_cst9217.c` | 驱动源码 |
| `Makefile` | 编译与安装脚本 |
| `cst9217-touch.dts` | 设备树 overlay（仅触摸） |
| `README.md` | 本说明文件 |

---

## 2. 编译驱动


> 安装编译依赖（如果还没有）
> 
```
# 更新软件包列表
sudo apt update

# 安装编译工具链与匹配的内核头文件
sudo apt install build-essential linux-headers-$(uname -r)

# 创建文件夹并进入
mkdir cst9217-touch && cd cst9217-touch

sudo nano hynitron_cst9217.c
sudo nano Makefile
```

> 编译

```
make clean
make
```
成功后会生成 hynitron_cst9217.ko


## 2. 安装驱动
# 方法一：临时加载（测试用）

```
sudo insmod ./hynitron_cst9217.ko
```

# 方法二：安装到系统（推荐）

```
make install
sudo modprobe hynitron_cst9217

```

开机自动加载（可选）：

```
echo "hynitron_cst9217" | sudo tee -a /etc/modules
```

## 4. 设备树配置（DTS）

### 4.1 编译 overlay

```
sudo nano cst9217-touch.dts
```

```
dtc -@ -I dts -O dtb -o cst9217-touch.dtbo cst9217-touch.dts
sudo cp cst9217-touch.dtbo /boot/firmware/overlays/
```

### 4.2 启用 overlay编辑 /boot/firmware/config.txt，添加：

```
dtoverlay=cst9217-touch
```

然后重启：

```
sudo reboot
```

## 5. 单独测试触摸
### 5.1 确认驱动已加载

```
lsmod | grep cst9217
dmesg | grep -i cst9217

```

期望看到类似信息：

```
CST9217 touchscreen registered (addr 0x5a)
using polling mode (20 ms)
# 或
using interrupt mode (IRQ xx)
```

5.2 确认 I2C 设备存在

```
# Pi5 DSI1 常见总线号为 10 或 11，请根据实际情况修改
sudo i2cdetect -y 10
```

5.3 查看输入设备

```
cat /proc/bus/input/devices
```

找到：

```
N: Name="Hynitron CST9217 Touchscreen"
H: Handlers=eventX
```
记下 eventX（例如 event2）。

5.4 使用 evtest 测试（推荐）

```
sudo apt install evtest
sudo evtest
```

选择对应的触摸设备编号，然后用手指点击屏幕。正常时会持续输出坐标：

```
Event: time ..., type 3 (EV_ABS), code 53 (ABS_MT_POSITION_X), value 230
Event: time ..., type 3 (EV_ABS), code 54 (ABS_MT_POSITION_Y), value 180
Event: time ..., type 1 (EV_KEY), code 330 (BTN_TOUCH), value 1
```

卸载驱动

```
sudo rmmod hynitron_cst9217
# 或
make uninstall

```