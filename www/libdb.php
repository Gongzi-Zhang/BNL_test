<?php 
class _caliDB
{
    protected static $dbName = "BNL_test.db";
    protected static $table = "runs";
    protected static $db;
    protected static $fieldTitle = array(
	'Length' => 'Len (m)',
	'Channels' => '#Ch',
	'Size'	 => 'Size (MB)',
	'T1'	 => 'T1 (V)',
	'T2'	 => 'T2 (V)',
	'T3'	 => 'T3 (V)',
	'T4'	 => 'T4 (V)',
    );
    protected static $trigger = array(
	1   => 'T1',	16 => 'T1',
	2   => 'T2',	32 => 'T2',
	4   => 'T3',	64 => 'T3',
	8   => 'T4',   128 => 'T4',
	3   => 'T1 && T2',
	6   => 'T2 && T3',
	7   => 'T1 && T2 && T3',
	12  => 'T3 && T4',
	28  => 'T1 || (T3 && T4)',
	60  => 'T1 || T2 || (T3 && T4)',
	67  => '(T1 && T2) || T3',
	112 => 'T1 || T2 || T3',
	131 => '(T1 && T2) || T4',
	240 => 'T1 && T2 && T3 && T4',
    );
    protected $reverse = false;

    function __construct() 
    {
	self::$db = new SQLite3(self::$dbName);
    }

    function setReverse($val = true)
    {
	$this->reverse = $val;
    }

    function unsetReverse()
    {
	$this->reverse = false;
    }

    function getRunsInfo($cond, $fields = 'Run')
    {
	$table = self::$table;
	$order = '';
	if ($this->reverse)
	    $order = 'ORDER BY run DESC';
	$sql = "SELECT {$fields} FROM {$table} WHERE {$cond} {$order};";
	return self::$db->query($sql);
    }

    function getRunInfo($run, $fields = '*')
    {
	return $this->getRunsInfo("Run = {$run}", $fields);
    }

    function getRuns($cond)
    {
	$res = $this->getRunsInfo($cond);
	$runs = array();
	while ($row = $res->fetchArray())
	    array_push($runs, $row['Run']);
	return $runs;
    }

    function getLatestRunNumber()
    {
	$table = self::$table;
	$sql = "SELECT Run FROM {$table} ORDER BY Run DESC LIMIT 1;";
	$res = self::$db->query($sql);
	return $res->fetchArray()["Run"];
    }

    function getLatestRuns($nRuns, $runTypes = 'data,cosmic')
    {
	$table = self::$table;
	$ta = explode(',', $runTypes);
	$type = trim($ta[0]);
	$types = "'$type'";
	for ($i=1; $i<count($ta); $i++)
	{
	    $type = trim($ta[i]);
	    $types .= ',' . "'$type'";
	}
	$sql = "SELECT Run FROM {$table} WHERE Type IN ({$types}) ORDER BY Run DESC LIMIT {$nRuns};";
	$res = self::$db->query($sql);
	if (!$res)
	{
	    echo "<p> failed to execute the sql </p>";
	    echo "<p> <b> $sql </b> </p>";
	    return;
	}
	return $res;
    }

    function getValue($field, $value)
    {
	$ret = $value;
	if ($field == 'col')
	{
	    $col = $value;
	    if (array_key_exists($col, self::$fieldTitle))
		$col = self::$fieldTitle[$col];
	    $ret = $col;
	}
	else if ($field == 'Run')
	{
	    $run = $value;
	    $res = $this->getRunInfo($run, "Type,StartTime");
	    $row = $res->fetchArray();
	    $type = $row['Type'];
	    $date = explode(' ', $row['StartTime'])[0];
	    $page = '';
	    if ($type == 'data' || $type == 'cosmic')
		$page = "QA.php?date={$date}&run={$run}";
	    else if ($type == 'ptrg')
		$page = "ptrg.php?date={$date}&run={$run}";

	    $ret = "<a href='$page'> $run </a>";
	}
	else if ($field == 'Trigger')
	{
	    $trg = $value;
	    if (array_key_exists($trg, self::$trigger))
		$trg = self::$trigger[$trg];
	    $ret = $trg;
	}
	return $ret;
    }

    // horizontal table
    public function tabularize($res)
    {
	// check query result not empty
	if ( ! $res->fetchArray() )
	{
	    return false;
	}

	$res->reset();
	$cols = array();
	$table = '<table class="tab-db">';
	$table .= "<tr>";
	for ($i=0; $i<$res->numColumns(); $i++)
	{
	    $col = $res->columnName($i);
	    array_push($cols, $col);
	    $table .= "<th> {$this->getValue('col', $col)} </th>";
	}
	$table .= "</tr>";
	while ($row = $res->fetchArray())
	{
	    $table .= "<tr>";
	    for ($i=0; $i<$res->numColumns(); $i++)
	    {
		$col = $cols[$i];
		$value = $row[$col];
		$value = $this->getValue($col, $value);
		$table .= "<td>" . $value . "</td>";
	    }
	    $table .= "</tr>";
	}
	$table .= '</table>';
	return $table;
    }

    // vertical table
    public function tabularize_v($res)
    {
	// check query result not empty
	if ( ! $res->fetchArray() )
	    return false;

	$table = '<table class="tab-db-v">';
	for ($i=0; $i<$res->numColumns(); $i++)
	{
	    $col = $res->columnName($i);

	    $table .= "<tr>";
	    $table .= "<th> {$this->getValue('col', $col)} </th>";

	    $res->reset();
	    while ($row = $res->fetchArray())
	    {
		$value = $row[$col];
		$value = $this->getValue($col, $value);
		$table .= "<td>" . $value . "</td>";
	    }
	    $table .= "</tr>";
	}
	$table .= '</table>';
	return $table;
    }
}

$cdb = new _caliDB();
?>
