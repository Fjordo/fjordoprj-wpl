<?php

	function Connection(){
		$server="localhost";
		$user="fjordoprj";
		$pass="";
		$db="my_fjordoprj";
	   	
		$connection = mysqli_connect($server, $user, $pass, $db);

		if (!$connection) {
	    	die('MySQL ERROR: ' . mysql_error());
		}

		return $connection;
	}
?>
