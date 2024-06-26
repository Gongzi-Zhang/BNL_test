var dateClick = '';
var runClick = '';

function zoom(ele)
{
    ele.classList.toggle("zoom");
}

function changeDateBgColor(date)
{
    if (date)
    {
	if (dateClick && document.getElementById(dateClick))
	{
	    document.getElementById(dateClick).style.background = '#F5F5DC';
	}
	document.getElementById(date).style.background = '#778899';
	dateClick = date;
    }
}

function changeRunBgColor(run)
{
    if (run)
    {
	if (runClick && document.getElementById(runClick))
	{
	    document.getElementById(runClick).style.background = '#8FBC8F';
	}
	document.getElementById(run).style.background = '#778899';
	runClick = run;
    }
}

function launchDate(date = '')
{
    if (!date)
	date = document.getElementById('datenav').title;

    changeDateBgColor(date);

    var page = document.getElementById('self').text;
    var func = '';
    if (page === 'HOME')
	func = 'getDateRunInfo';
    else if (page === 'QA')
	func = 'getDateRuns';
    else if (page === 'PTRG')
	func = 'getDatePtrgs';
    else if (page === 'CHANNEL')
	func = 'getDateRuns';

    var url = "api.php?func=" + func + "&date=" + date;

    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() {
	if (this.readyState == 4 && this.status == 200) {
	    if (page == 'HOME')
		document.getElementById("content").innerHTML = this.responseText;
	    else
	    {
		document.getElementById("runs").innerHTML = this.responseText;
		var run = null;
		if (document.getElementById('runs').hasAttribute('title'))
		{
		    var ele = document.getElementById('runs');
		    run = ele.title;
		    ele.removeAttribute('title');
		}
		else if (document.querySelectorAll('#runs a').length > 0)
		    run = document.getElementById('runs').lastElementChild.innerText;
		launchRun(run);
	    }
	}
    };

    xmlHttp.open("GET", url, true);
    xmlHttp.send();
}

function launchRun(run)
{
    var page = document.getElementById('self').text;
    var func = '';
    var id = '';
    if (page === 'QA')
    {
	func = 'getRunQA';
	id = 'QA-plots';
    }
    else if (page === 'PTRG')
    {
	func = 'getRunPtrg';
	id = 'ptrg-plots';
    }
    else if (page === 'CHANNEL')
    {
	func = 'getRunChannel';
	id = 'channel-plots';
    }

    if (!run)
    {
	document.getElementById(id).innerText = "";
	return;
    }

    changeRunBgColor(run);

    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById(id).innerHTML = this.responseText;
      }
    };
    var url = "api.php?func=" + func + "&run=" + run;

    xmlHttp.open("GET", url, true);
    xmlHttp.send();
}

function getRunsInfo(evt)
{
    evt.preventDefault();

    var formEle = document.forms.runNumbers;
    var form = new FormData(formEle);
    var runs = form.get('runs');

    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() {
	if (this.readyState == 4 && this.status == 200) {
	    document.getElementById("content").innerHTML = this.responseText;
	}
    };

    var url = "api.php?func=getRunsInfo&runs=" + runs;
    xmlHttp.open("GET", url, true);
    xmlHttp.send();
}
