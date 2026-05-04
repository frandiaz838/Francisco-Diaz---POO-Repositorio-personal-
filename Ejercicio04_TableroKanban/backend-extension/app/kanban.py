"""
Endpoints Kanban - Ejercicio 04
Montar sobre la app FastAPI existente (ver PARCHE-main.py.txt).
Rutas expuestas detras de nginx en /api/kanban/...

Autenticacion: Basic Auth del nginx (poo:clavepoo). Sin JWT propio.
"""

from __future__ import annotations

from typing import List, Optional

from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field

from .main import get_db

router = APIRouter(prefix="/kanban", tags=["kanban"])


# ── Modelos Pydantic ─────────────────────────────────────────────────────────

class ColumnaIn(BaseModel):
    name: str = Field(..., min_length=1, max_length=100)


class TarjetaIn(BaseModel):
    column_id: int
    title: str = Field(..., min_length=1, max_length=200)
    description: Optional[str] = None


class TarjetaUpdate(BaseModel):
    title: str = Field(..., min_length=1, max_length=200)
    description: Optional[str] = None


class MoverTarjeta(BaseModel):
    column_id: int
    position: int = Field(9999, ge=0)


class ReordenarColumna(BaseModel):
    card_ids: List[int]


# ── Helpers ──────────────────────────────────────────────────────────────────

def _recalcular_posiciones(cur, column_id: int) -> None:
    """Renumera 0,1,2,... las tarjetas de la columna por posicion actual."""
    cur.execute(
        "SELECT id FROM kanban_cards WHERE column_id = %s ORDER BY position ASC, id ASC",
        (column_id,),
    )
    for i, (cid,) in enumerate(cur.fetchall()):
        cur.execute("UPDATE kanban_cards SET position = %s WHERE id = %s", (i, cid))


# ── GET /kanban/board ─────────────────────────────────────────────────────────

@router.get("/board")
def get_board():
    """Devuelve todas las columnas con sus tarjetas ordenadas por posicion."""
    conn = get_db()
    try:
        cur = conn.cursor(dictionary=True)
        cur.execute(
            "SELECT id, name, position FROM kanban_columns ORDER BY position ASC, id ASC"
        )
        cols = cur.fetchall()

        cur.execute(
            "SELECT id, column_id, title, description, position "
            "FROM kanban_cards ORDER BY column_id, position ASC, id ASC"
        )
        cards = cur.fetchall()
    finally:
        conn.close()

    cards_by_col: dict = {}
    for c in cards:
        cards_by_col.setdefault(c["column_id"], []).append({
            "id":          c["id"],
            "column_id":   c["column_id"],
            "title":       c["title"],
            "description": c["description"] or "",
            "position":    c["position"],
        })

    return [
        {
            "id":       col["id"],
            "name":     col["name"],
            "position": col["position"],
            "cards":    cards_by_col.get(col["id"], []),
        }
        for col in cols
    ]


# ── CRUD Columnas ─────────────────────────────────────────────────────────────

@router.post("/columns", status_code=status.HTTP_201_CREATED)
def crear_columna(data: ColumnaIn):
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute("SELECT COALESCE(MAX(position), -1) + 1 FROM kanban_columns")
        pos = cur.fetchone()[0]
        cur.execute(
            "INSERT INTO kanban_columns (name, position) VALUES (%s, %s)",
            (data.name, pos),
        )
        conn.commit()
        return {"id": cur.lastrowid, "name": data.name, "position": pos}
    finally:
        conn.close()


@router.put("/columns/{col_id}")
def editar_columna(col_id: int, data: ColumnaIn):
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute(
            "UPDATE kanban_columns SET name = %s WHERE id = %s",
            (data.name, col_id),
        )
        if cur.rowcount == 0:
            raise HTTPException(status_code=404, detail="Columna no encontrada")
        conn.commit()
        return {"ok": True}
    finally:
        conn.close()


@router.delete("/columns/{col_id}", status_code=status.HTTP_204_NO_CONTENT)
def eliminar_columna(col_id: int):
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute("DELETE FROM kanban_columns WHERE id = %s", (col_id,))
        if cur.rowcount == 0:
            raise HTTPException(status_code=404, detail="Columna no encontrada")
        conn.commit()
    finally:
        conn.close()


# ── CRUD Tarjetas ─────────────────────────────────────────────────────────────

@router.post("/cards", status_code=status.HTTP_201_CREATED)
def crear_tarjeta(data: TarjetaIn):
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute("SELECT id FROM kanban_columns WHERE id = %s", (data.column_id,))
        if not cur.fetchone():
            raise HTTPException(status_code=404, detail="Columna no encontrada")
        cur.execute(
            "SELECT COALESCE(MAX(position), -1) + 1 FROM kanban_cards WHERE column_id = %s",
            (data.column_id,),
        )
        pos = cur.fetchone()[0]
        cur.execute(
            "INSERT INTO kanban_cards (column_id, title, description, position) VALUES (%s, %s, %s, %s)",
            (data.column_id, data.title, data.description, pos),
        )
        conn.commit()
        return {
            "id": cur.lastrowid,
            "column_id": data.column_id,
            "title": data.title,
            "description": data.description or "",
            "position": pos,
        }
    finally:
        conn.close()


@router.put("/cards/{card_id}")
def editar_tarjeta(card_id: int, data: TarjetaUpdate):
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute(
            "UPDATE kanban_cards SET title = %s, description = %s WHERE id = %s",
            (data.title, data.description, card_id),
        )
        if cur.rowcount == 0:
            raise HTTPException(status_code=404, detail="Tarjeta no encontrada")
        conn.commit()
        return {"ok": True}
    finally:
        conn.close()


@router.delete("/cards/{card_id}", status_code=status.HTTP_204_NO_CONTENT)
def eliminar_tarjeta(card_id: int):
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute("SELECT column_id FROM kanban_cards WHERE id = %s", (card_id,))
        row = cur.fetchone()
        if not row:
            raise HTTPException(status_code=404, detail="Tarjeta no encontrada")
        col_id = row[0]
        cur.execute("DELETE FROM kanban_cards WHERE id = %s", (card_id,))
        _recalcular_posiciones(cur, col_id)
        conn.commit()
    finally:
        conn.close()


@router.put("/cards/{card_id}/move")
def mover_tarjeta(card_id: int, data: MoverTarjeta):
    """Mueve una tarjeta a otra columna (o reordena dentro de la misma)."""
    conn = get_db()
    try:
        cur = conn.cursor(dictionary=True)
        cur.execute(
            "SELECT column_id FROM kanban_cards WHERE id = %s", (card_id,)
        )
        card = cur.fetchone()
        if not card:
            raise HTTPException(status_code=404, detail="Tarjeta no encontrada")
        cur.execute("SELECT id FROM kanban_columns WHERE id = %s", (data.column_id,))
        if not cur.fetchone():
            raise HTTPException(status_code=404, detail="Columna destino no encontrada")

        old_col = card["column_id"]
        cur.execute(
            "UPDATE kanban_cards SET column_id = %s, position = %s WHERE id = %s",
            (data.column_id, data.position, card_id),
        )
        _recalcular_posiciones(cur, old_col)
        if data.column_id != old_col:
            _recalcular_posiciones(cur, data.column_id)
        conn.commit()
        return {"ok": True}
    finally:
        conn.close()


@router.put("/columns/{col_id}/reorder")
def reordenar_columna(col_id: int, data: ReordenarColumna):
    """Reordena las tarjetas de una columna segun la lista de IDs recibida."""
    conn = get_db()
    try:
        cur = conn.cursor()
        cur.execute("SELECT id FROM kanban_columns WHERE id = %s", (col_id,))
        if not cur.fetchone():
            raise HTTPException(status_code=404, detail="Columna no encontrada")
        for i, cid in enumerate(data.card_ids):
            cur.execute(
                "UPDATE kanban_cards SET position = %s WHERE id = %s AND column_id = %s",
                (i, cid, col_id),
            )
        conn.commit()
        return {"ok": True}
    finally:
        conn.close()
