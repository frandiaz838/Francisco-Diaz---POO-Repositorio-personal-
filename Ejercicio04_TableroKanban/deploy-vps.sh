#!/bin/bash
# Script de deploy para el VPS - Ejercicio 04 Kanban
# Ejecutar en la terminal del VPS (desde VS Code SSH)

set -e
echo "=== Deploy Kanban VPS ==="

# ── 1. Verificar Docker ──────────────────────────────────────────────────────
echo "[1/6] Verificando Docker..."
docker --version || { echo "ERROR: Docker no instalado"; exit 1; }

# ── 2. Crear estructura de directorios ───────────────────────────────────────
echo "[2/6] Creando directorio del proyecto..."
mkdir -p /opt/vps-poo
cd /opt/vps-poo

# ── 3. Crear archivo .env ───────────────────────────────────────────────────
echo "[3/6] Creando .env..."
cat > .env << 'ENVEOF'
MYSQL_ROOT_PASSWORD=vpsroot
MYSQL_DATABASE=vps-poo
MYSQL_USER=poo_user
MYSQL_PASSWORD=poo_pass
JWT_SECRET=dev_secret_change_me
NGINX_SERVER_NAME=167.86.77.220
USE_LETSENCRYPT=0
CERTBOT_EMAIL=admin@example.com
ENVEOF

echo "   .env creado OK"

# ── 4. Verificar que existan los archivos del profe ──────────────────────────
echo "[4/6] Verificando archivos..."
if [ ! -f docker-compose.yml ]; then
    echo ""
    echo "  ATENCION: No se encontro docker-compose.yml en /opt/vps-poo"
    echo "  Necesitas subir los archivos del profe primero."
    echo "  Instrucciones al final de este script."
    echo ""
fi

echo ""
echo "=== INSTRUCCIONES COMPLETAR MANUALMENTE ==="
echo ""
echo "1. Subir archivos del profe al VPS (desde tu PC con Windows):"
echo "   Ejecuta esto en PowerShell de tu PC:"
echo "   scp -r 'C:/Users/octav/Downloads/vps-poo-2026-main/vps-poo-2026-main/*' root@167.86.77.220:/opt/vps-poo/"
echo ""
echo "2. Despues subir el backend del Kanban:"
echo "   scp 'C:/Users/octav/OneDrive/Escritorio/UBP/3er año/POO/Ejercicio04_TableroKanban/backend-extension/app/kanban.py' root@167.86.77.220:/opt/vps-poo/backend/app/"
echo ""
echo "3. En el VPS, agregar al final de /opt/vps-poo/backend/app/main.py:"
echo "   from .kanban import router as kanban_router"
echo "   app.include_router(kanban_router)"
echo ""
echo "4. Arrancar todo:"
echo "   cd /opt/vps-poo && docker-compose up -d --build"
echo ""
echo "5. Crear tablas de kanban (esperar 15 segundos a que MySQL arranque):"
echo "   sleep 15"
echo "   docker-compose exec db mysql -u poo_user -ppoo_pass vps-poo < /tmp/kanban_tables.sql"
echo ""
