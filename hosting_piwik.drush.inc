<?php

/**
 * Implementation of drush_hook_pre_hosting_task()
 */
function drush_hosting_piwik_pre_hosting_task() {

  $task = & drush_get_context('HOSTING_TASK');

  if ($task->ref->type == 'site' && 
      ($task->task_type == 'install') || ($task->task_type == 'verify')) {
    
    if (intval($task->ref->piwik_site_id) == 0) {
      $piwik_site_id = _hosting_piwik_create_new_piwik_site(
        $task->ref->hosting_name, $task->ref->aliases);
      hosting_piwik_write_to_piwik_table($task, $piwik_site_id);
      $task->options['piwik_site_id'] = $piwik_site_id;
    }
    else {
      $task->options['piwik_site_id'] = $task->ref->piwik_site_id;
    }
    
    hosting_piwik_write_to_piwik_alias_table($task);
    $task->options['site_aliases'] = $task->ref->aliases;
  }

  if ($task->ref->type == 'site' && $task->task_type == 'verify') {
    hosting_piwik_register_new_aliases($task->ref->vid, 
        $task->ref->piwik_site_id, $task->ref->aliases);
  }
}

/**
 * Implementation of hook_provision_drupal_config()
 *
 * Add the piwik site id, monitoring access token and array of site aliases
 * to the sites settings.php file
 */
function hosting_piwik_provision_drupal_config($uri, $data) {
  $site_aliases = drush_get_option('site_aliases', '');
  $piwik_site_id = (int) drush_get_option('piwik_site_id', '');

  if ($piwik_site_id > 0) {
    $settings .= '$' . "conf['piwik_site_id'] = " . $piwik_site_id . ";\n";
  }

  if (is_array($site_aliases)) {
    $settings .= '$' . "conf['site_aliases'] = array(\n";
    foreach ($site_aliases as $key => $value) {
      $settings .= "$key => '$value',\n";
    }
    $settings .= ");\n";
  }

  return $settings;
}

function hosting_piwik_write_to_piwik_table($task, $site_id) {
  db_query("INSERT INTO {hosting_piwik_sites} (vid, nid, piwik_site_id)
            VALUES (%d, %d, %d)",
            $task->ref->vid, $task->ref->nid, $site_id);
}

function hosting_piwik_write_to_piwik_alias_table($task) {
  $site_aliases = $task->ref->aliases;
  foreach ($site_aliases as $alias)
    db_query("INSERT INTO {hosting_piwik_site_aliases}
              (vid, nid, piwik_site_alias)
              VALUES (%d, %d, '%s');",
              $task->ref->vid, $task->ref->nid, $alias);
}

/**
 * Create a new piwik site:
 * - Register the site
 * - Provision access to the site for all Piwik users.
 * @param string $site_name
 *   The name of the site in Aegir.
 * @param array $site_aliases
 *   The URL aliases for the site in Aegir.
 * @return int
 *   The Piwik site ID.
 */
function _hosting_piwik_create_new_piwik_site($site_name, $site_aliases) {
  module_load_include('inc', 'hosting_piwik', 'hosting_piwik.piwik');
  $piwik_api = new PiwikAPI(variable_get('hosting_piwik_base_url', ''),
      variable_get('hosting_piwik_token', ''));

  $piwik_site_id = $piwik_api->register_site($site_name, $site_aliases);

  $users = $piwik_api->get_all_users();
  foreach ($users as $user) {
    if ($user == 'anonymous') {
      continue;
    }

    $piwik_api->set_user_access($piwik_site_id, $user);
  }

  return $piwik_site_id;
}

/**
 * Register the new aliases that have been added to a site with Piwik.
 * @param int $vid
 *   The site's node revision id in Aegir.
 * @param int $piwik_site_id
 *   The Piwik site ID.
 * @param array $all_aliases
 *   The list of all aliases associated with the site (as entered in the site
 *   editing form in Aegir)
 */
function hosting_piwik_register_new_aliases($vid, $piwik_site_id,
    $all_aliases) {
  
  $new_aliases = hosting_piwik_get_new_aliases($vid, $all_aliases);

  if (!$new_aliases) {
    return;
  }

  module_load_include('inc', 'hosting_piwik', 'hosting_piwik.piwik');
  $piwik_api = new PiwikAPI(variable_get('hosting_piwik_base_url', ''),
      variable_get('hosting_piwik_token', ''));

  $piwik_api->add_site_alias_urls($piwik_site_id, $new_aliases);
}

/**
 * Get the new aliases for a site.
 * @param int $vid
 *   The site's node revision id in Aegir.
 * @param array $all_aliases
 *   The list of all aliases associated with the site (as entered in the site
 *   editing form in Aegir)
 * @return array
 *   A list of aliases which were entered on the site editing form in Aegir,
 *   but which have not yet been registered with Piwik.
 */
function hosting_piwik_get_new_aliases($vid, $all_aliases) {
  $registered_aliases = hosting_piwik_get_registered_piwik_aliases($vid);

  $new_aliases = array_diff($all_aliases, $registered_aliases);
  return $new_aliases;
}

function hosting_piwik_get_registered_piwik_aliases($vid) {
  $sql = "SELECT piwik_site_alias
            FROM {hosting_piwik_site_aliases}
            WHERE vid = %d;";
  $res = db_query($sql, $task->ref->vid);

  $registered_aliases = array();
  while ($row = db_fetch_object($res)) {
    $registered_aliases[] = $row->piwik_site_alias;
  }

  return $registered_aliases;
}
