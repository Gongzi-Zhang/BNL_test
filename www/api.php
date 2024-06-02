<?php

include_once "libdb.php";
include_once "cali.php";

if ($_SERVER['REQUEST_METHOD'] == 'GET' && isset($_GET['func']))
{
    $func = $_GET['func'];
    $date = '';
    if (isset($_GET['date']))
        $date = $_GET['date'];
    $run = '';
    if (isset($_GET['run']))
	$run = $_GET['run'];
    $runs = '';
    if (isset($_GET['runs']))
	$runs = $_GET['runs'];

    if ($func == 'getDateRuns')
        echo getDateRuns($date);
    else if ($func == 'getDatePtrgs')
        echo getDatePtrgs($date);
    else if ($func == 'getDateQAs')
        echo getDateQAs($date, $run);
    else if ($func == 'getRunQA')
	echo getRunQA($run);
    else if ($func == 'getRunPtrg')
	echo getRunPtrg($run);
    else if ($func == 'getRunsInfo')
    {
	if (!$runs)
	    $runs = $cdb->getLatestRunNumber();
	$cond = 'Run in (' . $runs . ')';
	$res = $cdb->tabularize_v($cdb->getRunsInfo($cond, '*'));
	if (!$res)
	    $res = "<h2> No runs: {$run} in DB </h2>";
	echo $res;
    }
    else
	echo "<h2> Unknown function {$func} </h2>";
}

function getDateRuns($date)
{
    $fields = array('Run', 'Type', 'Flag', 'StartTime', 'StopTime', 'Length', 'Channels', 'Trigger', 'Events', 'PedRun');
    global $cdb;
    // $cdb->setReverse();
    $res = $cdb->getRunsInfo("date(StartTime) = '{$date}'", implode(',', $fields));
    $res = $cdb->tabularize($res);
    if (!$res)
        $res = "<h2> No run on {$date} </h2>";
    return $res;
}

function getDateQAs($date)
{
    global $cdb;
    $runs = $cdb->getRuns("date(StartTime) = '{$date}' AND Type in ('data', 'cosmic') AND Flag = 'good'");
    if (!$runs)
	return "<h2> No data/cosmic run on {$date} </h2>";

    $res = 'Run: ';
    foreach ($runs as $run )
    {
	$res .= "<a id='$run' href='#' onclick='launchRun({$run})'> {$run} </a>";
    }

    return $res;
}

function getRunQA($run)
{
    global $cali;
    $res = '';
    $imgs = array("event_rate", "hit_xy", "event_mul", "event_mul1", "event_mul2",
		  "hit_e", "event_e", "event_x", "event_y", "event_z");
    foreach ($cali::gains as $gain)
	foreach ($imgs as $img)
	    $res .= "<img src='figures/$run/{$gain}_{$img}.png' alt='{$gain}_{$img} does not exist' onclick='zoom(this);'/>";

    return $res;
}

function getDatePtrgs($date)
{
    global $cdb;
    $runs = $cdb->getRuns("date(StartTime) = '{$date}' AND Type = 'ptrg' AND Flag = 'good'");
    if (!$runs)
	return "<h2> No ptrg run on {$date} </h2>";

    $res = 'Run: ';
    foreach ($runs as $run )
    {
	foreach ($runs as $run )
	    $res .= "<a id='$run' href='#' onclick='launchRun({$run})'> {$run} </a>";
    }

    return $res;
}

function getRunPtrg($run)
{
    global $cali;
    $res = '';
    $imgs = array("ped", "ped_rms");
    foreach ($cali::gains as $gain)
    {
	foreach ($imgs as $img)
	    $res .= "<img src='figures/$run/{$gain}_{$img}.png' alt='{$gain}_{$img} does not exist' onclick='zoom(this);' />";
    }

    return $res;
}
?>
