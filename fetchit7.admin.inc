<?php

/**
 * @file
 * Adminisitration page callbacks for the fetchit7 module
 */
 
 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_settings() {
	// create a form with the basic fetchit configuration options
	// menu root name, menu root name title, which "db connected" form types exist
	// the "db connected" form types are new content types (node types) created and populated by
	// the fetchit7.install file

	// set the root menu machine name...perhaps allow the user to change this in a text box
	//variable_set('fetchit7_root_menu_name','fetchit7_nodes');
	$form['fetchit7_root_menu_name'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_root_menu_name','fetchit7_nodes'),
		'#title' => t('Enter the fetchit7 root menu internal machine name (generally, leave this alone...)'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	// set the root menu displayed name...perhaps allow the user to change this in a text box
	//variable_set('fetchit7_root_menu_title','fetchit7 nodes');
	$form['fetchit7_root_menu_title'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_root_menu_title','fetchit7 nodes'),
		'#title' => t('Enter the fetchit7 root menu displayed name'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
		
	// set whether the root menu is displayed...perhaps allow the user to change this with a check box
	//variable_set('fetchit7_root_menu_displayed',FALSE);
	$form['fetchit7_root_menu_displayed'] = array(
		'#type' => 'checkbox',
		'#default_value' => variable_get('fetchit7_root_menu_displayed',FALSE),
		'#title' => t('Display the fetchit7 nodes menu'),
		'#description' => t('Only select this if you want a fetchit7 menu with submenus for all the page content to be displayed by Drupal.'),
	);

	// get the fetchit7 content types from the fetchit7_content_types table and display them
	// this does not do much yet (maybe enables/disables them as a group?)
	$d7tablename = 'fetchit7_content_types';
	$d7returnfieldname = array('ndx','title','enabled');
	$query1 = db_select($d7tablename, 'n')
	->fields('n', $d7returnfieldname);
	$result1 = $query1->execute();
	$options = array();
	$enabled = array();
	// create the array of checkboxes to mark, enabled[]
	foreach ($result1 as $row1) {
		$options[$row1->ndx] = $row1->title;
		if ($row1->enabled) $enabled[] = $row1->ndx;
	}
	//
	$form['fetchit7_content_types'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Available fetchit7 content types'),
		'#options' => $options,
		'#default_value' => $enabled,
		'#description' => t('Check the content types where created forms should be enabled by default.'),
	);
	
	$form['#submit'][] = 'fetchit7_admin_settings_submit';
	return system_settings_form($form);
	
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_settings_submit ($form, $form_state) {
	// reset the fetchit7 root menu name
	if(!strlen(trim($string = $form_state['values']['fetchit7_root_menu_name']))) $string = 'fetchit7_nodes';
	variable_set('fetchit7_root_menu_name',$string);
	// reset the fetchit7 root menu label
	if(!strlen(trim($string = $form_state['values']['fetchit7_root_menu_title']))) $string = 'fetchit7 nodes';
	variable_set('fetchit7_root_menu_title',$string);
	// reset the fetchit7 root menu flag
	variable_set('fetchit7_root_menu_displayed',$form_state['values']['fetchit7_root_menu_displayed']);
	// update the fetchit7 content types enabled flags in the fetchit7_content_types table
	foreach ($form_state['values']['fetchit7_content_types'] as $key => $value) {
		$num_updated = db_update('fetchit7_content_types')
			->fields(array('enabled' => $value))
			->condition('ndx',$key,'=')
			->execute();
	}
}

/**
  * Form builder.  Configure fetchit7 database settings.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_dbsettings() {
	// these are the default database server connection settings
	// this will allow the fetchit7 user to only need to enter these once
	// i love my users :-)
	
	// set the default database server host name
	//variable_set('fetchit7_default_db_host','localhost');
	$form['fetchit7_default_db_host'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_host','localhost'),
		'#title' => t('Enter the default database server host name to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
		
	// set the default database server type
	//variable_set('fetchit7_default_db_type','PostgreSQL');
	$options = array();
	$options = array(0 => t('PostgreSQL'), 1 => t('MySQL'), 2 => t('SQL Server'));
	$form['fetchit7_default_db_type'] = array(
    '#type' => 'radios',
    '#title' => t('Select the default database server type to use for database connections'),
    '#default_value' => 0,
    '#options' => $options,
    '#description' => t('These are the database server types supported by fetchit7.'),
		'#required' => TRUE,
  );

	// set the default database server port number
	//variable_set('fetchit7_default_db_port', 5432);
	$form['fetchit7_default_db_port'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_port', 5432),
		'#title' => t('Enter the default database server port number to use for database connections'),
		'#size' => 10,
		'#maxlength' => 20,
		'#required' => TRUE,
	);
	
	// set the default database server database name
	//variable_set('fetchit7_default_db_name', 'default_db_name');
	$form['fetchit7_default_db_name'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_name', 'default_db_name'),
		'#title' => t('Enter the default database name to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);

	// set the default database server login name
	//variable_set('fetchit7_default_db_login', 'default_db_login');
	$form['fetchit7_default_db_login'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_login', 'default_db_login'),
		'#title' => t('Enter the default database login to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);

	// set the default database server password
	//variable_set('fetchit7_default_db_password', 'default_db_password');
	$form['fetchit7_default_db_password'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_password', 'default_db_password'),
		'#title' => t('Enter the default database password to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);

	$form['#submit'][] = 'fetchit7_admin_dbsettings_submit';
	return system_settings_form($form);
	
}


/**
 * Process fetchit7 default database settings submission
 */
function fetchit7_admin_dbsettings_submit ($form, $form_state) {
	//if(!strlen(trim($db_host = $form_state['values']['fetchit7_default_db_host']))) $db_host = 'localhost';
	$db_host = $form_state['values']['fetchit7_default_db_host'];
	//if(($db_type = $form_state['values']['fetchit7_default_db_type'])>2) $db_type = 0;
	$db_type = $form_state['values']['fetchit7_default_db_type'];
	//if(!($db_port = $form_state['values']['fetchit7_default_db_port'])) $db_port = 5432;
	$db_port = $form_state['values']['fetchit7_default_db_port'];
	$db_name = $form_state['values']['fetchit7_default_db_name'];
	$db_login = $form_state['values']['fetchit7_default_db_login'];
	$db_password = $form_state['values']['fetchit7_default_db_password'];
	// update the fetchit7 content types db settings in the fetchit7_content_types table
	$num_updated = db_update('fetchit7_content_types')
		->fields(array(
			'db_host' => $db_host,
			'db_type' => $db_type,
			'db_port' => $db_port,
			'db_name' => $db_name,
			'db_login' => $db_login,
			'db_password' => $db_password,
			)
		)
		->execute();
}

 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_reset() {
	$form['fetchit7_admin_reset_blurb'] = array(
		'#type' => 'item',
		'#markup' => t('here is where a reset button might exist.  not sure this is going to be needed, though.'),
	);

	$form['#submit'][] = 'fetchit7_admin_reset_submit';
	return system_settings_form($form);
	
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_reset_submit ($form, $form_state) {
}

 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_backupcontent() {
	$form['fetchit7_admin_backupcontent_blurb'] = array(
		'#type' => 'item',
		'#markup' => t('here is where a backup content button might exist.  not sure this is going to be needed, though.'),
	);

	$form['#submit'][] = 'fetchit7_admin_backupcontent_submit';
	return system_settings_form($form);
	
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_backupcontent_submit ($form, $form_state) {
}


 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_restorecontent() {
	$form['fetchit7_admin_restorecontent_blurb'] = array(
		'#type' => 'item',
		'#markup' => t('here is where a restore content button might exist.  not sure this is going to be needed, though.'),
	);

	$form['#submit'][] = 'fetchit7_admin_restorecontent_submit';
	return system_settings_form($form);
	
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_restorecontent_submit ($form, $form_state) {
}


/**
 * functions I might need...
 * http://drupal.stackexchange.com/questions/5951/is-there-a-drupal-api-function-to-get-the-list-of-nodes-by-type
 */
function node_load_by_type($type, $limit = 100, $offset = 0) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', $type)
    ->range($offset, $limit);
  $results = $query->execute();
  return node_load_multiple(array_keys($results['node']));
}
function entity_load_by_type($entity_type, $bundle, $limit = 100, $offset = 0) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type)
    ->entityCondition('bundle', $bundle)
    ->range($offset, $limit);
  $results = $query->execute();
  return entity_load($entity_type, array_keys($results[$bundle]));
}
