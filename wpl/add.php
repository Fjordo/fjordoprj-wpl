<?php
declare(strict_types=1);

header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');

include("connect.php");

// Token authentication
$token = isset($_POST['token']) ? (string)$_POST['token'] : '';
if ($token !== (string)(getenv('API_TOKEN') ?: '')) {
    http_response_code(403);
    die('Forbidden');
}

// Input validation
if (!isset($_POST['dist'], $_POST['vol'])) {
    http_response_code(400);
    die('Bad Request: missing dist or vol');
}

$distanza = (int)$_POST['dist'];
$volume   = (float)$_POST['vol'];

if ($distanza < 0 || $distanza > 500) {
    http_response_code(400);
    die('Bad Request: dist out of range (0-500 cm)');
}

$link = Connection();

// Rate limiting: reject if a measurement was already stored in the last 5 minutes.
// Prevents flooding while leaving room for the Arduino's 5-second retry gap.
$rl = mysqli_query($link, "SELECT COUNT(*) FROM wpl WHERE data_misurazione >= (NOW() - INTERVAL 5 MINUTE)");
if ((int) mysqli_fetch_row($rl)[0] > 0) {
    http_response_code(429);
    mysqli_close($link);
    die('Too Many Requests: wait before sending a new measurement');
}

// Optional NTP timestamp from the Arduino Yun Linux side (Unix seconds)
$ts_arduino = null;
if (isset($_POST['ts']) && ctype_digit((string)$_POST['ts'])) {
    $ts_arduino = date('Y-m-d H:i:s', (int)$_POST['ts']);
}

$stmt = mysqli_prepare($link, "INSERT INTO `wpl` (`distanza`, `volume_residuo`, `data_misurazione`, `timestamp_arduino`) VALUES (?, ?, NOW(), ?)");
mysqli_stmt_bind_param($stmt, 'ids', $distanza, $volume, $ts_arduino);
mysqli_stmt_execute($stmt);
mysqli_stmt_close($stmt);
mysqli_close($link);

http_response_code(201);
