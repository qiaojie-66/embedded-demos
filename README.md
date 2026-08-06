# Embedded Demos

个人嵌入式练手项目集合 - **Monorepo** 单仓库管理。

## 目录结构

```
embedded-demos/
├── esp32/             ESP-IDF 项目 (ESP32-S3)
├── stm32/             STM32 HAL 项目
├── lvgl/              LVGL 图形界面
└── qt_widget/         Qt Widget 桌面应用
```

## 项目列表

### esp32
- `demo03_ws2812/` - GPIO48 驱动 WS2812 灯珠

### stm32
- (待添加)

### lvgl
- (待添加)

### qt_widget
- (待添加)

## 工具链

| 平台 | 工具 |
|------|------|
| ESP32 | ESP-IDF v5.x |
| STM32 | STM32CubeIDE / Keil |
| LVGL | SquareLine Studio / 手写 |
| Qt | Qt 5.15+ |

## 提交规范

按子目录独立开发，统一提交到 `main` 分支。