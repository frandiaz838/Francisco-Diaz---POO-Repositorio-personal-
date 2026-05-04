import asyncio
import json
from pathlib import Path
from typing import Any, Dict, Set

from fastapi import FastAPI, WebSocket, WebSocketDisconnect

STATE_FILE = Path(__file__).resolve().parent / "canvas_data.json"

# JSON compacto en disco (sin indent): menos I/O y menos bloqueo si usás SSD lentos / VPS pequeños.
def _dump_state(state: Dict[str, Any]) -> str:
    return json.dumps(state, ensure_ascii=False, separators=(",", ":"))


def schedule_disk_persist(state: Dict[str, Any]) -> None:
    """Guarda en disco sin bloquear el WebSocket: primero la red, después el archivo."""
    text = _dump_state(state)

    async def _write() -> None:
        await asyncio.to_thread(STATE_FILE.write_text, text, encoding="utf-8")

    asyncio.create_task(_write())


def default_state() -> Dict[str, Any]:
    return {"revision": 0, "strokes": []}


def load_state() -> Dict[str, Any]:
    if not STATE_FILE.exists():
        return default_state()
    try:
        data = json.loads(STATE_FILE.read_text(encoding="utf-8"))
        if not isinstance(data, dict):
            return default_state()
        data.setdefault("revision", 0)
        data.setdefault("strokes", [])
        return data
    except (json.JSONDecodeError, OSError):
        return default_state()


state = load_state()
clients: Set[WebSocket] = set()

app = FastAPI()


@app.get("/canvas")
async def get_canvas():
    return {"revision": state["revision"], "strokes": state["strokes"]}


@app.post("/canvas/save")
async def save_canvas(body: Dict[str, Any]):
    global state
    base = int(body.get("baseRevision", -1))
    incoming = body.get("strokes") or []
    if not isinstance(incoming, list):
        incoming = []

    if base != int(state["revision"]):
        return {
            "conflict": True,
            "revision": state["revision"],
            "strokes": state["strokes"],
        }

    existing_ids = {s.get("id") for s in state["strokes"] if isinstance(s, dict)}
    added = 0
    for st in incoming:
        if not isinstance(st, dict):
            continue
        sid = st.get("id")
        if not sid or sid in existing_ids:
            continue
        state["strokes"].append(st)
        existing_ids.add(sid)
        added += 1

    if added:
        state["revision"] = int(state["revision"]) + 1
        schedule_disk_persist(state)

    return {"ok": True, "revision": state["revision"]}


async def broadcast_json(message: Dict[str, Any]) -> None:
    text = json.dumps(message, ensure_ascii=False, separators=(",", ":"))
    dead = []
    for ws in clients:
        try:
            await ws.send_text(text)
        except Exception:
            dead.append(ws)
    for ws in dead:
        clients.discard(ws)


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    clients.add(websocket)
    try:
        await websocket.send_json(
            {
                "type": "full_resync",
                "revision": state["revision"],
                "strokes": state["strokes"],
            }
        )
        while True:
            raw = await websocket.receive_text()
            try:
                data = json.loads(raw)
            except json.JSONDecodeError:
                continue
            if not isinstance(data, dict):
                continue
            if data.get("type") != "stroke":
                continue
            stroke = data.get("stroke")
            if not isinstance(stroke, dict):
                continue
            sid = stroke.get("id")
            if not sid:
                continue
            existing_ids = {s.get("id") for s in state["strokes"] if isinstance(s, dict)}
            if sid in existing_ids:
                continue
            state["strokes"].append(stroke)
            state["revision"] = int(state["revision"]) + 1
            await broadcast_json(
                {
                    "type": "stroke",
                    "stroke": stroke,
                    "revision": state["revision"],
                }
            )
            schedule_disk_persist(state)
    except WebSocketDisconnect:
        pass
    finally:
        clients.discard(websocket)
