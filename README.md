# Lotus Engine - WebAssembly Demo 🌐

This directory contains a minimalist web demo showcasing the speed and accuracy of the **Lotus Engine** running directly in the browser via WebAssembly (WASM).

## ✨ Features

- **Real-time Vietnamese Transformation**: Test TELEX and VNI input methods seamlessly.
- **Engine Logs**: View the 8-stage processing pipeline directly in the "Under the hood" UI.
- **Zero Configuration Server**: Runs entirely on the client-side. No backend required!

## 🛠️ Building

To build the WASM module from source, ensure you have the [Emscripten SDK (emsdk)](https://emscripten.org/docs/getting_started/downloads.html) installed and activated in your terminal.

Then, from the root of the project, run:

```bash
./build_wasm.sh
```

This script will compile the core engine and its C-API wrapper using `emcc`, generating `lotus_engine.js` and `lotus_engine.wasm` inside the `web/` directory.

## 🚀 Running Locally

Because browsers strictly block WASM file fetching over the `file://` protocol due to CORS, you must serve this directory with a local HTTP server.

Using Python 3:

```bash
cd web
python3 -m http.server 8000
```

Then open `http://localhost:8000` in your web browser.

## 🌐 Deployment (GitHub Pages)

Since the demo is just static HTML/JS/WASM files, you can easily host this directory on GitHub Pages or any static file hosting service.

1. Configure GitHub Pages to serve from the `docs` folder or `gh-pages` branch.
2. Ensure `index.html`, `style.css`, `app.js`, `lotus_engine.js`, and `lotus_engine.wasm` are included.
