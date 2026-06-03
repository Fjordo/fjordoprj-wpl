<?php

function Connection() {
    $server = getenv('DB_HOST');
    $user   = getenv('DB_USER');
    $pass   = getenv('DB_PASS');
    $db     = getenv('DB_NAME');

    $connection = mysqli_connect($server, $user, $pass, $db);

    if (!$connection) {
        die('MySQL ERROR: ' . mysqli_connect_error());
    }

    mysqli_set_charset($connection, 'utf8mb4');
    return $connection;
}
