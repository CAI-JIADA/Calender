# OAuth 認證問題排除指南

## 🔴 錯誤 400: redirect_uri_mismatch - Google Calendar

### 問題描述

當您嘗試登入 Google Calendar 時，看到以下錯誤訊息：

```
已封鎖存取權：這個應用程式的要求無效
發生錯誤 400： redirect_uri_mismatch
```

### 問題原因

這個錯誤表示 Google Cloud Console 中註冊的重新導向 URI 與應用程式實際使用的 URI 不匹配。

本應用程式使用的重新導向 URI 為：**`http://localhost:8080/`**

### ✅ 解決方案：更新 Google Cloud Console 設定

#### 步驟 1：前往 Google Cloud Console

1. 開啟瀏覽器，前往 [Google Cloud Console](https://console.cloud.google.com/)
2. 使用您的 Google 帳號登入（例如：b0970133883@gmail.com）

#### 步驟 2：選擇您的專案

1. 在頂部選單欄點選專案選擇器
2. 選擇您之前建立的專案（例如：「Calendar Integration」）

#### 步驟 3：前往憑證設定

1. 在左側選單中，點選「**API 和服務**」
2. 點選「**憑證**」

#### 步驟 4：編輯 OAuth 2.0 用戶端 ID

1. 在「OAuth 2.0 用戶端 ID」區段，找到您的桌面應用程式憑證
2. 點選憑證名稱或右側的編輯圖示（鉛筆符號）

#### 步驟 5：新增授權的重新導向 URI

在「授權的重新導向 URI」區段：

1. **檢查是否已存在以下 URI：**
   - `http://localhost:8080/`（**必須包含結尾的斜線 /**）

2. **如果不存在，請新增：**
   - 點選「+ 新增 URI」
   - 輸入：`http://localhost:8080/`
   - **重要：必須完整輸入，包括結尾的斜線 `/`**

3. **如果存在但格式不正確（例如 `http://localhost:8080` 沒有斜線）：**
   - 刪除現有的錯誤 URI
   - 新增正確的 URI：`http://localhost:8080/`

4. 點選「**儲存**」

#### 步驟 6：等待設定生效

- Google 的設定更新通常會立即生效
- 但有時可能需要等待 1-2 分鐘

#### 步驟 7：重新測試應用程式

1. 關閉應用程式（如果正在執行）
2. 重新啟動應用程式
3. 點選「連接 Google Calendar」
4. 瀏覽器會開啟授權頁面
5. 選擇您的 Google 帳號並授權
6. 應該可以成功完成認證

### 📋 檢查清單

在完成上述步驟後，請確認：

- [ ] Google Cloud Console 中的重新導向 URI 設定為 `http://localhost:8080/`（包含結尾斜線）
- [ ] 已點選「儲存」按鈕
- [ ] 重新啟動應用程式
- [ ] 埠 8080 沒有被其他程式佔用
- [ ] 防火牆允許 localhost 連線

### 🔍 進階除錯

#### 如何確認目前使用的重新導向 URI

應用程式會在主控台輸出除錯訊息。您可以檢查應用程式是否正確設定了重新導向 URI：

```
開始 Google Calendar OAuth 2.0 認證流程...
瀏覽器將會開啟授權頁面
```

#### 檢查埠是否可用

**Windows (PowerShell):**
```powershell
netstat -ano | findstr :8080
```

**Linux/macOS:**
```bash
lsof -i :8080
# 或
netstat -an | grep 8080
```

如果埠被佔用，請關閉佔用該埠的程式，或等待該埠釋放。

#### 手動測試重新導向 URI

您可以在瀏覽器中開啟以下 URL 測試本地伺服器是否正常：

```
http://localhost:8080/
```

應該會看到「連線失敗」或空白頁面（這是正常的，因為只有在 OAuth 流程中伺服器才會啟動）。

### 🔧 其他可能的解決方案

#### 方案 1：重新建立 OAuth 憑證

如果上述方法無效，您可以嘗試重新建立 OAuth 憑證：

1. 在 Google Cloud Console 的憑證頁面
2. 刪除現有的 OAuth 2.0 用戶端 ID
3. 建立新的 OAuth 2.0 用戶端 ID
   - 類型：桌面應用程式
   - 名稱：Calendar Integration Desktop
4. 在「授權的重新導向 URI」新增：`http://localhost:8080/`
5. 複製新的 Client ID 和 Client Secret
6. 更新環境變數或應用程式設定

#### 方案 2：檢查 OAuth 同意畫面設定

1. 前往「API 和服務」→「OAuth 同意畫面」
2. 確認您的 Google 帳號已新增為測試使用者
3. 確認應用程式狀態為「測試中」或「已發布」

#### 方案 3：清除快取並重試

某些情況下，瀏覽器快取可能會導致問題：

1. 清除瀏覽器快取和 Cookie
2. 使用無痕/私密瀏覽模式測試
3. 重新進行 OAuth 認證流程

---

## 🔵 錯誤: invalid_request (redirect_uri is not valid) - Microsoft Outlook

### 問題描述

當您嘗試登入 Microsoft Outlook 時，看到以下錯誤訊息：

```
invalid_request: The provided value for the input parameter 'redirect_uri' is not valid
```

### 問題原因

這個錯誤表示 Azure AD 應用程式註冊中的重新導向 URI 與應用程式實際使用的 URI 不匹配。

本應用程式使用的重新導向 URI 為：**`http://localhost:8081/`**

### ✅ 解決方案：更新 Azure AD 設定

#### 步驟 1：前往 Azure Portal

1. 開啟瀏覽器，前往 [Azure Portal](https://portal.azure.com/)
2. 使用您的 Microsoft 帳號登入

#### 步驟 2：開啟應用程式註冊

1. 在搜尋列中輸入「Azure Active Directory」或「Microsoft Entra ID」
2. 選擇「應用程式註冊」
3. 找到並點選您的應用程式（例如：「Calendar Integration」）

#### 步驟 3：編輯驗證設定

1. 在左側選單中，點選「**驗證**」(Authentication)

#### 步驟 4：更新重新導向 URI

在「平台組態」或「重新導向 URI」區段：

1. **檢查是否已存在以下 URI：**
   - `http://localhost:8081/`（**必須包含結尾的斜線 /**）

2. **如果不存在，請新增：**
   - 點選「+ 新增 URI」或「+ 新增平台」
   - 選擇「行動和傳統型應用程式」
   - 輸入自訂重新導向 URI：`http://localhost:8081/`
   - **重要：必須完整輸入，包括結尾的斜線 `/`**

3. **如果存在但格式不正確（例如 `http://localhost:8081` 沒有斜線）：**
   - 刪除現有的錯誤 URI
   - 新增正確的 URI：`http://localhost:8081/`

4. 點選「**儲存**」

#### 步驟 5：重新測試應用程式

1. 關閉應用程式（如果正在執行）
2. 重新啟動應用程式
3. 點選「連接 Microsoft Outlook」
4. 瀏覽器會開啟授權頁面
5. 選擇您的 Microsoft 帳號並授權
6. 應該可以成功完成認證

### 📋 檢查清單

在完成上述步驟後，請確認：

- [ ] Azure AD 中的重新導向 URI 設定為 `http://localhost:8081/`（包含結尾斜線）
- [ ] 已點選「儲存」按鈕
- [ ] 重新啟動應用程式
- [ ] 埠 8081 沒有被其他程式佔用
- [ ] 防火牆允許 localhost 連線

---

## 📞 仍然無法解決？

如果您按照上述步驟操作後仍然遇到問題，請提供以下資訊以便進一步協助：

1. 完整的錯誤訊息截圖
2. 您使用的作業系統（Windows/Linux/macOS）
3. 應用程式的除錯輸出（主控台訊息）
4. 您在 Google Cloud Console / Azure Portal 的設定截圖（請移除敏感資訊）

---

## 📚 相關文件

- [TESTING.md](./TESTING.md) - 完整的測試指南
- [README.md](./README.md) - 專案說明
- [Google OAuth 2.0 文件](https://developers.google.com/identity/protocols/oauth2)
- [Microsoft 身份識別平台文件](https://learn.microsoft.com/zh-tw/azure/active-directory/develop/)

---

**最後更新**: 2026 年 1 月
