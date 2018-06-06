<?php
/*
	sniproxy.priv.inc
*/
global $priv_list;
$priv_list['page-service-sniproxy'] = array();
$priv_list['page-service-sniproxy']['name'] = "WebCfg - Services: Sniproxy package";
$priv_list['page-service-sniproxy']['descr'] = "Allow access to sniproxy package GUI";
$priv_list['page-service-sniproxy']['match'] = array();
$priv_list['page-service-sniproxy']['match'][] = "pkg_edit.php?xml=sniproxy.xml*";
?>
