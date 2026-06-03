<?php
include("connect.php");

$link    = Connection();
$see_all = isset($_GET['all']) && $_GET['all'] === 'true';

if ($see_all) {
    $result = mysqli_query($link, "SELECT id, distanza, volume_residuo, data_misurazione FROM wpl ORDER BY data_misurazione DESC");
} else {
    $result = mysqli_query($link, "SELECT id, distanza, volume_residuo, data_misurazione FROM wpl WHERE data_misurazione >= (NOW() - INTERVAL 3 MONTH) ORDER BY data_misurazione DESC");
}
?>
<html>
<head>
    <title>Sensor Data</title>
</head>
<body>
    <h1>Water Level</h1>

    <?php if ($see_all): ?>
        <span><b>All records</b></span>
    <?php else: ?>
        <div>
            <span><b>Last 3 months</b></span>
            &nbsp;&nbsp;
            <a href="index.php?all=true">See All</a>
        </div>
    <?php endif; ?>

    <br>

    <table border="1" cellspacing="1" cellpadding="1">
        <tr>
            <td>&nbsp;Measured Distance (cm)&nbsp;</td>
            <td>&nbsp;Residual Volume (dm³)&nbsp;</td>
            <td>&nbsp;Timestamp&nbsp;</td>
        </tr>
        <?php if ($result !== false): ?>
            <?php while ($row = mysqli_fetch_array($result)): ?>
                <tr>
                    <td>&nbsp;<?php echo htmlspecialchars($row['distanza']); ?>&nbsp;</td>
                    <td>&nbsp;<?php echo htmlspecialchars($row['volume_residuo']); ?>&nbsp;</td>
                    <td>&nbsp;<?php echo htmlspecialchars($row['data_misurazione']); ?>&nbsp;</td>
                </tr>
            <?php endwhile; ?>
            <?php mysqli_free_result($result); ?>
        <?php endif; ?>
    </table>
</body>
</html>
<?php mysqli_close($link); ?>
