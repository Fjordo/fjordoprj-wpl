<?php
   	include("connect.php");
   	
   	$link=Connection();

	$distanza=$_POST["dist"];
	$volume=$_POST["vol"];

	$query = "INSERT INTO 'wpl' ('distanza', 'volume_residuo', 'data_misurazione') 
		VALUES ('".$distanza."','".$volume."', time())"; 
   	
   	mysqli_query($query,$link);
	mysqli_close($link);

   	header("Location: index.php");
?>
