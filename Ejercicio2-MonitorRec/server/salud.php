<?php
/**
 * Endpoint de salud para el panel Qt (MonitorVPS).
 * Subí este archivo a tu hosting (ej. public_html/salud.php) y apuntá la URL en la app.
 *
 * Requiere: PHP en un servidor Linux típico (VPS) con lectura de /proc.
 * Si algo falla, devuelve métricas de ejemplo para que igual veas JSON válido.
 */

declare(strict_types=1);

header('Content-Type: application/json; charset=utf-8');
header('Cache-Control: no-store');

// Opcional: protección simple. Definí la variable de entorno en Apache/Nginx o dejá comentado.
// if (getenv('SALUD_API_KEY') && ($_GET['key'] ?? '') !== getenv('SALUD_API_KEY')) {
//     http_response_code(403);
//     echo json_encode(['status' => 'forbidden'], JSON_UNESCAPED_UNICODE);
//     exit;
// }

/**
 * @return array<int, int>|null jiffies de la línea agregada "cpu" de /proc/stat
 */
function cpu_jiffies_line(): ?array
{
    $fh = @fopen('/proc/stat', 'r');
    if ($fh === false) {
        return null;
    }
    $line = fgets($fh);
    fclose($fh);
    if ($line === false || strncmp($line, 'cpu ', 4) !== 0) {
        return null;
    }
    $parts = preg_split('/\s+/', trim($line));
    if ($parts === false || count($parts) < 5) {
        return null;
    }
    array_shift($parts); // "cpu"
    $nums = [];
    foreach ($parts as $p) {
        $nums[] = (int) $p;
    }
    return $nums;
}

function cpu_percent_linux(): ?float
{
    $a = cpu_jiffies_line();
    if ($a === null) {
        return null;
    }
    usleep(200000);
    $b = cpu_jiffies_line();
    if ($b === null) {
        return null;
    }
    $n = min(count($a), count($b));
    $a = array_slice($a, 0, $n);
    $b = array_slice($b, 0, $n);

    $idleIdx = 3;
    $sumA = array_sum($a);
    $sumB = array_sum($b);
    $total = $sumB - $sumA;
    if ($total <= 0) {
        return null;
    }
    $idle = ($b[$idleIdx] ?? 0) - ($a[$idleIdx] ?? 0);
    $used = $total - $idle;
    return round(100 * max(0, min(1, $used / $total)), 1);
}

function mem_used_percent_linux(): ?float
{
    $raw = @file_get_contents('/proc/meminfo');
    if ($raw === false) {
        return null;
    }
    $total = null;
    $avail = null;
    if (preg_match('/MemTotal:\s+(\d+)\s+kB/', $raw, $m)) {
        $total = (int) $m[1];
    }
    if (preg_match('/MemAvailable:\s+(\d+)\s+kB/', $raw, $m)) {
        $avail = (int) $m[1];
    }
    if ($total === null || $total <= 0) {
        return null;
    }
    if ($avail === null && preg_match('/MemFree:\s+(\d+)\s+kB/', $raw, $m)) {
        $avail = (int) $m[1];
    }
    if ($avail === null) {
        return null;
    }
    return round(100 * (1 - $avail / $total), 1);
}

function disk_used_percent_root(): ?float
{
    $total = @disk_total_space('/');
    $free = @disk_free_space('/');
    if ($total === false || $free === false || $total <= 0) {
        return null;
    }
    return round(100 * (1 - $free / $total), 1);
}

function loadavg_linux(): ?float
{
    if (function_exists('sys_getloadavg')) {
        $v = sys_getloadavg();
        if (is_array($v) && isset($v[0])) {
            return round((float) $v[0], 2);
        }
    }
    $raw = @file_get_contents('/proc/loadavg');
    if ($raw === false) {
        return null;
    }
    $parts = explode(' ', trim($raw));
    return isset($parts[0]) ? round((float) $parts[0], 2) : null;
}

function uptime_seconds_linux(): ?float
{
    $raw = @file_get_contents('/proc/uptime');
    if ($raw === false) {
        return null;
    }
    $parts = explode(' ', trim($raw));
    return isset($parts[0]) ? (float) $parts[0] : null;
}

function build_payload(bool $demo): array
{
    if ($demo) {
        return [
            'status' => 'ok',
            'cpu' => 12.5,
            'load' => 0.21,
            'mem_used_pct' => 48.0,
            'disk_usage' => 62.0,
            'uptime_sec' => 86400 * 3 + 3600 * 2,
            'uptime_human' => '3d 2h (demo)',
        ];
    }

    $cpu = cpu_percent_linux();
    $mem = mem_used_percent_linux();
    $disk = disk_used_percent_root();
    $load = loadavg_linux();
    $upSec = uptime_seconds_linux();

    $payload = [
        'status' => 'ok',
        'cpu' => $cpu,
        'load' => $load,
        'mem_used_pct' => $mem,
        'disk_usage' => $disk,
        'uptime_sec' => $upSec,
    ];

    // Si casi todo falló (no Linux / permisos), modo demo para no romper el cliente Qt.
    $missing = ($cpu === null) + ($mem === null) + ($disk === null) + ($load === null) + ($upSec === null);
    if ($missing >= 3) {
        return build_payload(true);
    }

    return $payload;
}

$demo = isset($_GET['demo']) && $_GET['demo'] === '1';
$payload = build_payload($demo);

$json = json_encode($payload, JSON_UNESCAPED_UNICODE);
if ($json === false) {
    http_response_code(500);
    echo '{"status":"error"}';
    exit;
}
echo $json;
