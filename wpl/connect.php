<?php

	function Connection(){
		$server="my_ip_address";
		$user="my_username";
		$pass="my_password";
		$db="my_fjordoprj";
	   	
		$connection = mysqli_connect($server, $user, $pass, $db);

		if (!$connection) {
	    	die('MySQL ERROR: ' . mysql_error());
		}

		return $connection;
	}
?>
