<?php
declare(strict_types=1);

include("connect.php");

$method = $_SERVER['REQUEST_METHOD'];

// Whitelist of allowed range values mapped to SQL intervals
$range_map = [
    '1m'  => 'INTERVAL 1 MONTH',
    '3m'  => 'INTERVAL 3 MONTH',
    '6m'  => 'INTERVAL 6 MONTH',
    '1y'  => 'INTERVAL 1 YEAR',
    'all' => null,
];
$range    = isset($_GET['range']) && array_key_exists($_GET['range'], $range_map) ? $_GET['range'] : '3m';
$interval = $range_map[$range];

switch ($method) {
    case 'GET':
        if ($interval !== null) {
            $sql = "SELECT distanza, volume_residuo, data_misurazione
                    FROM wpl
                    WHERE data_misurazione >= (NOW() - $interval)
                    ORDER BY data_misurazione DESC";
        } else {
            $sql = "SELECT distanza, volume_residuo, data_misurazione
                    FROM wpl
                    ORDER BY data_misurazione DESC";
        }
        break;
    default:
        http_response_code(405);
        die('Method Not Allowed');
}

$link   = Connection();
$result = mysqli_query($link, $sql);

if (!$result) {
    http_response_code(500);
    die(mysqli_error($link));
}

header('Content-Type: application/json');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: DENY');

echo '[';
for ($i = 0; $i < mysqli_num_rows($result); $i++) {
    echo ($i > 0 ? ',' : '') . json_encode(mysqli_fetch_object($result));
}
echo ']';

mysqli_close($link);
