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
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_reset_submit ($form, $form_state) {
}

 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  * 
 * Backup/restore page callbacks for the fetchit7 admin
 * 
 * the objective is to create a backup and restore mechanism for fetchit7 content - the node content types and associated fields -
 * so that it can be moved from site to site, or recovered after a fetchit uninstall/install cycle
 * 
 * the idea on the backup side is to simply convert the existing drupal 7 schema to flat tables in the host database, one per content type
 * 
 * on the backup side, should be doable using the instance info functions to get the fields, and the field info functions to get the field types
 * 
 * on the restore side, more care has to be taken, because the schema might be different - as in a uninstall/install cycle on a fetchit upgrade
 * therefore new records for each type will be created with the default values, then those fields which still exist will be replaced with the saved values.
 * 
 * basic and straight-forward, but should work for most circumstances site developers will encounter
  */
  
function fetchit7_admin_backupcontent() {
	
	//	show the html page content
	$form = array();
	
	// text element
	$form['fetchit7_admin_backupcontent_name'] = array(
		'#type' => 'textfield',
		'#default_value' => 'default',
		'#title' => t('Enter a name for this backup.  Warning: any previous backup with this same name will be overwritten.'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	$form['fetchit7_admin_backupcontent_submit'] = array(
		'#type' => 'submit',
		'#value' => 'Backup All Fetchit7 Content',
	);
	

	//$form['#submit'][] = 'fetchit7_admin_backupcontent_submit';
	//return system_settings_form($form);
	
	return $form;
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_backupcontent_submit ($form, $form_state) {
	// pop a log message
	//watchdog("fetchit7 module","entered fetchit7_admin_backupcontent_submit");
	// get the user entered string containing the backup name
	if(!strlen(trim($string = $form_state['values']['fetchit7_admin_backupcontent_name']))) $string = 'default';
	// fetchit7 HTML table content
	// TODO: should loop through all fetchit7 content types - search by name? - rather than have them hardcoded here
	$content_types = array();
	$content_types[] = 'fetchit7_content_table';
	$contenttypecount = 0;
	$contentcount = 0;
	foreach ($content_types as $content_type) {
		$tablename = 'fetchit7_content_table_backup_'.$content_type;
		if($newrecords = fetchit7_content_backuptable_create($content_type,$tablename)) {
			++$contenttypecount;
			$contentcount = $contentcount + $newrecords;
		}
	}
	drupal_set_message(t("Successfully saved $contenttypecount content types, $contentcount records total."),'status');

}

function fetchit7_content_backuptable_create($type,$tablename) {
	//=============================================================
	// create the table, deleting any existing table with that name
	//=============================================================
	$nodetablefields = array();
	$entityfields = array();
	$fieldstrings = array();
	$fieldstrings[] = 'ndx int PRIMARY KEY';
	$query  = fetchit7_maketablequery_create($tablename,$fieldstrings);
	if(!($result = db_query($query))) return FALSE;
	
	$query = "DROP TABLE $tablename;";
	if(!($result = db_query($query))) return FALSE;
	// the node table fields
	$fieldstrings = array();
	$fieldstrings[] = 'nid int PRIMARY KEY';
	$nodetablefields[] = 'nid';
	$fieldstrings[] = 'type varchar (255)';
	$nodetablefields[] = 'type';
	$fieldstrings[] = 'title varchar (255)';
	$nodetablefields[] = 'title';
	// get the fetchit7 installed fields for this type (i.e. instances)
	$instances = field_info_instances('node',$type);
	foreach ($instances as $instance_name => $instance) {
		$field_name = $instance['field_name'];
		$entityfields[] = $field_name;
		$field = field_info_field($field_name);
		$field_type = $field['columns']['value']['type'];
		if ($field_type == 'varchar') {
			$field_type .= ' ('.$field['columns']['value']['length'].')';
		}
		$fieldstrings[] = $field_name.' '.$field_type;
	}
	$query  = fetchit7_maketablequery_create($tablename,$fieldstrings);
	if(!($result = db_query($query))) return FALSE;

	//=============================================================
	// populate the new table based on the output of EntityFieldQuery
	// and the node table
	//=============================================================
	$query = new EntityFieldQuery();
	$query
		->entityCondition('entity_type', 'node')
		->entityCondition('bundle', $type)
		->propertyCondition('status', 1)
		->propertyOrderBy('created', 'DESC')
		//->fieldCondition('field_us_state', 'value', array('CT', 'NJ', 'NY'))
		//->range(0,10)
		;
	$result = $query->execute();
	$inserted_count = 0;
	if (!empty($result['node'])) {
		// get the nids
		$entityfieldnids = array_keys($result['node']);
		$entityfieldnodes = node_load_multiple($entityfieldnids);
		//$entityfieldnodes_form = node_view_multiple($entityfieldnodes, 'full');
		//now get the nids in the node table for this type
		$sql = 'SELECT * FROM {node} n WHERE n.type = :type';
		$result = db_query($sql, array(':type' => $type));
		// loop through the records and get the field values from both object arrays
		// to create an insert query
		// loop over the node table list - this should be the same as the entityfield list (right?)
		// the $entityfieldnodes object array is keyed by the nid, so it will be easy to get the object we need
		foreach ($result as $row) {
			$data = array();
			foreach ($nodetablefields as $nodetablefield) {
				$data[$nodetablefield] = $row->$nodetablefield;
			}
			foreach ($entityfields as $entityfield) {
				$mynode = $entityfieldnodes[$row->nid];
				$myfield = $mynode->$entityfield;
				$data[$entityfield] = $myfield['und'][0]['value'];;
			}
			//now do the insert
			$insertednid = db_insert($tablename) // Table name no longer needs {}
				->fields($data)
				->execute();
			if ($insertednid) ++$inserted_count;
		}
	}
	
	return $inserted_count;
}


/**
 * function to create a MAKE TABLE query
 */
function fetchit7_maketablequery_create($tablename,$fieldstrings) {
	$query  = 'CREATE TABLE IF NOT EXISTS ';
	$query .= $tablename;
	$query .= ' (';
	foreach ($fieldstrings as $key=>$fieldstring) {
		if ($key) $query .= ',';
		$query .= $fieldstring;
	}
	$query .= ');';
	
	return $query; 
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
  $result = $query->execute();
  $nodes = array();
  if (isset($result['node'])) {
  	$nodes = node_load_multiple(array_keys($result['node']));
	}
	
	return $nodes;
}

function entity_load_by_type($entity_type, $bundle, $limit = 100, $offset = 0) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type)
    ->entityCondition('bundle', $bundle)
    ->range($offset, $limit);
  $result = $query->execute();
  $entities = array();
  if (isset($result[$bundle])) {
  	$entities = entity_load($entity_type, array_keys($result[$bundle]));
	}
	
  return $entities;
}


