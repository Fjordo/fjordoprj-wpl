<?php
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');

include("connect.php");

// Token authentication
$token = isset($_POST['token']) ? $_POST['token'] : '';
if ($token !== getenv('API_TOKEN')) {
    http_response_code(403);
    die('Forbidden');
}

// Input validation
$distanza = isset($_POST['dist']) ? intval($_POST['dist']) : null;
$volume   = isset($_POST['vol'])  ? floatval($_POST['vol']) : null;

if ($distanza === null || $volume === null) {
    http_response_code(400);
    die('Bad Request: missing dist or vol');
}

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

$stmt = mysqli_prepare($link, "INSERT INTO `wpl` (`distanza`, `volume_residuo`, `data_misurazione`) VALUES (?, ?, NOW())");
mysqli_stmt_bind_param($stmt, 'id', $distanza, $volume);
mysqli_stmt_execute($stmt);
mysqli_stmt_close($stmt);
mysqli_close($link);

http_response_code(201);
