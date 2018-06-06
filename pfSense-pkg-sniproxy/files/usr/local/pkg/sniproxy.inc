<?php

require_once('config.inc');
require_once('functions.inc');
require_once('interfaces.inc');
require_once('globals.inc');
require_once('services.inc');
require_once('service-utils.inc');
require_once('util.inc');

$shortcut_section = "sniproxy";

define('CONFIG_BASE', '/var/etc/');
define('RC_CONF_BASE', '/usr/local/etc/rc.conf.d');

function sync_sniproxy_conf(){
    global $config;
    
    if (!isset($config['installedpackages']['sniproxy']['config'][0]['enable'])) {
            return 1;
    }

    if (isset($config['installedpackages']['sniproxy']['config'][0])) {
        $sniproxy_config = $config['installedpackages']['sniproxy']['config'][0];
    } else {
        $sniproxy_config = array();
    }

    $ip_address = $sniproxy_config['ip_address'];
    $http_port = $sniproxy_config['http_port'];
    $https_port = $sniproxy_config['https_port'];
    $dns_address = $sniproxy_config['dns_address'];
    $bad_requests = $sniproxy_config['bad_requests'] == 'on' ? 'log' : 'off';
    $access_log = $sniproxy_config['access_log'];
    $error_log = $sniproxy_config['error_log'];
    $table_raw = preg_replace('/\r\n/', "\n", base64_decode($sniproxy_config['table_mappings']));
    $table_raw_https = preg_replace('/\r\n/', "\n", base64_decode($sniproxy_config['table_mappings_https']));
    $http_table_mappings = create_table_mappings($table_raw);
    $https_table_mappings = create_table_mappings($table_raw_https);

    $http_table_mappings = rtrim($http_table_mappings);
    $https_table_mappings = rtrim($https_table_mappings);

    $use_source = $sniproxy_config['use_wan_as_source'] == 'on';
    if($use_source) {
        $wan_address = get_current_wan_address();
        if(!empty($wan_address)) {
            $source = "source {$wan_address}";
        }
    }

    include("/usr/local/pkg/sniproxy.conf.template");
    $tconf .= PHP_EOL;

    file_put_contents(CONFIG_BASE . "sniproxy.conf", $tconf, LOCK_EX);

    $rc_start = "/usr/local/sbin/sniproxy -c /var/etc/sniproxy.conf";
    $rc_stop = "/usr/bin/killall sniproxy";

    write_rcfile(array('file' => 'sniproxy.sh', 'start' => $rc_start, 'stop' => $rc_stop));

    if(!verify_vip($ip_address)) {
        return 1;
    }

    if(is_service_running('sniproxy')) {
        restart_service('sniproxy');
    } else {
        start_service('sniproxy');
    }
}

function validate_sniproxy_conf() {
    global $input_errors;
    if(!verify_vip($_POST['ip_address'])) {
        $input_errors[] = 'The Source IP Address must be either a CARP or IP ALIAS';
    }
}
function create_table_mappings($table_raw) {
    // clean up the table
    $separator = "\n";
    $indent = str_repeat(" ", 4);
    $line = strtok($table_raw, $separator);
    $table_mappings = "";
    while ($line !== false) {
        $line = trim($line);
        if(!empty($line)) {
            $table_mappings .= $indent . $line . PHP_EOL;
        }
        $line = strtok( $separator );
    }
    return $table_mappings;
}

function verify_vip($ip_address) {
    $vip_list = get_configured_vip_list('all', VIP_IPALIAS);
    $vip_list = array_merge($vip_list, get_configured_vip_list('all', VIP_CARP));
    if(empty($vip_list)) {
        return false;
    }
    foreach ($vip_list as $ip) {
        if($ip_address === $ip) {
            return true;
        }
    }
    return false;
}
?>