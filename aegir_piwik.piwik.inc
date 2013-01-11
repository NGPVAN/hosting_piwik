<?php

class PiwikAPI {
  
  private $_base_url = '';
  private $_token_auth = '';
  
  public function __construct($base_url, $token_auth) {
    $this->_base_url = $base_url;
    $this->_token_auth = $token_auth;
  }
  
  private function _execute_api_call($module, $method, $params) {
    $url = $this->_base_url;
    $url .= "?module=$module&$method=$method";
    
    foreach ($params as $key => $val) {
      $url .= "&" . urlencode($key) . "=" . urlencode($val);
    }
    
    $url .= "&format=JSON&token_auth=" . $this->_token_auth;
    
    $response = drupal_http_request($url);
    $response->data_decoded = json_decode($response->data);
    return $response;
  }
  
  public function register_site($site_name, $urls) {
    $params = array();
    $params['siteName'] = $site_name;
    
    for ($i = 0; $i < count($urls); $i++) {
      $params["urls[$i]"] = $urls[$i];
    }

    $response = $this->_execute_api_call('API', 'SitesManager.addSite',
        $params);
    $site_id = (int) $response->data_decoded['value'];
    
    return $site_id;
  }
  
  public function get_all_users() {
    $params = array();
    $response = $this->_execute_api_call('API', 'UsersManager.getUsersLogin',
        $params);
    return $response->data_decoded;
  }
  
  public function set_user_access($site_id, $user) {
    $params = array();
    $params['userLogin'] = $user;
    $params['access'] = 'admin';
    $params['idSite'] = $site_id;
    
    $this->_execute_api_call('API', 'UsersManager.setUserAccess', $params);
    return ;
  }
  
  public function add_site_alias_urls($site_id, $urls) {
    $params = array();
    $params['idSite'] = $site_id;
    for ($i = 0; $i < count($urls); $i++) {
      $params["urls[$i]"] = $urls[$i];
    }

    $this->_execute_api_call('API', 'SitesManager.addSiteAliasUrls', $params);    
  }
}
