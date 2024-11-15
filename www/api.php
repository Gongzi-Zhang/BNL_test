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

    if ($func == 'getDateRunInfo')
        echo getDateRunInfo($date);
    else if ($func == 'getDateRuns')
        echo getDateRuns($date);
    else if ($func == 'getDatePtrgs')
        echo getDatePtrgs($date);
    else if ($func == 'getDateMips')
        echo getDateMips($date);
    else if ($func == 'getRunQA')
	echo getRunQA($run);
    else if ($func == 'getRunChannel')
	echo getRunChannel($run);
    else if ($func == 'getRunPtrg')
	echo getRunPtrg($run);
    else if ($func == 'getRunMip')
	echo getRunMip($run);
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

function getDateRunInfo($date)
{
    $fields = array('Run', 'Type', 'Flag', 'StartTime', 'StopTime', 'Length', 'Channels', 'Trigger', 'T1', 'T2', 'T3', 'Events', 'PedRun', 'LG', 'HG', 'Vbias');
    global $cdb;
    // $cdb->setReverse();
    $res = $cdb->getRunsInfo("date(StartTime) = '{$date}'", implode(',', $fields));
    $res = $cdb->tabularize($res);
    if (!$res)
        $res = "<h2> No run on {$date} </h2>";
    return $res;
}

function getDateRuns($date)
{
    global $cdb;
    $runs = $cdb->getRuns("date(StartTime) = '{$date}' AND Type in ('data', 'cmdata') AND Flag = 'good'");
    if (!$runs)
	return "<h2> No data run on {$date} </h2>";

    $res = 'Run: ';
    foreach ($runs as $run )
    {
	$res .= "<a id='$run' href='#' onclick='launchRun({$run})'> {$run} </a>";
    }

    return $res;
}

function getDateMips($date)
{
    global $cdb;
    $runs = $cdb->getRuns("date(StartTime) = '{$date}' AND Type = 'mip' AND Flag = 'good'");
    if (!$runs)
	return "<h2> No MIP run on {$date} </h2>";

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
    $imgs = array("event_rate", "hit_xy", "hit_mul", 
	"hit_MIP", "event_MIP", "event_MIP_vs_hit_mul",
	"event_x", "event_y", "event_z");
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
	$res .= "<a id='$run' href='#' onclick='launchRun({$run})'> {$run} </a>";

    return $res;
}

function getRunChannel($run)
{
    global $cali;

    $table = '<table CELLSPACING=0>';
    $imgs = array("HG_vs_LG");
    foreach ($imgs as $img)
    {
	$table .= '<tr> <td>';
	$table .= "<a href='#' onclick='launchChannelPlot(\"figures/$run/${img}.png\")'> $img </a>";
	$table .= '</td> </tr>';
    }

    $imgs = array("ch_e");
    foreach ($imgs as $img)
    {
	$table .= "<tr> <td>";
	$table .= "<b> ${img} </b>";
	foreach ($cali::gains as $gain)
	    $table .= "<a href='#' onclick='launchChannelPlot(\"figures/$run/${gain}_${img}.png\")'> $gain </a>";
	$table .= "</td> </tr>";
    }
    $table .= '</table>';

    return $table;
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

function getRunMip($run)
{
    global $cali;
    $res = '';
    $imgs = array("MIP_HG");
    foreach ($imgs as $img)
	$res .= "<img src='figures/$run/{$img}.png' alt='{$img} does not exist' onclick='zoom(this);' />";

    return $res;
}
?>
