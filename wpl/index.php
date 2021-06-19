<?php

	include("connect.php");

	$link=Connection();
	$see_all = $_GET['all'] == true ? true : false;

	if ($see_all) {
		$result=mysqli_query($link, "SELECT id,distanza,volume_residuo,data_misurazione FROM wpl ORDER BY data_misurazione DESC");
	}
	else {
		$result=mysqli_query($link, "SELECT id,distanza,volume_residuo,data_misurazione FROM wpl WHERE data_misurazione <= (NOW() - INTERVAL 3 MONTH) ORDER BY data_misurazione DESC");
	}
?>

<html>
   <head>
      <title>Sensor Data</title>
   </head>
<body>
   <h1>Water Level</h1>

   	<?php
	if ($see_all)
		echo '<span><b>All records</b></span>';
	else
		echo '
			<div>
			<span><b>Last 3 months</b></span>
			&nbsp;&nbsp;
			<a href="https://fjordoprj.altervista.org/wpl/index.php?all=true">See All</a>
			</div>
		';
   	?>

	<br>

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
			}
			mysqli_close();
      ?>

   </table>
</body>
</html>