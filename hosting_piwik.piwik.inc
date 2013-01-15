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
    $url .= "?module=$module&method=$method";

    foreach ($params as $key => $val) {
      $url .= "&" . urlencode($key) . "=" . urlencode($val);
    }

    $url .= "&format=JSON&token_auth=" . $this->_token_auth;

    $response = drupal_http_request($url);
    $response->data_decoded = json_decode($response->data);
    return $response;
  }

  /**
   * Register a site in the Piwik instance.
   * @param string $site_name
   *  The site name to provide to piwik, e.g. "Example Site"
   * @param array $urls
   *  The URLs to associate with the site,
   *  e.g. array('example.org', 'www.example.org', 'awesomealias.org')
   * @return int
   *  The ID which the Piwik instance assigns to the site.
   */
  public function register_site($site_name, $urls) {
    $params = array();
    $params['siteName'] = $site_name;

    for ($i = 0; $i < count($urls); $i++) {
      $params["urls[$i]"] = $urls[$i];
    }

    $response = $this->_execute_api_call('API', 'SitesManager.addSite',
        $params);
    $site_id = (int) $response->data_decoded->value;

    return $site_id;
  }

  /**
   * Get a list of all users in the Piwik instance, including 'anonymous'.
   * @return array
   *   An array of strings in format ('username1', 'username2', ...).
   */
  public function get_all_users() {
    $params = array();
    $response = $this->_execute_api_call('API', 'UsersManager.getUsersLogin',
        $params);
    $response_decoded = $response->data_decoded;

    $users = array_map(create_function('$u', 'return $u[0];'),
      $response_decoded);
    return $users;
  }

  /**
   * Give the specified user admin access to the specified site.
   * @param int $site_id
   *  The Piwik instance's ID for a site.
   * @param string $user
   *  The username of a user in the Piwik instance.
   */
  public function set_user_access($site_id, $user) {
    $params = array();
    $params['userLogin'] = $user;
    $params['access'] = 'admin';
    $params['idSites'] = $site_id;

    $this->_execute_api_call('API', 'UsersManager.setUserAccess', $params);
    return ;
  }

  /**
   * Add aliases to a site.
   * @param int $site_id
   *  The Piwik instance's ID for a site.
   * @param array $urls
   *  The URLs to add to the site,
   *  e.g. array('example2.org', 'www.example2.org', 'awesomealias2.org')
   */
  public function add_site_alias_urls($site_id, $urls) {
    $params = array();
    $params['idSite'] = $site_id;
    for ($i = 0; $i < count($urls); $i++) {
      $params["urls[$i]"] = $urls[$i];
    }

    $this->_execute_api_call('API', 'SitesManager.addSiteAliasUrls', $params);
  }
}

