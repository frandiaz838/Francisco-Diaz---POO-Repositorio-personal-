-- Tablas para el Ejercicio 04 - Tablero Kanban
-- Ejecutar en la DB del VPS: docker-compose exec db mysql -u poo_user -ppoo_pass vps-poo < kanban_tables.sql

CREATE TABLE IF NOT EXISTS kanban_columns (
  id         INT AUTO_INCREMENT PRIMARY KEY,
  name       VARCHAR(100) NOT NULL,
  position   INT NOT NULL DEFAULT 0,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS kanban_cards (
  id          INT AUTO_INCREMENT PRIMARY KEY,
  column_id   INT NOT NULL,
  title       VARCHAR(200) NOT NULL,
  description TEXT,
  position    INT NOT NULL DEFAULT 0,
  created_at  TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at  TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  FOREIGN KEY (column_id) REFERENCES kanban_columns(id) ON DELETE CASCADE
);

-- Columnas iniciales de ejemplo
INSERT IGNORE INTO kanban_columns (id, name, position) VALUES
  (1, 'Por hacer',   0),
  (2, 'En progreso', 1),
  (3, 'Listo',       2);
