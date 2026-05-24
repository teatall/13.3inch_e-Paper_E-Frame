#pragma once
#include <pgmspace.h>

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
   <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>EPD Frame Manager</title>
      <link rel="stylesheet" href="cropper.min.css">
      <link rel="stylesheet" href="styles.css">
      <script src="cropper.min.js"></script>
   </head>
   <body>
      <svg xmlns="http://www.w3.org/2000/svg" style="display: none;">
         <symbol id="power" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round"><path d="M18.36 4.64a9 9 0 1 1-12.73 0"></path><line x1="12" y1="2" x2="12" y2="12"></line></symbol>
         <symbol id="vars" viewBox="0 0 24 24" stroke="currentColor" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round"><path d="M3 8h12m0 0a3 3 0 1 0 6 0 3 3 0 1 0-6 0zm-6 8h12M9 16a3 3 0 1 1-6 0 3 3 0 1 1 6 0z" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></symbol>
         <symbol id="earth" viewBox="0 0 24 24" stroke="currentColor" fill="none" stroke-width="2" stroke-linejoin="round" stroke-linecap="round" ><path d="M22.812 12A10.812 10.812 0 0 1 12 22.812M22.812 12A10.812 10.812 0 0 0 12 1.188M22.812 12c0 1.991-4.84 3.604-10.812 3.604S1.188 13.991 1.188 12m21.625 0c0-1.991-4.84-3.604-10.812-3.604S1.188 10.009 1.188 12M12 22.812A10.812 10.812 0 0 1 1.188 12M12 22.812c-1.991 0-3.604-4.84-3.604-10.812S10.009 1.188 12 1.188m0 21.625c1.991 0 3.604-4.84 3.604-10.812S13.991 1.188 12 1.188M1.188 12A10.812 10.812 0 0 1 12 1.188"/></symbol>
      </svg>
      <div class="container" id="appContainer">
         <div class="header">
            <div class="tabs">
               <button class="tab-btn active" id="tab-upload" onclick="switchTab('upload')" data-i18n="tabUpload">Upload</button>
               <button class="tab-btn" id="tab-gallery" onclick="switchTab('gallery')" data-i18n="tabGallery">Gallery</button>
               <button class="tab-btn" id="tab-settings" onclick="switchTab('settings')" data-i18n="tabSettings">Settings</button>
            </div>
            <div class="header-actions">
               <button class="btn btn-secondary btn-sm" onclick="toggleLanguage()"><svg style="vertical-align:middle;" width="16" height="16"><use xlink:href="#earth"></use></svg></button>
               <button class="exit-btn btn btn-danger-outline btn-sm" onclick="triggerExit()">
                  <svg width="16" height="16" style="vertical-align:middle;"><use xlink:href="#power"></use></svg>
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
                  <svg viewBox="0 0 24 24">
                     <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z" />
                  </svg>
                  <div data-i18n="uploadHint">Click or Drag & Drop Image Here</div>
               </div>
               <img id="imageToCrop">
            </div>
            <div class="action-group" id="actionGroup">
               <button class="btn btn-secondary" id="btnClear" onclick="resetImage()" data-i18n="btnClear">Clear</button>
               <div class="btn-group btn-main">
                  <button class="btn btn-primary btn-main" id="btnSave" onclick="processAndUpload()">
                     <span id="btnText" data-i18n="btnSave">Save</span>
                  </button>
                  <div class="upload-progress" id="saveProgress"></div>
                  <button class="btn btn-primary btn-sm" onclick="showVarGroup()">
                     <svg width="16" height="16" style="vertical-align:middle;"><use xlink:href="#vars"></use></svg>
                  </button>
               </div>
            </div>
            <div id="varGroup" class="image-var-group hidden">
               <div class="slider-container">
                  <label for="gamma" data-i18n="varGamma">Gamma</label>
                  <input type="range" id="gamma" min="0" max="1" step="0.01" value="0.82" oninput="applyImageVar('gamma')">
                  <div id="gammaDisplay" class="slide-display">0.82</div>
               </div>
               <div class="slider-container">
                  <label for="saturation" data-i18n="varSaturation">Saturation</label>
                  <input type="range" id="saturation" min="0" max="200" step="10" value="100" oninput="applyImageVar('saturation')">
                  <div id="saturationDisplay" class="slide-display">100</div>
               </div>
            </div>
         </div>
         <div id="gallery" class="content">
            <h3 data-i18n="galleryHint" style="font-size: 1.1rem; margin-top: 0;">Photos on SD Card</h3>
            <div id="batchToolbar" style="display: none;">
               <button class="btn btn-xs btn-secondary" id="btnSelectAll" onclick="toggleSelectAll()"></button>
               <button id="btnBatchAction" class="btn btn-xs" onclick="executeBatchAction()" style="display: none; color: #fff;"></button>
            </div>
            <div id="imageList">
               <p style="color: #6c757d; font-size: 0.9rem;">Loading...</p>
            </div>
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
                  <div id="timeValueDisplay" class="slide-display">1 Hour</div>
               </div>
            </div>
            <button class="btn btn-primary" id="btnSaveSet" onclick="saveSettings()" data-i18n="btnSaveSettings">Save Settings</button>
         </div>
      </div>
      <div class="goodbye-screen" id="goodbyeScreen">
         <h2 data-i18n="goodbyeTitle">Management Mode Exited</h2>
         <p data-i18n="goodbyeText">The e-Paper is refreshing. You can safely close this page.</p>
      </div>
      <script>
         const dict = {
           "en": { tabUpload: "Upload", tabGallery: "Gallery", tabSettings: "Settings", btnExit: "Exit", optLandscape: "Landscape", optPortrait: "Portrait", uploadHint: "Click or Drag & Drop Image Here", btnClear: "Clear", btnSave: "Save", varGamma: "Gamma", varSaturation: "Saturation", galleryHint: "Photos in Gallery", btnDelete: "Delete", sysStatus: "System Status", lblVersion: "Firmware Version", lblBattery: "Battery", lblStorage: "Storage Usage", lblWifi: "WiFi Signal", playSettings: "Playback Settings", lblInterval: "Switch Interval", btnSaveSettings: "Save Settings", goodbyeTitle: "Exited", goodbyeText: "Refreshing screen. You can close this page.", charging: "Charging", timeOpts: ["10 Min", "30 Min", "1 Hour", "2 Hours", "3 Hours", "6 Hours", "12 Hours", "24 Hours"] },
           "zh": { tabUpload: "上传", tabGallery: "照片列表", tabSettings: "设置", btnExit: "退出", optLandscape: "横向", optPortrait: "纵向", uploadHint: "点击或拖拽图片至此", btnClear: "清除", btnSave: "保存", varGamma: "明暗度", varSaturation: "饱和度", galleryHint: "已有照片", btnDelete: "删除", sysStatus: "系统状态", lblVersion: "系统版本", lblBattery: "电池电量", lblStorage: "存储状况", lblWifi: "信号强度 (dBm)", playSettings: "播放设置", lblInterval: "换图间隔", btnSaveSettings: "保存设置", goodbyeTitle: "已退出", goodbyeText: "相框正在刷新屏幕。您可以关闭此页面。", charging: "充电中", timeOpts: ["10 分钟", "30 分钟", "1 小时", "2 小时", "3 小时", "6 小时", "12 小时", "24 小时"] }
         };
         
         let currentLang = "en";
         let selectedFiles = new Set(); // 新增：全局记录选中文件
         
         function initLang() { if ((navigator.language || '').toLowerCase().includes('zh')) currentLang = "zh"; applyLang(); }
         function toggleLanguage() { currentLang = currentLang === "en" ? "zh" : "en"; applyLang(); updateBatchToolbar(); }
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
         
         function applyImageVar(id) {
            // 1. 更新显示的数值
            document.getElementById(id + 'Display').innerText = document.getElementById(id).value;

            // 2. 获取两个滑块的当前值
            const gammaVal = parseFloat(document.getElementById('gamma').value);
            const satVal = document.getElementById('saturation').value; // 0-200

            // 3. 构建 CSS 滤镜 (视觉欺骗)
            // Saturation 直接对应 CSS saturate(x%)
            // Gamma (0~1): 值越小画面越亮。0.82 默认时不改变亮度(1.0)，向 0 滑动时提亮，向 1 滑动时变暗。
            const cssBrightness = 1 + (0.82 - gammaVal) * 1.5; 

            // 4. 将滤镜应用到整个预览区域
            const wrapper = document.getElementById('wrapper');
            wrapper.style.filter = `saturate(${satVal}%) brightness(${cssBrightness})`;
         }
         
         // ==========================================
         // 🚀 图库与多选逻辑
         // ==========================================
         function loadGallery() {
           const list = document.getElementById('imageList');
           list.innerHTML = '<p style="color: #6c757d; font-size: 0.9rem;">Loading files...</p>';
           
           // 重置选中状态并更新工具栏
           selectedFiles.clear();
           updateBatchToolbar();
         
           fetch('/api/list')
             .then(res => res.json())
             .then(data => {
               if(data.length === 0) { 
                   list.innerHTML = '<p style="color: #6c757d;">No BMP files found.</p>'; 
                   updateBatchToolbar(); 
                   return; 
               }
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
                     <input class="img-checkbox" type="checkbox" name="gallery" value="${file.name}" onchange="handleSelection(this)">
                     <div class="thumb-preview" onclick="window.open('/api/img?name=${encodeURIComponent(file.name)}')">
                        ${thumbHTML}
                     </div>
                     <div class="img-meta">
                       <div>${file.name}</div>
                       <div style="font-size: 0.8rem; color: #6c757d; margin-top: 4px;">${(file.size/1024/1024).toFixed(2)} MB</div>
                     </div>
                   </div>
                   <button class="btn btn-danger-outline btn-xs" onclick="deleteFile('${file.name}')">${dict[currentLang].btnDelete}</button>
                 `;
                 list.appendChild(row);
               });
               updateBatchToolbar(); // 列表渲染完成后显示工具栏
             });
         }
         
         function handleSelection(cb) {
           if (cb.checked) selectedFiles.add(cb.value);
           else selectedFiles.delete(cb.value);
           updateBatchToolbar();
         }
         
         function toggleSelectAll() {
           const checkboxes = document.querySelectorAll('.img-checkbox');
           if (selectedFiles.size === checkboxes.length && checkboxes.length > 0) {
             checkboxes.forEach(cb => cb.checked = false);
             selectedFiles.clear();
           } else {
             checkboxes.forEach(cb => { cb.checked = true; selectedFiles.add(cb.value); });
           }
           updateBatchToolbar();
         }
         
         function updateBatchToolbar() {
           const tb = document.getElementById('batchToolbar');
           const btnAction = document.getElementById('btnBatchAction');
           const btnSelectAll = document.getElementById('btnSelectAll');
           const total = document.querySelectorAll('.img-checkbox').length;
         
           if (total === 0) { 
             tb.style.display = 'none'; 
             return; 
           }
           
           tb.style.display = 'flex';
           btnSelectAll.innerText = selectedFiles.size === total ? 
             (currentLang === 'zh' ? '取消全选' : 'Deselect All') : 
             (currentLang === 'zh' ? '全选' : 'Select All');
         
           if (selectedFiles.size === 0) {
             btnAction.style.display = 'none';
           } else if (selectedFiles.size === 1) {
             btnAction.style.display = 'block';
             btnAction.innerText = currentLang === 'zh' ? '▶ 播放所选照片' : '▶ Play Selected';
             btnAction.classList.remove('btn-danger');
             btnAction.classList.add('btn-success');
           } else {
             btnAction.style.display = 'block';
             btnAction.innerText = currentLang === 'zh' ? `删除所选 (${selectedFiles.size})` : `Delete Selected (${selectedFiles.size})`;
             btnAction.classList.add('btn-danger');
             btnAction.classList.remove('btn-success');
           }
         }
         
         function executeBatchAction() {
           if (selectedFiles.size === 1) {
             const name = Array.from(selectedFiles)[0];
             fetch('/api/play_now?name=' + encodeURIComponent(name), { method: 'POST' })
               .then(() => {
                 document.getElementById('appContainer').style.display = 'none';
                 document.getElementById('goodbyeScreen').style.display = 'block';
               });
           } else if (selectedFiles.size > 1) {
             if (!confirm(currentLang === 'zh' ? '确定删除选中的照片？' : 'Delete selected photos?')) return;
             const params = new URLSearchParams();
             params.append('filenames', Array.from(selectedFiles).join(','));
             fetch('/api/delete', { method: 'POST', body: params }).then(() => loadGallery());
           }
         }
         
         function deleteFile(name) {
           if(!confirm(currentLang==='zh'?'确定删除照片？':'Delete this photo?')) return;
           const params = new URLSearchParams(); params.append('filename', name);
           fetch('/api/delete', { method: 'POST', body: params }).then(() => loadGallery());
         }
         
         // ==========================================
         // 系统功能交互逻辑
         // ==========================================
         function loadSysInfo() {
           fetch('/api/sysinfo')
             .then(res => res.json())
             .then(data => {
               document.getElementById('versionVal').innerText = data.version;
               const battEl = document.getElementById('battVal');
               if (data.batt_pct > 100) {
                   battEl.innerText = `${data.batt_v}V (${dict[currentLang].charging})`;
                   battEl.style.color = "#28a745"; 
                   battEl.style.fontWeight = "bold";
               } else {
                   battEl.innerText = `${data.batt_v}V (${data.batt_pct}%)`;
                   battEl.style.color = ""; 
                   battEl.style.fontWeight = "normal";
               }
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
         
         // ==========================================
         // 图片处理与上传逻辑
         // ==========================================
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
                 autoCropArea: 1 
             });
           }; 
           reader.readAsDataURL(file); 
         }
         
         function resetImage() {
           if (cropper) { cropper.destroy(); cropper = null; }
           document.getElementById('fileInput').value = '';
           document.getElementById('imageToCrop').src = '';
           document.getElementById('actionGroup').style.display = 'none';
           document.getElementById('varGroup').classList.add('hidden');
           hintArea.style.display = 'flex';
           wrapper.classList.add('empty-state');
           resetImageFilter();
         }
                  
         function resetImageFilter() {
           document.getElementById('wrapper').style.filter = 'none';
           document.getElementById('gamma').value = 0.82;
           document.getElementById('gammaDisplay').innerText = '0.82';
           document.getElementById('saturation').value = 100;
           document.getElementById('saturationDisplay').innerText = '100';
         }

         function showVarGroup() {
           document.getElementById('varGroup').classList.toggle('hidden');
         }
         
         function syncDeviceTime() {
           const now = new Date();
           const params = new URLSearchParams({
               y: now.getFullYear(),
               m: now.getMonth() + 1,
               d: now.getDate(),
               h: now.getHours(),
               min: now.getMinutes(),
               s: now.getSeconds()
           });
           fetch('/api/time?' + params.toString(), { method: 'POST' }).catch(()=>{});
         }
         
         async function processAndUpload() {
           //const btn = document.getElementById('btnSave');
           //const btnText = document.getElementById('btnText');
           //const btnClear = document.getElementById('btnClear');
           const allInputs = document.querySelectorAll('button, input');
           const progress = document.getElementById('saveProgress');
           if (cropper) {
             cropper.disable();
           }
           allInputs.forEach(element => {
             element.disabled = true;
           });
           progress.style.width = '0%';
           btnText.innerText = (currentLang === 'zh' ? '正在保存...' : 'Saving...');
           try {
             const w = mode === 'L' ? 1600 : 1200, h = mode === 'L' ? 1200 : 1600;
             const cvs = cropper.getCroppedCanvas({ width: w, height: h });
             const imgData = cvs.getContext('2d').getImageData(0, 0, w, h);
             const currentGamma = parseFloat(document.getElementById('gamma').value);
             const currentSaturation = parseFloat(document.getElementById('saturation').value) / 100.0;
             brightenForEink(imgData, currentGamma, currentSaturation);
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
             //btn.disabled = false;
             //btnClear.disabled = false;
              allInputs.forEach(element => {
                element.disabled = false;
              });
             progress.style.width = '0%';
             btnText.innerText = dict[currentLang].btnSave;
             resetImage(); 
           }
         }
         
         function brightenForEink(imgData, gamma, saturation) {
           const { data } = imgData;
           const lut = new Uint8Array(256);
           
           for (let i = 0; i < 256; i++) {
             lut[i] = Math.round(Math.pow(i / 255, gamma) * 255);
           }
           
           for (let i = 0; i < data.length; i += 4) {
             let r = lut[data[i]];
             let g = lut[data[i + 1]];
             let b = lut[data[i + 2]];
             
             if (saturation !== 1.0) {
               const luma = 0.299 * r + 0.587 * g + 0.114 * b;
               
               r = luma + (r - luma) * saturation;
               g = luma + (g - luma) * saturation;
               b = luma + (b - luma) * saturation;
               
               r = Math.min(255, Math.max(0, r));
               g = Math.min(255, Math.max(0, g));
               b = Math.min(255, Math.max(0, b));
             }
             
             data[i]     = r;
             data[i + 1] = g;
             data[i + 2] = b;
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
               const diff = (nx, ny) => {
                 if (nx >= 0 && nx < width && ny < height) {
                   const idx = (ny * width + nx) * 4;
                   buffer[idx]     += er * 0.125;
                   buffer[idx + 1] += eg * 0.125;
                   buffer[idx + 2] += eb * 0.125;
                 }
               };
               diff(x + 1, y); diff(x + 2, y); diff(x - 1, y + 1);
               diff(x,     y + 1); diff(x + 1, y + 1); diff(x,     y + 2);
             }
           }
         }
         
         function createBMP(w, h, data) {
           const rowSize = Math.floor((w * 3 + 3) / 4) * 4;
           const size = 54 + rowSize * h;
           const buffer = new ArrayBuffer(size);
           const v = new DataView(buffer);
           v.setUint8(0, 0x42); v.setUint8(1, 0x4D); v.setUint32(2, size, true); v.setUint32(10, 54, true); v.setUint32(14, 40, true);
           v.setInt32(18, w, true); v.setInt32(22, h, true); v.setUint16(26, 1, true); v.setUint16(28, 24, true);
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
