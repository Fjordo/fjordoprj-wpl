<?php

	include("connect.php"); 	
	
	$link=Connection();

	$result=mysqli_query($link, "SELECT * FROM wpl ORDER BY data_misurazione DESC");
?>

<html>
   <head>
      <title>Sensor Data</title>
   </head>
<body>
   <h1>Water Level</h1>

   <table border="1" cellspacing="1" cellpadding="1">
		<tr>
			<td>&nbsp;Measured Distance (cm)&nbsp;</td>
			<td>&nbsp;Residual Volume (m^3)&nbsp;</td>
			<td>&nbsp;Timestamp &nbsp;</td>
		</tr>

      <?php
      
		  if($result!==FALSE){
		     while($row = mysqli_fetch_array($result)) {
		        printf("<tr><td> &nbsp;%s </td><td> &nbsp;%s&nbsp; </td><td> &nbsp;%s&nbsp; </td></tr>", 
		           $row["distanza"], $row["volume_residuo"], $row["data_misurazione"]);
		     }
		     mysqli_free_result($result);
		     mysqli_close();
		  }
      ?>

   </table>
</body>
</html>