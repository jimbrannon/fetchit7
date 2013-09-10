<?php

/**
 * @file
 * Adminsitration page callbacks for the fetchit7 module
 */
 
 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_settings() {
	// create a form with the basic fetchit configuration options
	// menu root name, menu root name title, which "db connected" form types are enabled
	// the "db connected" form types are new content types (nodes) created and populated by
	// the fetchit7.install file

	// set the root menu machine name...perhaps allow the user to change this in a text box
	//variable_set('fetchit7_root_menu_name','fetchit7_nodes');
	$form['fetchit7_root_menu_name'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_root_menu_name','fetchit7_nodes'),
		'#title' => t('Enter the fetchit7 root menu internal machine name (generally, leave this alone...)'),
		'#size' => 60,
		'#maxlength' => 128,
		'#required' => TRUE,
	);
	
	// set the root menu displayed name...perhaps allow the user to change this in a text box
	//variable_set('fetchit7_root_menu_title','fetchit7 nodes');
	$form['fetchit7_root_menu_title'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_root_menu_title','fetchit7 nodes'),
		'#title' => t('Enter the fetchit7 root menu displayed name'),
		'#size' => 60,
		'#maxlength' => 128,
		'#required' => TRUE,
	);
		
	// set whether the root menu is displayed...perhaps allow the user to change this with a check box
	//variable_set('fetchit7_root_menu_displayed',FALSE);
	$form['fetchit7_root_menu_displayed'] = array(
		'#type' => 'checkbox',
		'#default_value' => variable_get('fetchit7_root_menu_displayed',FALSE),
		'#title' => t('Display the fetchit7 nodes menu'),
	);

	// get the fetchit7 content types
	$options = array();
	$options['html_table'] = 'HTML Table';
	$options['select_list'] = 'Select List';
	$options['editable_grid'] = 'Editable Table';
	$enabled = array();
	$enabled['html_table'] = FALSE;
	$enabled['select_list'] = FALSE;
	$enabled['editable_grid'] = FALSE;
	$form['fetchit7_content_types'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Enabled fetchit7 content types'),
		'#options' => $options,
		'#default_values' => $enabled,
		'#description' => t('These fetchit7 content types will be available when creating Drupal 7 page or block content'),
	);
	
	$form['#submit'][] = 'fetchit7_admin_settings_submit';
	return system_settings_form($form);
	
}

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
		'#size' => 60,
		'#maxlength' => 128,
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
		'#size' => 60,
		'#maxlength' => 128,
		'#required' => TRUE,
	);

	// set the default database server login name
	//variable_set('fetchit7_default_db_login', 'default_db_login');
	$form['fetchit7_default_db_login'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_login', 'default_db_login'),
		'#title' => t('Enter the default database login to use for database connections'),
		'#size' => 60,
		'#maxlength' => 128,
		'#required' => TRUE,
	);

	// set the default database server password
	//variable_set('fetchit7_default_db_password', 'default_db_password');
	$form['fetchit7_default_db_password'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_password', 'default_db_password'),
		'#title' => t('Enter the default database password to use for database connections'),
		'#size' => 60,
		'#maxlength' => 128,
		'#required' => TRUE,
	);

	$form['#submit'][] = 'fetchit7_admin_dbsettings_submit';
	return system_settings_form($form);
	
}
