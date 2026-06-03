<?php
include("connect.php");

$method  = $_SERVER['REQUEST_METHOD'];
$request = explode('/', trim($_SERVER['PATH_INFO'], '/'));
$input   = json_decode(file_get_contents('php://input'), true);

$link = Connection();

$table = preg_replace('/[^a-z0-9_]+/i', '', array_shift($request));

switch ($method) {
    case 'GET':
        $sql = "SELECT distanza, volume_residuo, data_misurazione
                FROM wpl
                WHERE data_misurazione >= (NOW() - INTERVAL 3 MONTH)
                ORDER BY data_misurazione DESC";
        break;
    default:
        http_response_code(405);
        die('Method Not Allowed');
}

$result = mysqli_query($link, $sql);

if (!$result) {
    http_response_code(500);
    die(mysqli_error($link));
}

header('Content-Type: application/json');
echo '[';
for ($i = 0; $i < mysqli_num_rows($result); $i++) {
    echo ($i > 0 ? ',' : '') . json_encode(mysqli_fetch_object($result));
}
echo ']';

mysqli_close($link);
