// app.js — mejoras en upload y listFiles :contentReference[oaicite:0]{index=0}

const API = "http://localhost:18080";

document.addEventListener("DOMContentLoaded", () => {
  // Upload
  document.getElementById("btnUpload").addEventListener("click", upload);

  // Listar
  document.getElementById("btnListar").addEventListener("click", listFiles);

  // Download
  document.getElementById("btnDownload").addEventListener("click", download);

  // Delete
  document.getElementById("btnDelete").addEventListener("click", remove);
});

async function upload() {
  const input = document.getElementById("fileInput");
  const result = document.getElementById("uploadResult");
  result.textContent = "";
  if (!input.files.length) {
    result.textContent = "⚠️ Selecciona un archivo";
    return;
  }
  const file = input.files[0];
  try {
    const res = await fetch(`${API}/upload/${encodeURIComponent(file.name)}`, {
      method: "PUT",
      mode: "cors",
      body: file,
    });
    const txt = await res.text();
    result.textContent = (res.ok ? "✅ " : "❌ ") + txt;
  } catch (e) {
    result.textContent = "❌ Error de red: " + e.message;
  }
}

async function listFiles() {
  const listEl = document.getElementById("fileList");
  const errEl = document.getElementById("listError");
  listEl.innerHTML = "";
  errEl.textContent = "";
  try {
    const res = await fetch(`${API}/list`, { mode: "cors" });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const json = await res.json();
    if (!json.files || !json.files.length) {
      errEl.textContent = "No hay archivos almacenados.";
      return;
    }
    // Pintar cada fichero en un <li>
    json.files.forEach((name) => {
      const li = document.createElement("li");
      li.textContent = name;
      listEl.appendChild(li);
    });
  } catch (e) {
    errEl.textContent = "❌ Error listando archivos: " + e.message;
  }
}

function download() {
  const name = document.getElementById("downName").value.trim();
  if (!name) return alert("Escribe un nombre");
  window.open(`${API}/download/${encodeURIComponent(name)}`, "_blank");
}

async function remove() {
  const name = document.getElementById("delName").value.trim();
  const resEl = document.getElementById("delResult");
  resEl.textContent = "";
  if (!name) {
    resEl.textContent = "⚠️ Escribe un nombre";
    return;
  }
  try {
    const res = await fetch(`${API}/delete/${encodeURIComponent(name)}`, {
      method: "DELETE",
      mode: "cors",
    });
    const txt = await res.text();
    resEl.textContent = (res.ok ? "✅ " : "❌ ") + txt;
  } catch (e) {
    resEl.textContent = "❌ Error de red: " + e.message;
  }
}
