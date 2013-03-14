<?php

function hosting_piwik_settings(&$form_state) {
  $form = array();

  $form['hosting_piwik_base_url'] = array(
    '#title' => t('Piwik Base URL'),
    '#description' => t('The URL where Piwik API requests will be sent.  ' .
      'For ex http://example.com/piwik/index.php'),
    '#type' => 'textfield',
    '#default_value' =>
      variable_get('hosting_piwik_base_url', ''),
  );

  $form['hosting_piwik_token'] = array(
    '#title' => t('Piwik Access Token'),
    '#description' => t('The access token to authenticate with the ' .
        'Piwik instance.'),
    '#type' => 'textfield',
    '#default_value' => variable_get('hosting_piwik_token', ''),
  );

  return system_settings_form($form);
}
