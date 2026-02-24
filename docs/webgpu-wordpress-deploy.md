# WebGPU Build Deploy Guide (WordPress Hosting)

This guide is for deploying the browser build from this project to a website that uses WordPress.

Important: WordPress (`/wp-admin`) is not where you upload these files.  
You upload to the hosting file system (File Manager / cPanel / Plesk / SFTP).

## Short Answer

Yes, your father can upload the 3 release files into a folder like:

- `angelov.design/trywasm/`

If that folder is publicly served, this should work.

## What Gets Uploaded

From local build output:

- `build/Web/Release/index.html`
- `build/Web/Release/app.js`
- `build/Web/Release/app.wasm`

For this project, these 3 files are enough.

## 1) Build Release Locally

```bash
cd /Users/angelangelov/Repos/metal-cpp-glfw-webgpu
./scripts/generateProjectFiles.sh web
emmake make -B -C build/ProjectFiles/gmake config=release metalCppWeb
```

## 2) Upload To Hosting

In hosting panel or SFTP:

1. Open web root (`public_html`, `www`, or equivalent).
2. Create folder `trywasm` (or any name).
3. Upload `index.html`, `app.js`, `app.wasm` into that folder.

Expected URL:

- `https://angelov.design/trywasm/`

## 3) Ensure WASM MIME Type

Server must return `.wasm` as:

- `Content-Type: application/wasm`

If Apache, place `.htaccess` in the same folder:

```apache
AddType application/wasm .wasm
AddType application/javascript .js
```

## 4) Test After Upload

Open:

- `https://angelov.design/trywasm/`
- `https://angelov.design/trywasm/app.wasm`

If `app.wasm` gives 404 or wrong type, fix path/MIME before debugging anything else.

## 5) WordPress Page Integration (Optional)

If you want this inside a WP page, add a Custom HTML block:

```html
<iframe
  src="https://angelov.design/trywasm/"
  style="width:100%;height:800px;border:0;"
  loading="lazy"
></iframe>
```

## 6) Updating Without Breaking Live Page

Your note is correct: replacing files in-place can be awkward.

Safer method:

1. Upload each version to a new folder:
   - `/trywasm-v1/`
   - `/trywasm-v2/`
2. Test new version URL.
3. Only then switch iframe/page link to new folder.

This avoids downtime and rollback is easy.

## Common Issues

- Opening local `index.html` with `file://` instead of hosted URL.
- `.wasm` not served as `application/wasm`.
- Cache/optimizer plugins rewriting `app.js`.
- Browser without WebGPU enabled.

## Bulgarian Quick Note

Да, това ще работи:

> „Мога да отворя папка, примерно `angelov.design/trywasm` и да кача там файловете.“

По-стабилно за обновяване е да качвате по версии (`trywasm-v1`, `trywasm-v2`) и после да сменяте линка.
