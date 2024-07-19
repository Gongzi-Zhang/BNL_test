<!DOCTYPE html>
<html>
    <head>
        <title>CALI: MISC</title>
	<link rel="stylesheet" type="text/css" href="cali.css">
	<script type="text/javascript" src="cali.js"> 
	</script>
    </head>
    <body>
	<?php 
	    include_once "libdb.php";
	    include_once "cali.php";
	    include_once "api.php";
	?>
	<div id="topnav">
	    <?php 
		echo $chtml->topNav('MISC');
	    ?>
	</div>

	<div id='leftnav'>
	    <div style='text-align: center;background-color: #F5F5DC;'>
		<?php
		    $out = '';
		    $imgs = array('IV');
		    foreach ($imgs as $img)
			$out .= "<p id='$img'> <a href='#' onclick=\"launchMisc('${img}.png')\"> $img </a> </p>";
		    echo $out;
		?>
	    </div>
	</div>

	<div id='content'>
	    <div id='misc-plots'>
		<img src='figures/misc/IV.png' alt='IV plot does not exist' onclick='zoom(this);' />
	    </div>
	</div>
    </body>
</html>
