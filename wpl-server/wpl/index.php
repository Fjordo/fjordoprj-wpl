<?php

	include("connect.php"); 	
	
	$link=Connection();

	$result=mysqli_query($link, "SELECT distanza, volume_residuo, data_misurazione FROM wpl ORDER BY data_misurazione DESC");
?>

<html>
   <head>
      <title>Sensor Data</title>
		<script src="https://code.highcharts.com/highcharts.js"></script>
		<script src="https://code.highcharts.com/modules/series-label.js"></script>
		<script src="https://code.highcharts.com/modules/exporting.js"></script>
		<script src="https://code.highcharts.com/modules/export-data.js"></script>
   </head>
<body>
   <h1>Water Level</h1>

   <!-- <table border="1" cellspacing="1" cellpadding="1">
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

   </table> -->

	<div id="container" style="width:100%; height:400px;"></div>
	<script>
		document.addEventListener('DOMContentLoaded', function () {

			var queryData = "<?php echo $result ?>";
			console.log(queryData);

			var myChart = Highcharts.chart('container', {
				title: {
					text: 'Livello Acqua Cisterna'
				},

				subtitle: {
					text: 'sottotitolo'
				},

				yAxis: {
					title: {
							text: 'cm misurati'
					}
				},
				legend: {
					layout: 'vertical',
					align: 'right',
					verticalAlign: 'middle'
				},

				plotOptions: {
					series: {
							label: {
								connectorAllowed: false
							},
							pointStart: 2019
					}
				},

				series: [{
					name: 'distanza',
					data: [43934, 52503, 57177, 69658, 97031, 119931, 137133, 154175]
				}],

				responsive: {
					rules: [{
							condition: {
								maxWidth: 500
							},
							chartOptions: {
								legend: {
									layout: 'horizontal',
									align: 'center',
									verticalAlign: 'bottom'
								}
							}
					}]
				}
			});
		});
	</script>
</body>
</html>