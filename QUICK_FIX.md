# 🚨 OAuth 錯誤快速修復指南

## Google Calendar 錯誤 400: redirect_uri_mismatch

### 問題
```
已封鎖存取權：這個應用程式的要求無效
發生錯誤 400： redirect_uri_mismatch
```

### 3 步驟快速修復 ⚡

1. **前往** [Google Cloud Console 憑證頁面](https://console.cloud.google.com/apis/credentials)

2. **編輯您的 OAuth 2.0 用戶端 ID**，在「授權的重新導向 URI」新增：
   ```
   http://localhost:8080/
   ```
   ⚠️ **重要**：必須包含結尾的斜線 `/`

3. **點選「儲存」** → 重新啟動應用程式 → 完成！

---

## Microsoft Outlook 錯誤: invalid_request

### 問題
```
invalid_request: The provided value for the input parameter 'redirect_uri' is not valid
```

### 3 步驟快速修復 ⚡

1. **前往** [Azure Portal](https://portal.azure.com/)

2. **開啟應用程式註冊** → 選擇「驗證」→ 新增重新導向 URI：
   ```
   http://localhost:8081/
   ```
   ⚠️ **重要**：必須包含結尾的斜線 `/`

3. **點選「儲存」** → 重新啟動應用程式 → 完成！

---

## 📖 需要更詳細的說明？

查看完整的 [OAuth 認證問題排除指南 (OAUTH_TROUBLESHOOTING.md)](./OAUTH_TROUBLESHOOTING.md)

---

## ✅ 檢查清單

### Google Calendar
- [ ] Google Cloud Console 中新增了 `http://localhost:8080/`
- [ ] 包含了結尾斜線 `/`
- [ ] 已點選「儲存」
- [ ] 已重新啟動應用程式

### Microsoft Outlook
- [ ] Azure Portal 中新增了 `http://localhost:8081/`
- [ ] 包含了結尾斜線 `/`
- [ ] 已點選「儲存」
- [ ] 已重新啟動應用程式

---

**提示**：如果問題仍然存在，請檢查：
- 埠 8080 (Google) 或 8081 (Outlook) 是否被其他程式佔用
- 防火牆是否允許 localhost 連線
- 是否等待了 1-2 分鐘讓設定生效
