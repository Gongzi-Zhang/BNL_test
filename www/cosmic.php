<!DOCTYPE html>
<html>
    <head>
        <title>CALI: COSMIC </title>
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
		echo $chtml->topNav('COSMIC');
	    ?>
	</div>

	<div id="leftnav" >
	    <?php
		$res = $cdb->query("SELECT DISTINCT date(StartTime) AS Date From runs WHERE Type = 'cosmic' AND Flag = 'good' ORDER by Date DESC");
		$out = "<div id='datenav' style='text-align: center;background-color: #F5F5DC;'>";
		while ($row = $res->fetchArray())
		{
		    $date = $row['Date'];
		    $out .= "<p id='$date'> <a href='#' onclick=\"launchDate('$date')\"> $date </a> </p>";
		}
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
	    <div id='cosmic-plots'>
	    </div>
	</div>
    </body>
</html>
