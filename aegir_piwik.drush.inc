<?php

/**
 * Implementation of drush_hook_pre_hosting_task()
 */
function drush_aegir_piwik_pre_hosting_task() {

  $task = & drush_get_context('HOSTING_TASK');

  if ($task->ref->type == 'site' && $task->task_type == 'install') {
    module_load_include('inc', 'aegir_piwik', 'aegir_piwik.piwik');
    $piwik_api = new PiwikAPI(variable_get('aegir_piwik_base_url', ''), 
        variable_get('aegir_piwik_token', ''));

    $piwik_site_id = $piwik_api->register_site(
        $task->ref->hosting_name, $task->ref->aliases);
    
    $users = $piwik_api->get_all_users();
    foreach ($users as $user) {
      if ($user == 'anonymous') {
        continue;
      }
      
      $piwik_api->set_user_access($piwik_site_id, $user);
    }
    
    aegir_piwik_write_to_piwik_table($task, $piwik_site_id);
    aegir_piwik_write_to_piwik_alias_table($task);

    $task->options['piwik_site_id'] = $piwik_site_id;
    $task->options['site_aliases'] = $task->ref->aliases;
  }

  if ($task->ref->type == 'site' && $task->task_type == 'verify') {
    module_load_include('inc', 'aegir_piwik', 'aegir_piwik.piwik');
    $piwik_api = new PiwikAPI(variable_get('aegir_piwik_base_url', ''), 
        variable_get('aegir_piwik_token', ''));

    aegir_piwik_register_new_aliases($task, $piwik_api);
    
    $task->options['piwik_site_id'] = $task->ref->piwik_site_id;
    $task->options['site_aliases'] = $task->ref->aliases;
  }

}

/**
 * Implementation of hook_provision_drupal_config()
 *
 * Add the piwik site id, monitoring access token and array of site aliases
 * to the sites settings.php file
 */
function aegir_piwik_provision_drupal_config($uri, $data) {
  $site_aliases = drush_get_option('site_aliases', '');
  $piwik_site_id = drush_get_option('piwik_site_id', '');

  $settings .= '$' . "conf['piwik_site_id'] = " . $piwik_site_id . ";\n";
  $settings .= '$' . "conf['site_aliases'] = array(\n";

  foreach ($site_aliases as $key => $value) {
    $settings .= "$key => '$value',\n";
  }
  $settings .= ");\n";

  return $settings;
}

function aegir_piwik_write_to_piwik_table($task, $site_id) {
  db_query("INSERT INTO {aegir_piwik_sites} (vid, nid, piwik_site_id)
            VALUES (%d, %d, %d)",
            $task->ref->vid, $task->ref->nid, $site_id);
}

function aegir_piwik_write_to_piwik_alias_table($task) {
  $site_aliases = $task->ref->aliases;
  foreach ($site_aliases as $alias)
    db_query("INSERT INTO {aegir_piwik_site_aliases}
              (vid, nid, piwik_site_alias)
              VALUES (%d, %d, '%s');",
              $task->ref->vid, $task->ref->nid, $alias);
}

function aegir_piwik_register_new_aliases($task, $piwik_api) {
  $new_aliases = aegir_piwik_get_new_aliases($task);
  $site_id = drush_get_option('piwik_site_id', '');

  if (!$new_aliases) {
    return;
  }

  $piwik_api->add_site_alias_urls($site_id, $new_aliases);
}

function aegir_piwik_get_new_aliases($task) {
  $task_aliases = $task->ref->aliases;
  $registered_aliases = aegir_piwik_get_registered_piwik_aliases($task);

  $new_aliases = array_diff($task_aliases, $registered_aliases);
  return $new_aliases;
}

function aegir_piwik_get_registered_piwik_aliases($task) {
  $sql = "SELECT piwik_site_alias
            FROM {aegir_piwik_site_aliases}
            WHERE vid = %d;";
  $res = db_query($sql, $task->ref->vid);

  $registered_aliases = array();
  while ($row = db_fetch_array($res)) {
    $registered_aliases[] = $row;
  }

  return $registered_aliases;
}

