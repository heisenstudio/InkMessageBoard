# Hardware

这里放的是 InkMessageBoard 的硬件资料。当前是学习原型资料，不是成熟量产包。

## 板子

| 板子 | 源工程 | 说明 |
| --- | --- | --- |
| ESP32-S3 主控板 | `source/altium/esp32_s3_board/` | AD 工程 |
| 电源板 | `source/altium/power_board/` | AD 工程 |
| 2.66 寸墨水屏板 | `source/lceda/ink_board/` | 嘉立创 EDA 工程 |

## 目录

```text
hardware/
├── source/       # 可编辑源工程
├── schematic/    # 原理图 PDF
├── pcb/          # PCB 正反面图片
├── gerber/       # Gerber 压缩包
├── bom/          # BOM 表
└── production/   # 贴片坐标和装配说明
```

## 开源硬件说明

硬件开源不只是放几张图。这个目录里保留了可编辑源工程，也另外导出了原理图、PCB 图片、Gerber、BOM 和贴片坐标，方便别人学习、检查或继续修改。

发布或打板前建议再做一次人工核对：

- 原理图是否对应当前固件引脚
- PCB 是否是实际测试过的版本
- BOM 封装和值是否正确
- Gerber 和贴片坐标是否来自同一版工程
- 电源输入、屏幕排线方向、触摸按键是否和实物一致
