<?php

include_once "libdb.php";
include_once "cali.php";

$date = date('Y-m-d', time());
if ($_SERVER['REQUEST_METHOD'] == 'GET' && isset($_GET['date']))
    $date = $_GET['date'];

$fields = array('Run', 'Type', 'Flag', 'StartTime', 'StopTime', 'Length', 'Channels', 'Trigger', 'Events', 'PedRun');
$cdb->setReverse();
$res = $cdb->getRunsInfo("date(StartTime) = '{$date}'", implode(',', $fields));
$res = $cdb->tabularize($res);
if ($res)
    echo $res;
else
    echo "<b> No run on {$date} </b>";
?>
