<?php
declare(strict_types=1);

function Connection(): \mysqli {
    $server = (string)(getenv('DB_HOST') ?: '');
    $user   = (string)(getenv('DB_USER') ?: '');
    $pass   = (string)(getenv('DB_PASS') ?: '');
    $db     = (string)(getenv('DB_NAME') ?: '');

    $connection = mysqli_connect($server, $user, $pass, $db);

    if (!$connection) {
        die('MySQL ERROR: ' . mysqli_connect_error());
    }

    mysqli_set_charset($connection, 'utf8mb4');
    return $connection;
}
