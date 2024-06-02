<!DOCTYPE html>
<html>
    <head>
        <title>CALI: HOME</title>
	<link rel="stylesheet" type="text/css" href="cali.css">
	<script type="text/javascript" src="cali.js"> 
	</script>
    </head>
    <body onload='launchDate()'>
	<?php 
	    include_once "libdb.php";
	    include_once "cali.php";
	    include_once "api.php";
	?>
	<div id="topnav">
	    <?php 
		echo $chtml->topNav('HOME');
	    ?>
	</div>

	<div id='leftnav'>
	    <?php
		echo $chtml->searchRuns();

		$date = date('Y-m-d', time());
		if (isset($_GET['date']))
		    $date = $_GET['date'];
		$Year = (int)explode('-', $date)[0];

		$out .= "<div id='datenav' title='{$date}')>";
		$out .= $chtml->dateNav($Year);
		$out .= "</div>";
		echo $out;
	    ?>
	</div>

	<div id='content'>
	</div>
    </body>
</html>
