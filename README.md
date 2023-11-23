# ESP32-C3-DevKitM-1 v1.0 Sesame5 開關控制範例

這個專案展示了如何使用 ESP32-C3-DevKitM-1 v1.0 微控制器來註冊和控制 Sesame5 智能鎖。本範例使用 ESP-IDF 開發框架，透過 BLE 技術自動尋找、連接並註冊附近的 Sesame5 設備。當 ESP32-C3-DevKitM-1 v1.0 偵測到 Sesame5 達到開鎖位置時，會發出指令自動上鎖。

## 多語言版本
- [English version](README_EN.md)
- [日本語版](README_JP.md)

## 前提條件
您需要先安裝 ESP-IDF，可以透過 ESP-IDF 的 `install.sh` 腳本來安裝必要的工具鏈和依賴。

## 安裝與環境設定
1. 請確保您已經通過 ESP-IDF 的 `install.sh` 安裝了工具鏈。
2. 開啟終端機，導航到 ESP-IDF 的路徑，並執行 `export.sh` 加入環境變數。
3. 將ESP32-C3-DevKitM-1 v1.0透過USB連接到您的電腦。
4. 回到專案資料夾，運行 `idf.py flash` 進行編譯和燒錄。

## 使用方法
燒錄並重啟 ESP32-C3-DevKitM-1 v1.0 後，它會自動搜尋附近的未註冊的Sesame設備。在連接並註冊後，ESP32-C3-DevKitM-1 v1.0 會監聽 Sesame5 的狀態，並在適當的時機發送上鎖指令。

## 特色與功能
- **自動設備探索**：自動搜尋和連接附近的 Sesame5 智能鎖。
- **自動上鎖**：當 Sesame5 達到預設開鎖位置時，ESP32-C3-DevKitM-1 v1.0 會發出上鎖指令。

## 原始碼參考
這個範例改自 ESP-IDF 內的 nimble BLE Central Example。

## 額外資源
- 請訪問 [CANDY HOUSE 官方網站](https://jp.candyhouse.co/) 了解更多關於 Sesame5 智能鎖的信息。

## 許可證
本專案採用 MIT 許可證。詳情請參見 `LICENSE` 文件。

## 致謝
感謝您對此專案的興趣，以及 CANDY HOUSE 公司對於開放資源的支持。
