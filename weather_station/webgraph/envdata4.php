<!doctype html>
<html>
	<head>
		<title>Environment Data</title>
		<script src="./Chart.js"></script>
		<meta name = "viewport" content = "initial-scale = 1, user-scalable = no">
		<style>
			canvas{
				float: center;
			}
		</style>
	</head>
	<body>
<?php
try {
	/*** connect to SQLite database ***/
	$dbh = new PDO("sqlite:/media/data/termal.db3");

}
catch(PDOException $e)
{
	echo $e->getMessage();
}

$hlabels=array( "00:00","01:00","02:00","03:00","04:00","05:00","06:00","07:00","08:00","09:00","10:00","11:00",
                "12:00","13:00","14:00","15:00","16:00","17:00","18:00","19:00","20:00","21:00","22:00","23:00","00:00");

$hlabels_len=count($hlabels);

$labels=array();

$today = date("Y-m-d");
$ora=date("H");
$ora_bis=$ora;

$dati_prs="";
$dati_hum="";
$dati_temp="";

while( $ora<=23) {
        $sql = "SELECT avg(pressure)/100 AS prs, avg(temperature) AS temp, avg(humidity) AS hum FROM samples WHERE date >= date( 'now','-1 day') AND time LIKE '".sprintf("%02d", $ora)."%' ORDER BY time;";
        foreach ($dbh->query($sql) as $row) {
                if ( $row['prs'] <> "") {
                        #
                        if ( $dati_prs == "") {
                                $dati_prs = $row['prs'];
                                $dati_hum = $row['hum'];
                                $dati_temp = $row['temp'];
                        } else {
                                $dati_prs = $dati_prs . "," . $row['prs'];
                                $dati_hum = $dati_hum . "," . $row['hum'];
                                $dati_temp = $dati_temp . "," . $row['temp'];
                        }
                } else {
                        if ( $dati_prs == "") {
                                $dati_prs = 0;
                                $dati_hum = 0;
                                $dati_temp = 0;
			} else {
                        	$dati_prs = $dati_prs . ", 0";
                        	$dati_hum = $dati_hum . ", 0";
                        	$dati_temp = $dati_temp . ", 0";
			}
                }
        }
        $ora++;
}

$ora=0;

while( $ora<=$ora_bis) {
        $sql = "SELECT avg(pressure)/100 AS prs, avg(temperature) AS temp, avg(humidity) AS hum FROM samples WHERE date >= date('now') AND time LIKE '".sprintf("%02d", $ora)."%' ORDER BY time;";
        foreach ($dbh->query($sql) as $row) {
                if ( $row['prs'] <> "") {
                        #
                        if ( $dati_prs == "") {
                                $dati_prs = $row['prs'];
                                $dati_hum = $row['hum'];
                                $dati_temp = $row['temp'];
                        } else {
                                $dati_prs = $dati_prs . "," . $row['prs'];
                                $dati_hum = $dati_hum . "," . $row['hum'];
                                $dati_temp = $dati_temp . "," . $row['temp'];
                        }
                } else {
                        if ( $dati_prs == "") {
                                $dati_prs = 0;
                                $dati_hum = 0;
                                $dati_temp = 0;
                        } else {
                                $dati_prs = $dati_prs . ", 0";
                                $dati_hum = $dati_hum . ", 0";
                                $dati_temp = $dati_temp . ", 0";
                        }
                }
        }
        $ora++;
}

$dbh = null;
$dati_prs = "[".$dati_prs."]";
$dati_hum = "[".$dati_hum."]";
$dati_temp = "[".$dati_temp."]";

$labels="";

for($x=0;$x<$hlabels_len;$x++) {
  if ( $x <> 0) {
    $labels = $labels.",\"".$hlabels[ $ora_bis]."\"";
  } else {
    $labels = "[ \"".$hlabels[ $ora_bis]."\"";
  }
  $ora_bis=$ora_bis+1;
  $ora_bis = $ora_bis % 24;
}
$labels = $labels." ],\n";

?>

<table><tr>
<td>
	<canvas id="temperature" width="512" height="384" ></canvas>
</td><td>
	<canvas id="pressure" width="512" height="384" ></canvas>
</td></tr>
</table>


	<script>

		var lineChartData = {
<?php
		echo "labels : ".$labels;
?>
			datasets : [
				{
					fillColor : "rgba(151,187,205,0.5)",
					strokeColor : "rgba(151,187,205,1)",
					pointColor : "rgba(151,187,205,1)",
					pointStrokeColor : "#fff",
<?php
		echo "data : ".$dati_hum;
?>
				},
				{
					fillColor : "rgba(151,187,205,0.5)",
					strokeColor : "rgba(151,187,205,1)",
					pointColor : "rgba(151,187,205,1)",
					pointStrokeColor : "#fff",
<?php
		echo "data : ".$dati_temp;
?>
				}
			]
			
		};
                var lineChartPressureData = {
<?php
		echo "labels : ".$labels;
?>
                        datasets : [
                                {
                                        fillColor : "rgba(151,187,205,0.5)",
                                        strokeColor : "rgba(151,187,205,1)",
                                        pointColor : "rgba(151,187,205,1)",
                                        pointStrokeColor : "#fff",
<?php
		echo "data : ".$dati_prs;
?>
                                }
                        ]

                }

	new Chart(document.getElementById("temperature").getContext("2d")).Line(lineChartData);
	new Chart(document.getElementById("pressure").getContext("2d")).Line(lineChartPressureData);

	</script>
	</body>
</html>
