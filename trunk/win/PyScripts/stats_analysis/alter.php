<?php

require '/home/omexca2/bumptop_admin_mysql_config.php'; // this file sets the variables used to connect

$connect = mysql_connect($mysql_server, $mysql_user, $mysql_password) or die(mysql_error());

$query_create_table = "ALTER TABLE invite_codes ADD COLUMN (sent_time DATETIME, first_auth_time DATETIME) AFTER md5_hashes";
$result_create_table = mysql_query($query_create_table) or die(mysql_error());

?>