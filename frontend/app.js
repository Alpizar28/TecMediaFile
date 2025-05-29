const API = "http://localhost:18080";

async function upload() {
  const input = document.getElementById("fileInput");
  if (!input.files.length) return alert("Selecciona un archivo");
  const file = input.files[0];
  const res = await fetch(`${API}/upload/${encodeURIComponent(file.name)}`, {
    method: "PUT",
    mode: "cors",
    body: file,
  });
  const txt = await res.text();
  document.getElementById("uploadResult").innerText = txt;
}

async function listFiles() {
  console.log("listFiles() llamada");
  try {
    const res = await fetch(`${API}/list`, { mode: "cors" });
    console.log("fetch listo", res.status);
    const json = await res.json();
    console.log("json recibido", json);
    document.getElementById("listado").innerText = JSON.stringify(
      json,
      null,
      2
    );
  } catch (e) {
    console.error("Error listando archivos:", e);
  }
}

function download() {
  const name = document.getElementById("downName").value.trim();
  if (!name) return alert("Escribe un nombre");
  window.open(`${API}/download/${encodeURIComponent(name)}`, "_blank");
}

async function remove() {
  const name = document.getElementById("delName").value.trim();
  if (!name) return alert("Escribe un nombre");
  const res = await fetch(`${API}/delete/${encodeURIComponent(name)}`, {
    method: "DELETE",
    mode: "cors",
  });
  const txt = await res.text();
  document.getElementById("delResult").innerText = txt;
}

// Vinculamos el Listar cuando el DOM esté listo
document.addEventListener("DOMContentLoaded", () => {
  console.log("app.js cargado, vinculando botón Listar");
  const btn = document.getElementById("btnListar");
  if (btn) {
    btn.addEventListener("click", () => {
      console.log("Botón Listar clickeado");
      listFiles();
    });
  } else {
    console.error("No encontré el botón #btnListar");
  }
});
