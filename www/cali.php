<?php 
$monthName = array(
    1 => 'January',
    2 => 'February',
    3 => 'March',
    4 => 'April',
    5 => 'May',
    6 => 'June',
    7 => 'July',
    8 => 'August',
    9 => 'September',
    10 => 'Octorber',
    11 => 'November',
    12 => 'December',
);

class _cali
{
    const startTime = '20240401';
    const gains = array('LG', 'HG');
}

class _caliHtml
{
    protected static $pages = array(
	'HOME' => 'index.php',
	'QA'   => 'QA.php',
	'PTRG' => 'ptrg.php',
	// 'DB'   => 'db.php'
    );

    public function topNav($page)
    {
	$out = '<div style="padding-left: 30px;padding-right: 50px"> CALI </div>';
	foreach (self::$pages as $x => $y)
	{
	    if ($x == $page)
		$out .= "<a href={$y} id='self'>$x</a>";
	    else
		$out .= "<a href={$y}>$x</a>";
	}

	return $out;
    }

    public function dateNav($Year)
    {
	if ($Year < 2024 || $Year > (int)date('Y'))
	    return "<p> Not a valid date: {$date} </p>";
	$month = (int)date('m');

	/* // for 2025 update
	$res .= '<div>';
	$res .= '<center>';
	for ($y=2024; $y<=$Year; $y++)
	    $res .= "<a href=ptrg.php?year={$y}> $y </a>";
	$res .= '</center>';
	$res .= '</div>';
	*/
	// while (($Year > 2024 && $month > 0) || ($Year == 2024 && $month >= 4))
	$res .= "<h4> Select Day </h4>";
	while ($month >= 4)
	{
	    $res .= '&nbsp;' . $GLOBALS['monthName'][$month] . ', ' . $Year;
	    $res .= $this->getDays($Year, $month);
	    $month--;
	}
	return $res;
    }

    public function getDays($Year, $month)
    {
	$numDays = cal_days_in_month(CAL_GREGORIAN, $month, $Year);
	$firstDay = date('w', strtotime("{$Year}-{$month}-1"));

	$table = '<div class="calendar">';
	$table .= '<table CELLSPACING=0>';
	$table .= '<tr>';

	$wd = 0;
	for ($i=0; $i<$firstDay; $i++)
	{
	    $table .= '<td></td>';
	    $wd++;
	}

	for ($i=1; $i<=$numDays; $i++)
	{
	    $dt = sprintf('%4d-%02d-%02d', $Year, $month, $i);
	    $table .= "<td class='date' id='{$dt}'> <a href='#' onclick='launchDate(\"{$dt}\")'> $i </a> </td>";
	    $wd++;
	    if ($wd % 7 == 0)
	    {
		$table .= '</tr>';
		$table .= '<tr>';
		$wd = 0;
	    }
	}

	while ($wd < 7)
	{
	    $table .= '<td></td>';
	    $wd++;
	}

	$table .= '</tr>';
	$table .= '</table>';
	$table .= '</div>';
	return $table;
    }

    public function searchRuns()
    {
	$res .= "<h4> Select Run </h4>";
	$res .= "<div>";
	$res .=	    "<form id='runNumbers' onsubmit='getRunsInfo(event)'>";
	$res .=		"<input name='runs' type='text' placeholder='run1 [, run2...]'>";
	$res .=		"<input type='submit' hidden />";
	$res .=	    "</form>";
	$res .= "</div>";
	// $res .= "<h4> Advanced Search </h4>";
	// $res .= "<div id='advanced-search'>";
	// $res .=	    "<form>";
	// $res .=     	"<input name='condition' type='text' placeholder='sql condition'>";
	// $res .=     	"<input type='submit' hidden />";
	// $res .=	    "</form>";
	// $res .= "</div>";
        return $res;
    }
}

$cali = new _cali();
$chtml = new _caliHtml();
?>
