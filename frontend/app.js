// app.js - Cliente REST para TECMFS

// Configuración del servidor
const SERVER_URL = "http://localhost:18080";

// Referencias a elementos del DOM
const fileInput = document.getElementById("fileInput");
const btnUpload = document.getElementById("btnUpload");
const uploadResult = document.getElementById("uploadResult");
const delName = document.getElementById("delName");
const btnDelete = document.getElementById("btnDelete");
const delResult = document.getElementById("delResult");
const filesTable = document
  .getElementById("files-table")
  .getElementsByTagName("tbody")[0];

// Función para mostrar mensajes
function showMessage(element, message, isError = false) {
  element.textContent = message;
  element.style.color = isError ? "#e74c3c" : "#27ae60";
}

// Función para subir archivo
async function uploadFile() {
  const file = fileInput.files[0];
  if (!file) {
    showMessage(uploadResult, "Por favor selecciona un archivo", true);
    return;
  }

  try {
    const response = await fetch(`${SERVER_URL}/upload/${file.name}`, {
      method: "PUT",
      body: file,
    });

    if (response.ok) {
      showMessage(uploadResult, `Archivo "${file.name}" subido exitosamente`);
      fileInput.value = "";
      // Actualizar la tabla inmediatamente y luego otra vez después de 1 segundo
      loadFilesAndStatus();
      setTimeout(loadFilesAndStatus, 1000);
    } else {
      const errorText = await response.text();
      showMessage(
        uploadResult,
        `Error al subir archivo: ${response.status} - ${errorText}`,
        true
      );
    }
  } catch (error) {
    showMessage(uploadResult, `Error de conexión: ${error.message}`, true);
  }
}

// Función para eliminar archivo
async function deleteFile() {
  const fileName = delName.value.trim();
  if (!fileName) {
    showMessage(delResult, "Por favor ingresa el nombre del archivo", true);
    return;
  }

  try {
    const response = await fetch(`${SERVER_URL}/delete/${fileName}`, {
      method: "DELETE",
    });

    if (response.ok) {
      showMessage(delResult, `Archivo "${fileName}" eliminado exitosamente`);
      delName.value = "";
      // Actualizar la tabla inmediatamente y luego otra vez después de 1 segundo
      loadFilesAndStatus();
      setTimeout(loadFilesAndStatus, 1000);
    } else {
      const errorText = await response.text();
      showMessage(
        delResult,
        `Error al eliminar archivo: ${response.status} - ${errorText}`,
        true
      );
    }
  } catch (error) {
    showMessage(delResult, `Error de conexión: ${error.message}`, true);
  }
}

// Función para descargar archivo
async function downloadFile(fileName) {
  try {
    const response = await fetch(`${SERVER_URL}/download/${fileName}`);

    if (response.ok) {
      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = fileName;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      window.URL.revokeObjectURL(url);
    } else {
      const errorText = await response.text();
      alert(`Error al descargar archivo: ${response.status} - ${errorText}`);
    }
  } catch (error) {
    alert(`Error de conexión: ${error.message}`);
  }
}

// Función para eliminar archivo desde la tabla
async function deleteFileFromTable(fileName) {
  if (confirm(`¿Estás seguro de que quieres eliminar "${fileName}"?`)) {
    try {
      const response = await fetch(`${SERVER_URL}/delete/${fileName}`, {
        method: "DELETE",
      });

      if (response.ok) {
        // Actualizar la tabla inmediatamente y luego otra vez después de 1 segundo
        loadFilesAndStatus();
        setTimeout(loadFilesAndStatus, 1000);
      } else {
        const errorText = await response.text();
        alert(`Error al eliminar archivo: ${response.status} - ${errorText}`);
      }
    } catch (error) {
      alert(`Error de conexión: ${error.message}`);
    }
  }
}

// Función para recuperar un nodo
async function recoverNode(nodeId) {
  try {
    console.log(`Intentando recuperar nodo ${nodeId}...`);
    const response = await fetch(`${SERVER_URL}/recover/${nodeId}`, {
      method: "POST",
    });

    if (response.ok) {
      const result = await response.json();
      console.log(`Recuperación de nodo ${nodeId}:`, result);

      // Mostrar mensaje de éxito/fallo
      alert(result.message || `Nodo ${nodeId} procesado`);

      // Actualizar la tabla para reflejar cambios
      setTimeout(loadFilesAndStatus, 500);
    } else {
      const errorText = await response.text();
      alert(
        `Error al recuperar nodo ${nodeId}: ${response.status} - ${errorText}`
      );
    }
  } catch (error) {
    alert(`Error de conexión al recuperar nodo ${nodeId}: ${error.message}`);
  }
}

// Función para limpiar nodos huérfanos
async function cleanupNodes() {
  try {
    console.log("Iniciando limpieza de nodos huérfanos...");
    const response = await fetch(`${SERVER_URL}/cleanup`, {
      method: "POST",
    });

    if (response.ok) {
      const result = await response.json();
      console.log("Resultado de limpieza:", result);
      alert(
        `Limpieza completada: ${result.cleaned_nodes || 0} nodos eliminados`
      );

      // Actualizar la tabla
      setTimeout(loadFilesAndStatus, 500);
    } else {
      const errorText = await response.text();
      alert(`Error en limpieza: ${response.status} - ${errorText}`);
    }
  } catch (error) {
    alert(`Error de conexión en limpieza: ${error.message}`);
  }
}

// Función para cargar archivos y su estado
async function loadFilesAndStatus() {
  try {
    console.log("Cargando archivos y estado...");
    console.log("Conectando con:", `${SERVER_URL}/list`);

    // Obtener lista de archivos
    const filesResponse = await fetch(`${SERVER_URL}/list`);
    let files = [];

    console.log("Respuesta /list - Status:", filesResponse.status);

    if (filesResponse.ok) {
      const responseData = await filesResponse.json();
      console.log("Datos recibidos de /list:", responseData);

      // CORRECCIÓN: Verificar que responseData sea un array válido
      if (Array.isArray(responseData)) {
        files = responseData.filter((file) => {
          // Verificar que cada archivo tenga al menos un nombre
          return file && (file.name || typeof file === "string");
        });

        // Normalizar archivos
        files = files.map((file) => {
          if (typeof file === "string") {
            return { name: file, size: 0 };
          }
          return {
            name: file.name || "Sin nombre",
            size: typeof file.size === "number" ? file.size : 0,
          };
        });
      } else {
        console.warn("Respuesta de /list no es un array:", responseData);
        files = [];
      }

      console.log("Archivos procesados:", files);
    } else {
      console.error(
        "Error al obtener lista de archivos:",
        filesResponse.status
      );
      const errorText = await filesResponse.text();
      console.error("Detalles del error:", errorText);
    }

    // Obtener estado de nodos
    let status = {};
    try {
      console.log("Obteniendo estado de nodos...");
      const statusResponse = await fetch(`${SERVER_URL}/status`);
      if (statusResponse.ok) {
        const statusData = await statusResponse.json();
        console.log("Estado de nodos recibido:", statusData);

        // CORRECCIÓN: Verificar que statusData sea un objeto válido
        if (
          statusData &&
          typeof statusData === "object" &&
          !Array.isArray(statusData)
        ) {
          status = statusData;
        } else {
          console.warn(
            "Respuesta de /status no es un objeto válido:",
            statusData
          );
          status = {};
        }
      } else {
        console.error(
          "Error al obtener estado de nodos:",
          statusResponse.status
        );
        const errorText = await statusResponse.text();
        console.error("Detalles del error de status:", errorText);
      }
    } catch (statusError) {
      console.error(
        "Error al conectar con el endpoint de status:",
        statusError
      );
    }

    // Limpiar tabla
    filesTable.innerHTML = "";

    // CORRECCIÓN: Verificar archivos disponibles
    console.log(`Archivos encontrados: ${files.length}`);
    console.log(`Archivos en status: ${Object.keys(status).length}`);

    // Si no hay archivos en /list pero sí en /status, usar los del status
    if (files.length === 0 && Object.keys(status).length > 0) {
      console.log("No hay archivos en /list, extrayendo de /status...");
      files = Object.keys(status).map((fileName) => ({
        name: fileName,
        size: 0, // No tenemos información de tamaño desde /status
      }));
      console.log("Archivos extraídos del status:", files);
    }

    // Si definitivamente no hay archivos
    if (files.length === 0) {
      const row = filesTable.insertRow();
      const cell = row.insertCell(0);
      cell.colSpan = 4;

      if (filesResponse.ok) {
        cell.textContent = "No hay archivos en el sistema";
        cell.style.color = "#7f8c8d";
      } else {
        cell.textContent = `Error de conexión: ${filesResponse.status} - Verifica que el servidor esté ejecutándose en ${SERVER_URL}`;
        cell.style.color = "#e74c3c";
      }

      cell.style.textAlign = "center";
      cell.style.fontStyle = "italic";
      return;
    }

    // Agregar filas para cada archivo
    files.forEach((file, index) => {
      try {
        const row = filesTable.insertRow();

        // Verificar datos del archivo
        const fileName = file.name || `archivo_${index}`;
        const fileSize = typeof file.size === "number" ? file.size : 0;

        console.log(`Procesando archivo: ${fileName}, tamaño: ${fileSize}`);

        // Columna de nombre
        const nameCell = row.insertCell(0);
        nameCell.textContent = fileName;

        // Columna de tamaño - CORRECCIÓN AQUÍ
        const sizeCell = row.insertCell(1);
        sizeCell.textContent = formatFileSize(fileSize);
        if (fileSize === 0) {
          sizeCell.style.color = "#f39c12"; // Naranja para tamaños 0
          sizeCell.title = "Tamaño no disponible o archivo vacío";
        }

        // Columna de acciones
        const actionsCell = row.insertCell(2);
        actionsCell.className = "actions-cell";

        const downloadBtn = document.createElement("button");
        downloadBtn.textContent = "Descargar";
        downloadBtn.className = "btn-download";
        downloadBtn.onclick = () => downloadFile(fileName);

        const deleteBtn = document.createElement("button");
        deleteBtn.textContent = "Eliminar";
        deleteBtn.className = "btn-delete";
        deleteBtn.onclick = () => deleteFileFromTable(fileName);

        actionsCell.appendChild(downloadBtn);
        actionsCell.appendChild(deleteBtn);

        // Columna de estado de nodos - CORRECCIÓN AQUÍ
        const statusCell = row.insertCell(3);
        const nodeStates = status[fileName];

        console.log(`Estado de nodos para ${fileName}:`, nodeStates);

        if (
          !nodeStates ||
          !Array.isArray(nodeStates) ||
          nodeStates.length === 0
        ) {
          statusCell.textContent = "Sin información";
          statusCell.style.color = "#7f8c8d";
          statusCell.style.fontStyle = "italic";
        } else {
          statusCell.innerHTML = ""; // Limpiar contenido

          nodeStates.forEach((isActive, nodeIndex) => {
            // Contenedor para cada nodo
            const nodeContainer = document.createElement("span");
            nodeContainer.style.marginRight = "8px";
            nodeContainer.style.display = "inline-block";

            // Indicador de estado
            const nodeIndicator = document.createElement("span");
            nodeIndicator.className = `node-status ${
              isActive ? "node-active" : "node-inactive"
            }`;
            nodeIndicator.style.display = "inline-block";
            nodeIndicator.style.width = "12px";
            nodeIndicator.style.height = "12px";
            nodeIndicator.style.borderRadius = "50%";
            nodeIndicator.style.marginRight = "4px";
            nodeIndicator.style.backgroundColor = isActive
              ? "#27ae60"
              : "#e74c3c";
            nodeIndicator.title = `Nodo ${nodeIndex}: ${
              isActive ? "Activo" : "Inactivo"
            }${!isActive ? " (Click para recuperar)" : ""}`;

            // Etiqueta del nodo
            const nodeLabel = document.createElement("span");
            nodeLabel.textContent = `N${nodeIndex}`;
            nodeLabel.style.fontSize = "0.8em";
            nodeLabel.style.color = isActive ? "#27ae60" : "#e74c3c";
            nodeLabel.style.fontWeight = "bold";

            // Si el nodo está inactivo, permitir recuperación
            if (!isActive) {
              nodeIndicator.style.cursor = "pointer";
              nodeLabel.style.cursor = "pointer";

              const recoveryHandler = () => recoverNode(nodeIndex);
              nodeIndicator.onclick = recoveryHandler;
              nodeLabel.onclick = recoveryHandler;
            }

            nodeContainer.appendChild(nodeIndicator);
            nodeContainer.appendChild(nodeLabel);
            statusCell.appendChild(nodeContainer);
          });
        }
      } catch (fileError) {
        console.error(
          `Error procesando archivo en índice ${index}:`,
          fileError
        );
      }
    });

    console.log("Tabla actualizada exitosamente");
  } catch (error) {
    console.error("Error crítico al cargar archivos y estado:", error);

    // Limpiar tabla y mostrar error
    filesTable.innerHTML = "";
    const row = filesTable.insertRow();
    const cell = row.insertCell(0);
    cell.colSpan = 4;
    cell.textContent = `Error crítico: ${error.message}. Verifica que el servidor TECMFS esté ejecutándose en ${SERVER_URL}`;
    cell.style.textAlign = "center";
    cell.style.color = "#e74c3c";
    cell.style.fontWeight = "bold";
  }
}

// Función para formatear tamaño de archivo
function formatFileSize(bytes) {
  if (bytes === 0) return "0 Bytes";
  if (bytes < 0) return "Error";

  const k = 1024;
  const sizes = ["Bytes", "KB", "MB", "GB", "TB"];
  const i = Math.floor(Math.log(bytes) / Math.log(k));

  if (i >= sizes.length) return `${bytes} Bytes`; // Fallback para tamaños muy grandes

  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + " " + sizes[i];
}

// Función para probar la conectividad del servidor
async function testServerConnection() {
  try {
    console.log("Probando conexión con el servidor...");
    const response = await fetch(`${SERVER_URL}/debug`, {
      method: "GET",
      mode: "cors",
    });
    console.log("Conexión exitosa, status:", response.status);

    if (response.ok) {
      const debugData = await response.json();
      console.log("Información del servidor:", debugData);
    }

    return response.ok;
  } catch (error) {
    console.error("Error de conexión:", error);
    return false;
  }
}

// Función para mostrar información de debug
async function showDebugInfo() {
  try {
    const response = await fetch(`${SERVER_URL}/debug`);
    if (response.ok) {
      const debugData = await response.json();
      console.log("=== INFORMACIÓN DE DEBUG ===");
      console.log("Estado del servidor:", debugData.server_status);
      console.log("Total de archivos:", debugData.total_files);
      console.log("Archivos:", debugData.files);
      console.log(
        "Timestamp:",
        new Date(debugData.timestamp * 1000).toLocaleString()
      );

      alert(
        `Servidor: ${debugData.server_status}\nArchivos: ${
          debugData.total_files
        }\nÚltima actualización: ${new Date(
          debugData.timestamp * 1000
        ).toLocaleString()}`
      );
    }
  } catch (error) {
    console.error("Error obteniendo información de debug:", error);
  }
}

// Event listeners
btnUpload.addEventListener("click", uploadFile);
btnDelete.addEventListener("click", deleteFile);

// Agregar botón de limpieza (si existe en el HTML)
const btnCleanup = document.getElementById("btnCleanup");
if (btnCleanup) {
  btnCleanup.addEventListener("click", cleanupNodes);
}

// Agregar botón de debug (si existe en el HTML)
const btnDebug = document.getElementById("btnDebug");
if (btnDebug) {
  btnDebug.addEventListener("click", showDebugInfo);
}

// Función de inicialización
async function initializeApp() {
  console.log("=== INICIANDO CLIENTE TECMFS ===");
  console.log("Servidor configurado:", SERVER_URL);

  const isConnected = await testServerConnection();
  if (!isConnected) {
    console.warn("⚠️  No se pudo conectar con el servidor TECMFS");
    console.warn("⚠️  Asegúrate de que esté ejecutándose en:", SERVER_URL);
    console.warn("⚠️  Verifica que el servidor tenga CORS habilitado");
  } else {
    console.log("✅ Conexión con servidor establecida");
  }

  // Cargar datos iniciales
  await loadFilesAndStatus();
}

// Inicializar aplicación
initializeApp();

// Actualizar datos cada 5 segundos (más frecuente para ver cambios rápidamente)
setInterval(loadFilesAndStatus, 5000);

// Limpiar consola cada minuto para evitar saturación
setInterval(() => {
  if (console.clear) {
    console.clear();
    console.log("=== CLIENTE TECMFS ACTIVO ===");
    console.log("Servidor:", SERVER_URL);
  }
}, 60000);
