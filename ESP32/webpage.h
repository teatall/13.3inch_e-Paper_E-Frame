#pragma once
#include <pgmspace.h>

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>EPD Frame Manager</title>
    <link rel="stylesheet" href="/cropper.min.css">
    <link rel="stylesheet" href="/styles.css">
    <script src="/cropper.min.js"></script>
  </head>
  <body>
    <div class="container" id="appContainer">
      <div class="header">
        <div class="tabs">
          <button class="tab-btn active" id="tab-upload" onclick="switchTab('upload')" data-i18n="tabUpload">Upload</button>
          <button class="tab-btn" id="tab-gallery" onclick="switchTab('gallery')" data-i18n="tabGallery">Gallery</button>
          <button class="tab-btn" id="tab-settings" onclick="switchTab('settings')" data-i18n="tabSettings">Settings</button>
        </div>
        <div class="header-actions">
          <button class="lang-btn" onclick="toggleLanguage()">🌐</button>
          <button class="exit-btn" onclick="triggerExit()">
            <svg viewBox="0 0 24 24" width="16" height="16" stroke="currentColor" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round" style="margin-right: 4px; vertical-align: middle;">
              <path d="M18.36 4.64a9 9 0 1 1-12.73 0"></path>
              <line x1="12" y1="2" x2="12" y2="12"></line>
            </svg>
            <span data-i18n="btnExit">Exit</span>
          </button>
        </div>
      </div>
      <div id="upload" class="content active">
        <div class="orientation-selector">
          <button class="orient-btn active" id="btnP" onclick="setMode('P')" data-i18n="optPortrait">Portrait</button>
          <button class="orient-btn" id="btnL" onclick="setMode('L')" data-i18n="optLandscape">Landscape</button>
        </div>
        <input type="file" id="fileInput" accept="image/*" style="display:none">
        <div class="upload-frame-wrapper empty-state" id="wrapper">
          <div class="content-area" id="hint">
            <svg viewBox="0 0 24 24"><path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z" /></svg>
            <div data-i18n="uploadHint">Click or Drag & Drop Image Here</div>
          </div>
          <img id="imageToCrop">
        </div>
        <div class="action-group" id="actionGroup">
          <button class="btn-secondary" id="btnClear" onclick="resetImage()" data-i18n="btnClear">Clear</button>
          <button class="btn" id="btnSave" onclick="processAndUpload()">
            <div class="upload-progress" id="saveProgress"></div>
            <span id="btnText" data-i18n="btnSave">Save</span>
          </button>
        </div>
      </div>
      <div id="gallery" class="content">
        <h3 data-i18n="galleryHint" style="font-size: 1.1rem; margin-top: 0;">Photos on SD Card</h3>
        <div id="imageList"><p style="color: #6c757d; font-size: 0.9rem;">Loading...</p></div>
      </div>
      <div id="settings" class="content">
        <div class="stat-group">
          <h3 data-i18n="sysStatus" style="font-size: 1.1rem; margin-top: 0;">System Status</h3>
          <div class="stat-row"><span class="stat-label" data-i18n="lblVersion">Version</span><span id="versionVal">...</span></div>
          <div class="stat-row"><span class="stat-label" data-i18n="lblBattery">Battery</span><span id="battVal">...</span></div>
          <div class="stat-row"><span class="stat-label" data-i18n="lblStorage">Storage Usage</span><span id="storageVal">...</span></div>
          <div class="stat-row"><span class="stat-label" data-i18n="lblWifi">WiFi Signal</span><span id="wifiSignal">...</span></div>
        </div>
        <div class="stat-group">
          <h3 data-i18n="playSettings" style="font-size: 1.1rem; margin-top: 0;">Playback Settings</h3>
          <span class="stat-label" data-i18n="lblInterval">Switch Interval</span>
          <div class="slider-container">
            <input type="range" id="timeSlider" min="0" max="7" value="2" oninput="updateTimeText()">
            <div id="timeValueDisplay" style="font-weight: 700; min-width: 80px; text-align: right;">1 Hour</div>
          </div>
        </div>
        <button class="btn" id="btnSaveSet" onclick="saveSettings()" style="margin-top: 1rem;" data-i18n="btnSaveSettings">Save Settings</button>
      </div>
    </div>
    <div class="goodbye-screen" id="goodbyeScreen">
      <h2 data-i18n="goodbyeTitle">Management Mode Exited</h2>
      <p data-i18n="goodbyeText">The e-Paper is refreshing. You can safely close this page.</p>
    </div>
    <script>
      const dict = {
        "en": { tabUpload: "Upload", tabGallery: "Gallery", tabSettings: "Settings", btnExit: "Exit", optLandscape: "Landscape", optPortrait: "Portrait", uploadHint: "Click or Drag & Drop Image Here", btnClear: "Clear", btnSave: "Save", galleryHint: "Photos on SD Card", btnDelete: "Delete", sysStatus: "System Status", lblVersion: "Firmware Version", lblBattery: "Battery", lblStorage: "Storage Usage", lblWifi: "WiFi Signal", playSettings: "Playback Settings", lblInterval: "Switch Interval", btnSaveSettings: "Save Settings", goodbyeTitle: "Exited", goodbyeText: "Refreshing screen. You can close this page.", timeOpts: ["10 Min", "30 Min", "1 Hour", "2 Hours", "3 Hours", "6 Hours", "12 Hours", "24 Hours"] },
        "zh": { tabUpload: "上传", tabGallery: "照片列表", tabSettings: "设置", btnExit: "退出管理", optLandscape: "横向", optPortrait: "纵向", uploadHint: "点击或拖拽图片至此", btnClear: "清除", btnSave: "保存", galleryHint: "SD 卡内的照片", btnDelete: "删除", sysStatus: "系统状态", lblVersion: "系统版本号", lblBattery: "电池电量", lblStorage: "存储状况", lblWifi: "信号强度 (dBm)", playSettings: "播放设置", lblInterval: "换图间隔", btnSaveSettings: "保存设置", goodbyeTitle: "已退出管理模式", goodbyeText: "相框正在刷新屏幕。您可以关闭此页面。", timeOpts: ["10 分钟", "30 分钟", "1 小时", "2 小时", "3 小时", "6 小时", "12 小时", "24 小时"] }
      };
      
      let currentLang = "en";
      function initLang() { if ((navigator.language || '').toLowerCase().includes('zh')) currentLang = "zh"; applyLang(); }
      function toggleLanguage() { currentLang = currentLang === "en" ? "zh" : "en"; applyLang(); }
      function applyLang() { document.querySelectorAll('[data-i18n]').forEach(el => { const key = el.getAttribute('data-i18n'); el.innerText = dict[currentLang][key] || key; }); updateTimeText(); }
      
      function switchTab(id) {
        document.querySelectorAll('.content').forEach(c => c.classList.remove('active'));
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        document.getElementById(id).classList.add('active');
        document.getElementById('tab-'+id).classList.add('active');
        if(id === 'gallery') loadGallery();
        if(id === 'settings') loadSysInfo();
      }
      
      function updateTimeText() { const slider = document.getElementById('timeSlider'); document.getElementById('timeValueDisplay').innerText = dict[currentLang].timeOpts[slider.value]; }
      
      // ==========================================
      // 🚀 极速加载与前端时间倒序
      // ==========================================
      function loadGallery() {
        const list = document.getElementById('imageList');
        list.innerHTML = '<p style="color: #6c757d; font-size: 0.9rem;">Loading files...</p>';
        fetch('/api/list')
          .then(res => res.json())
          .then(data => {
            if(data.length === 0) { list.innerHTML = '<p style="color: #6c757d;">No BMP files found.</p>'; return; }
            list.innerHTML = '';
            data.sort((a, b) => b.time - a.time);
            data.forEach(file => {
              const row = document.createElement('div');
              row.className = 'img-row';
              let thumbHTML = '';
              if (file.hasThumb) {
                  let thumbName = file.name.replace('.bmp', '.jpg').replace('.BMP', '.jpg');
                  thumbHTML = `<img src="/api/thumb?name=${encodeURIComponent(thumbName)}" class="real-thumb" loading="lazy">`;
              } else {
                  thumbHTML = `<div class="thumb-icon">🖼️</div><div class="thumb-label">BMP</div>`;
              }
              row.innerHTML = `
                <div class="img-info">
                  <div class="thumb-preview" onclick="window.open('/api/img?name=${encodeURIComponent(file.name)}')">
                     ${thumbHTML}
                  </div>
                  <div class="img-meta">
                    <div>${file.name}</div>
                    <div style="font-size: 0.8rem; color: #6c757d; margin-top: 4px;">${(file.size/1024/1024).toFixed(2)} MB</div>
                  </div>
                </div>
                <button class="del-btn" onclick="deleteFile('${file.name}')">${dict[currentLang].btnDelete}</button>
              `;
              list.appendChild(row);
            });
          });
      }
      
      function deleteFile(name) {
        if(!confirm(currentLang==='zh'?'确定要彻底删除这张照片吗？(含缩略图)':'Delete this photo and its thumbnail?')) return;
        const params = new URLSearchParams(); params.append('filename', name);
        fetch('/api/delete', { method: 'POST', body: params }).then(() => loadGallery());
      }
      
      function loadSysInfo() {
        fetch('/api/sysinfo')
          .then(res => res.json())
          .then(data => {
            document.getElementById('versionVal').innerText = data.version;
            document.getElementById('battVal').innerText = `${data.batt_v}V (${data.batt_pct}%)`;
            document.getElementById('storageVal').innerText = `${data.sd_used} / ${data.sd_total} GB`;
            document.getElementById('wifiSignal').innerText = `${data.wifi_rssi} dBm`; 
            document.getElementById('timeSlider').value = data.interval_idx;
            updateTimeText();
          });
      }
      
      function saveSettings() {
        const btn = document.getElementById('btnSaveSet');
        btn.innerText = '...';
        const params = new URLSearchParams(); params.append('interval', document.getElementById('timeSlider').value);
        fetch('/api/settings', { method: 'POST', body: params }).then(() => { btn.innerText = 'OK!'; setTimeout(()=>applyLang(), 2000); });
      }
      
      function triggerExit() { 
        fetch('/api/exit', { method: 'POST' })
          .then(() => { document.getElementById('appContainer').style.display = 'none'; document.getElementById('goodbyeScreen').style.display = 'block'; })
          .catch(()=> { document.getElementById('appContainer').style.display = 'none'; document.getElementById('goodbyeScreen').style.display = 'block'; }); 
      }
      
      const E6_PALETTE = [[0,0,0],[255,255,255],[255,255,0],[255,0,0],[0,0,255],[0,255,0]];
      let cropper = null, mode = 'P';
      const wrapper = document.getElementById('wrapper');
      const fileInput = document.getElementById('fileInput');
      const hintArea = document.getElementById('hint');
      
      function setMode(m) { 
        mode = m; 
        document.getElementById('btnL').classList.toggle('active', m === 'L');
        document.getElementById('btnP').classList.toggle('active', m === 'P'); 
        if (cropper) cropper.setAspectRatio(m === 'L' ? 4 / 3 : 3 / 4); 
      }
      
      // 🚀 修复 Bug：防止事件冒泡导致裁剪时弹窗
      hintArea.onclick = () => { fileInput.click(); };
      wrapper.onclick = (e) => { if (cropper) return; fileInput.click(); };
      fileInput.onchange = (e) => { if (e.target.files[0]) handleFile(e.target.files[0]); };
      
      function handleFile(file) { 
        const reader = new FileReader();
        reader.onload = (e) => { 
          hintArea.style.display = 'none'; 
          wrapper.classList.remove('empty-state'); 
          document.getElementById('actionGroup').style.display = 'flex';
          const img = document.getElementById('imageToCrop');
          img.src = e.target.result;
          if (cropper) cropper.destroy();
          cropper = new Cropper(img, { 
              aspectRatio: mode === 'L' ? 4 / 3 : 3 / 4, 
              viewMode: 1,
              autoCropArea: 1 // 🚀 修复 Bug：裁剪框默认铺满全屏
          });
        }; 
        reader.readAsDataURL(file); 
      }
      
      function resetImage() {
        if (cropper) { cropper.destroy(); cropper = null; }
        document.getElementById('fileInput').value = '';
        document.getElementById('imageToCrop').src = '';
        document.getElementById('actionGroup').style.display = 'none';
        hintArea.style.display = 'flex';
        wrapper.classList.add('empty-state');
      }
      
      function syncDeviceTime() {
        const now = new Date();
        const params = new URLSearchParams({
            y: now.getFullYear(),
            m: now.getMonth() + 1, // JS月份是 0-11
            d: now.getDate(),
            h: now.getHours(),
            min: now.getMinutes(),
            s: now.getSeconds()
        });
        // 悄悄发送，不需要处理 response，不阻塞 UI
        fetch('/api/time?' + params.toString(), { method: 'POST' }).catch(()=>{});
      }
      
      // ==========================================
      // 真实多文件流式上传逻辑
      // ==========================================
      let savingInterval = null;
      
      function setUploadingState(isUploading) {
        const btnSave = document.getElementById('btnSave');
        const btnClear = document.getElementById('btnClear');
        if (isUploading) {
          btnSave.disabled = true;
          btnClear.disabled = true;
          btnClear.style.opacity = "0.5"; 
          btnClear.style.cursor = "not-allowed";
          btnSave.style.cursor = "not-allowed";
          let dotCount = 1;
          btnSave.innerText = "Saving.";
          savingInterval = setInterval(() => {
            dotCount = (dotCount % 3) + 1;
            btnSave.innerText = "Saving" + ".".repeat(dotCount);
          }, 400);
        } else {
          clearInterval(savingInterval);
          btnSave.disabled = false;
          btnClear.disabled = false;
          btnClear.style.opacity = "1";
          btnSave.style.cursor = "pointer";
          btnSave.innerText = dict[currentLang].btnSave; 
        }
      }
      
      // 带进度条的上传
      async function processAndUpload() {
        const btn = document.getElementById('btnSave');
        const btnText = document.getElementById('btnText');
        const progress = document.getElementById('saveProgress');
        
        btn.disabled = true;
        progress.style.width = '0%';
        btnText.innerText = (currentLang === 'zh' ? '正在保存...' : 'Saving...');
        
        try {
          const w = mode === 'L' ? 1600 : 1200, h = mode === 'L' ? 1200 : 1600;
          const cvs = cropper.getCroppedCanvas({ width: w, height: h });
          const imgData = cvs.getContext('2d').getImageData(0, 0, w, h);
          applyDither(imgData);
          const bmpBlob = createBMP(w, h, imgData.data);
          
          const thumbBlob = await new Promise(resolve => {
             const tCvs = document.createElement('canvas');
             tCvs.width = mode === 'L' ? 160 : 120;
             tCvs.height = mode === 'L' ? 120 : 160;
             tCvs.getContext('2d').drawImage(cvs, 0, 0, tCvs.width, tCvs.height);
             tCvs.toBlob(resolve, 'image/jpeg', 0.6); 
          });
          
          const fileName = `E${Date.now()}`;
          const formData = new FormData();
          formData.append("file1", bmpBlob, `${fileName}.bmp`);
          formData.append("file2", thumbBlob, `${fileName}.jpg`);
          
          // 使用 XMLHttpRequest 以跟踪进度
          const xhr = new XMLHttpRequest();
          await new Promise((resolve, reject) => {
            xhr.upload.onprogress = (e) => {
              if (e.lengthComputable) {
                const percent = (e.loaded / e.total) * 100;
                progress.style.width = percent + '%';
              }
            };
            xhr.onload = () => (xhr.status >= 200 && xhr.status < 300) ? resolve() : reject();
            xhr.onerror = () => reject();
            xhr.open('POST', '/api/upload');
            xhr.send(formData);
          });
          
          btnText.innerText = (currentLang === 'zh' ? '成功!' : 'Success!');
          await new Promise(r => setTimeout(r, 1000));
          switchTab('gallery');
          
        } catch (error) {
          alert("Upload failed: " + error.message);
        } finally {
          btn.disabled = false;
          progress.style.width = '0%';
          btnText.innerText = dict[currentLang].btnSave;
          resetImage(); 
        }
      }
      
      function applyDither(imgData) {
        const { data, width, height } = imgData;
        const buffer = new Float32Array(data);
        for (let y = 0; y < height; y++) {
          for (let x = 0; x < width; x++) {
            const i = (y * width + x) * 4;
            const r = buffer[i], g = buffer[i + 1], b = buffer[i + 2];
            let best = E6_PALETTE[0], minD = Infinity;
            for (const p of E6_PALETTE) {
              const d = (r - p[0]) ** 2 + (g - p[1]) ** 2 + (b - p[2]) ** 2;
              if (d < minD) { minD = d; best = p; }
            }
            data[i] = best[0]; data[i + 1] = best[1]; data[i + 2] = best[2];
            const er = r - best[0], eg = g - best[1], eb = b - best[2];
            const diff = (nx, ny, w) => { 
              if (nx >= 0 && nx < width && ny < height) { 
                const idx = (ny * width + nx) * 4;
                buffer[idx] += er * w; buffer[idx + 1] += eg * w; buffer[idx + 2] += eb * w;
              } 
            };
            diff(x + 1, y, 7 / 16); diff(x - 1, y + 1, 3 / 16);
            diff(x, y + 1, 5 / 16); diff(x + 1, y + 1, 1 / 16);
          }
        }
      }
      
      function createBMP(w, h, data) {
        const rowSize = Math.floor((w * 3 + 3) / 4) * 4;
        const size = 54 + rowSize * h;
        const buffer = new ArrayBuffer(size);
        const v = new DataView(buffer);
        v.setUint8(0, 0x42);
        v.setUint8(1, 0x4D); v.setUint32(2, size, true); v.setUint32(10, 54, true); v.setUint32(14, 40, true);
        v.setInt32(18, w, true); v.setInt32(22, h, true); v.setUint16(26, 1, true);
        v.setUint16(28, 24, true);
        let p = 54;
        for (let y = h - 1; y >= 0; y--) {
          for (let x = 0; x < w; x++) {
            const i = (y * w + x) * 4;
            v.setUint8(p++, data[i + 2]); v.setUint8(p++, data[i + 1]); v.setUint8(p++, data[i]);
          }
          for (let pad = 0; pad < (rowSize - w * 3); pad++) v.setUint8(p++, 0);
        }
        return new Blob([buffer], { type: "image/bmp" });
      }
      
      window.onload = () => { initLang(); syncDeviceTime(); loadSysInfo(); };
    </script>
  </body>
</html>
)=====";