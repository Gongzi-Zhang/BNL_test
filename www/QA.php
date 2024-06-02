<!DOCTYPE html>
<html>
    <head>
        <title>CALI: QA</title>
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
		echo $chtml->topNav('QA');
	    ?>
	</div>

	<div id="leftnav">
	    <?php
		$date = date('Y-m-d', time());
		if (isset($_GET['date']))
		    $date = $_GET['date'];

		$Year = (int)explode('-', $date)[0];
		$out = "<div id='datenav' title='{$date}'>";
		$out .= $chtml->dateNav($Year);
		$out .= "</div>";
		echo $out;
	    ?>
	</div>

	<div id='content'>
	    <?php
		$run = '';
		if (isset($_GET['run']))
		    $run = $_GET['run'];

		if ($run)
		    echo "<div id='runs' title='$run'>";
		else
		    echo "<div id='runs'>";
	    ?>
	    </div>
	    <div id='QA-plots'>
	    </div>
	</div>
    </body>
</html>
