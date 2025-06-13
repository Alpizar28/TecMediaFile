// Configuración del servidor
const SERVER_URL = "http://localhost:18080";

// Referencias a elementos del DOM
const fileInput = document.getElementById("fileInput");
const btnUpload = document.getElementById("btnUpload");
const uploadResult = document.getElementById("uploadResult");
const delName = document.getElementById("delName");
const btnDelete = document.getElementById("btnDelete");
const filesTable = document
  .getElementById("files-table")
  .getElementsByTagName("tbody")[0];

// Mostrar mensajes
function showMessage(element, message, isError = false) {
  element.textContent = message;
  element.style.color = isError ? "#e74c3c" : "#27ae60";
}

// Subir archivo
async function uploadFile() {
  const file = fileInput.files[0];
  if (!file) {
    showMessage(uploadResult, "Por favor selecciona un archivo", true);
    return;
  }
  try {
    const response = await fetch(
      `${SERVER_URL}/upload/${encodeURIComponent(file.name)}`,
      { method: "PUT", body: file }
    );
    if (response.ok) {
      showMessage(uploadResult, `Archivo "${file.name}" subido exitosamente`);
      fileInput.value = "";
      await loadFilesAndStatus();
    } else {
      const errorText = await response.text();
      showMessage(
        uploadResult,
        `Error: ${response.status} - ${errorText}`,
        true
      );
    }
  } catch (error) {
    showMessage(uploadResult, `Error de conexión: ${error.message}`, true);
  }
}

// Descargar archivo
async function downloadFile(fileName) {
  try {
    const response = await fetch(
      `${SERVER_URL}/download/${encodeURIComponent(fileName)}`
    );
    if (response.ok) {
      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = fileName;
      document.body.appendChild(a);
      a.click();
      a.remove();
      window.URL.revokeObjectURL(url);
    } else {
      const errorText = await response.text();
      alert(`Error al descargar: ${response.status} - ${errorText}`);
    }
  } catch (error) {
    alert(`Error de conexión: ${error.message}`);
  }
}

// Eliminar archivo
async function deleteFile(fileName, refresh = true) {
  if (!confirm(`¿Eliminar "${fileName}"? Esta acción es irreversible.`)) return;
  try {
    const response = await fetch(
      `${SERVER_URL}/delete/${encodeURIComponent(fileName)}`,
      { method: "DELETE" }
    );
    if (response.ok) {
      if (refresh) await loadFilesAndStatus();
    } else {
      const errorText = await response.text();
      alert(`Error: ${response.status} - ${errorText}`);
    }
  } catch (error) {
    alert(`Error de conexión: ${error.message}`);
  }
}

// Ver nodos activos e inactivos y permitir eliminar/recuperar
async function showAndManageNodes(filename, manageCell) {
  try {
    // 1) Fetch de nodos activos
    const resNodes = await fetch(
      `${SERVER_URL}/nodes/${encodeURIComponent(filename)}`
    );
    if (!resNodes.ok) throw new Error(`GET /nodes ${resNodes.status}`);
    const { active_nodes } = await resNodes.json();

    // 2) Fetch de estado completo para detectar inactivos
    const resStatus = await fetch(`${SERVER_URL}/status`);
    if (!resStatus.ok) throw new Error(`GET /status ${resStatus.status}`);
    const statusMap = await resStatus.json();
    const allStatuses = statusMap[filename] || [];

    manageCell.innerHTML = "";

    // Mostrar nodos activos (Eliminar)
    active_nodes.forEach((nodeId) => {
      const badge = document.createElement("span");
      badge.textContent = `N${nodeId}`;
      badge.className = "badge badge-success m-1";
      manageCell.appendChild(badge);

      const btnDel = document.createElement("button");
      btnDel.textContent = "Eliminar Nodo";
      btnDel.className = "btn btn-sm btn-danger m-1";
      manageCell.appendChild(btnDel);

      btnDel.addEventListener("click", async () => {
        if (!confirm(`⚠️ Eliminar nodo ${nodeId} de '${filename}'?`)) return;
        const delRes = await fetch(
          `${SERVER_URL}/delete_node/${encodeURIComponent(filename)}/${nodeId}`,
          { method: "DELETE" }
        );
        const result = await delRes.json();
        alert(result.message || result.error);
        await showAndManageNodes(filename, manageCell);
      });
    });

    // Separador
    manageCell.appendChild(document.createElement("hr"));

    // Mostrar nodos inactivos (Recuperar)
    allStatuses.forEach((isActive, nodeId) => {
      if (!isActive) {
        const badge = document.createElement("span");
        badge.textContent = `N${nodeId}`;
        badge.className = "badge badge-danger m-1";
        manageCell.appendChild(badge);

        const btnRec = document.createElement("button");
        btnRec.textContent = "Recuperar Nodo";
        btnRec.className = "btn btn-sm btn-success m-1";
        manageCell.appendChild(btnRec);

        btnRec.addEventListener("click", async () => {
          if (!confirm(`⚠️ Recuperar nodo ${nodeId} de '${filename}'?`)) return;
          const recRes = await fetch(
            `${SERVER_URL}/enable_node/${encodeURIComponent(
              filename
            )}/${nodeId}`,
            { method: "PUT" }
          );
          const result = await recRes.json();
          alert(result.message || result.error);
          await showAndManageNodes(filename, manageCell);
        });
      }
    });
  } catch (err) {
    manageCell.textContent = "Error cargando nodos";
    console.error(err);
  }
}

// Cargar archivos y estado de nodos
async function loadFilesAndStatus() {
  try {
    const listRes = await fetch(`${SERVER_URL}/list`);
    const files = listRes.ok ? await listRes.json() : [];
    const statusRes = await fetch(`${SERVER_URL}/status`);
    const statusMap = statusRes.ok ? await statusRes.json() : {};

    filesTable.innerHTML = "";
    if (!Array.isArray(files) || files.length === 0) {
      Object.keys(statusMap).forEach((fileName) => {
        files.push({ name: fileName, size: 0 });
      });
    }
    if (files.length === 0) {
      const row = filesTable.insertRow();
      const cell = row.insertCell(0);
      cell.colSpan = 5;
      cell.textContent = "No hay archivos disponibles";
      return;
    }

    files.forEach((file) => {
      const { name, size = 0 } = file;
      const row = filesTable.insertRow();
      row.insertCell(0).textContent = name;
      row.insertCell(1).textContent = formatFileSize(size);

      // Acciones: Descargar y Eliminar archivo
      const actions = row.insertCell(2);
      const dlBtn = document.createElement("button");
      dlBtn.textContent = "Descargar";
      dlBtn.className = "btn btn-sm btn-primary m-1";
      dlBtn.onclick = () => downloadFile(name);
      const delBtn = document.createElement("button");
      delBtn.textContent = "Eliminar";
      delBtn.className = "btn btn-sm btn-danger m-1";
      delBtn.onclick = () => deleteFile(name);
      actions.append(dlBtn, delBtn);

      // Estado de nodos
      const stateCell = row.insertCell(3);
      (statusMap[name] || []).forEach((isActive, idx) => {
        const dot = document.createElement("span");
        dot.textContent = `N${idx}`;
        dot.className = isActive ? "node-active m-1" : "node-inactive m-1";
        stateCell.appendChild(dot);
      });

      // Botón “Ver nodos”
      const manageCell = row.insertCell(4);
      const viewBtn = document.createElement("button");
      viewBtn.textContent = "Ver nodos";
      viewBtn.className = "btn btn-sm btn-warning m-1";
      manageCell.appendChild(viewBtn);
      viewBtn.addEventListener("click", () =>
        showAndManageNodes(name, manageCell)
      );
    });
  } catch (err) {
    console.error("Error cargando archivos:", err);
  }
}

// Formatear tamaño de archivo
function formatFileSize(bytes) {
  const sizes = ["Bytes", "KB", "MB", "GB"];
  if (bytes === 0) return "0 Bytes";
  const i = Math.floor(Math.log(bytes) / Math.log(1024));
  return (bytes / Math.pow(1024, i)).toFixed(2) + " " + sizes[i];
}

// Eventos DOM
btnUpload.addEventListener("click", uploadFile);
btnDelete.addEventListener("click", () =>
  deleteFile(delName.value.trim(), true)
);

// Inicialización automática
(async () => {
  await loadFilesAndStatus();
  setInterval(loadFilesAndStatus, 7000);
})();
